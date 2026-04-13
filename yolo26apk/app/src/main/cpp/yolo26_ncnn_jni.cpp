#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <net.h>
#include <algorithm>
#include <cmath>
#include <vector>

#define LOG_TAG "yolo26_ncnn_jni"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {
struct Object {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
    int label;
};

constexpr int kInputSize = 640;
constexpr int kNumClass = 6;
ncnn::Net g_net;
bool g_ready = false;

float intersection_area(const Object& a, const Object& b) {
    float x1 = std::max(a.x1, b.x1);
    float y1 = std::max(a.y1, b.y1);
    float x2 = std::min(a.x2, b.x2);
    float y2 = std::min(a.y2, b.y2);
    float w = std::max(0.0f, x2 - x1);
    float h = std::max(0.0f, y2 - y1);
    return w * h;
}

void nms_sorted_bboxes(const std::vector<Object>& objects, std::vector<int>& picked, float nms_threshold) {
    picked.clear();
    const int n = static_cast<int>(objects.size());
    std::vector<float> areas(n);
    for (int i = 0; i < n; i++) {
        areas[i] = std::max(0.0f, objects[i].x2 - objects[i].x1) * std::max(0.0f, objects[i].y2 - objects[i].y1);
    }

    for (int i = 0; i < n; i++) {
        const Object& a = objects[i];
        bool keep = true;
        for (int j : picked) {
            const Object& b = objects[j];
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[j] - inter_area;
            if (union_area > 0.0f && inter_area / union_area > nms_threshold) {
                keep = false;
                break;
            }
        }
        if (keep) picked.push_back(i);
    }
}

}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_steeldefect_NcnnYoloDetector_nativeInit(
        JNIEnv *env,
        jobject /*thiz*/,
        jobject assetManager,
        jstring paramPath,
        jstring binPath) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        LOGE("AAssetManager_fromJava failed");
        return JNI_FALSE;
    }

    const char* param_name = env->GetStringUTFChars(paramPath, nullptr);
    const char* bin_name = env->GetStringUTFChars(binPath, nullptr);

    g_net.clear();
    g_net.opt.use_vulkan_compute = false;
    g_net.opt.num_threads = 4;

    int ret_p = g_net.load_param(mgr, param_name);
    int ret_m = g_net.load_model(mgr, bin_name);

    env->ReleaseStringUTFChars(paramPath, param_name);
    env->ReleaseStringUTFChars(binPath, bin_name);

    g_ready = (ret_p == 0 && ret_m == 0);
    if (!g_ready) {
        LOGE("load param/model failed ret_p=%d ret_m=%d", ret_p, ret_m);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_example_steeldefect_NcnnYoloDetector_nativeDetect(
        JNIEnv *env,
        jobject /*thiz*/,
        jobject bitmap,
        jfloat scoreThreshold,
        jfloat nmsThreshold) {
    if (!g_ready) {
        return env->NewFloatArray(0);
    }

    AndroidBitmapInfo info{};
    if (AndroidBitmap_getInfo(env, bitmap, &info) != 0) {
        return env->NewFloatArray(0);
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("bitmap format is not RGBA_8888");
        return env->NewFloatArray(0);
    }

    void* pixels = nullptr;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != 0 || !pixels) {
        return env->NewFloatArray(0);
    }

    const int img_w = static_cast<int>(info.width);
    const int img_h = static_cast<int>(info.height);
    const float scale = std::min(static_cast<float>(kInputSize) / img_w, static_cast<float>(kInputSize) / img_h);
    const int resized_w = static_cast<int>(std::round(img_w * scale));
    const int resized_h = static_cast<int>(std::round(img_h * scale));
    const int pad_w = kInputSize - resized_w;
    const int pad_h = kInputSize - resized_h;
    const int pad_left = pad_w / 2;
    const int pad_top = pad_h / 2;

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(
            reinterpret_cast<const unsigned char*>(pixels),
            ncnn::Mat::PIXEL_RGBA2RGB,
            img_w,
            img_h,
            resized_w,
            resized_h
    );
    AndroidBitmap_unlockPixels(env, bitmap);

    ncnn::Mat in_pad;
    ncnn::copy_make_border(
            in,
            in_pad,
            pad_top,
            pad_h - pad_top,
            pad_left,
            pad_w - pad_left,
            ncnn::BORDER_CONSTANT,
            114.f
    );

    const float norm_vals[3] = {1.f / 255.f, 1.f / 255.f, 1.f / 255.f};
    in_pad.substract_mean_normalize(nullptr, norm_vals);

    ncnn::Extractor ex = g_net.create_extractor();
    ex.input("in0", in_pad);

    ncnn::Mat out_blob;
    if (ex.extract("out0", out_blob) != 0) {
        LOGE("extract out0 failed");
        return env->NewFloatArray(0);
    }

    std::vector<Object> proposals;
    if (out_blob.dims == 2) {
        int rows = out_blob.h;
        int cols = out_blob.w;
        bool transposed = false;
        if (cols != 4 + kNumClass && out_blob.h == 4 + kNumClass) {
            rows = out_blob.w;
            cols = out_blob.h;
            transposed = true;
        }
        if (cols == 4 + kNumClass) {
            for (int i = 0; i < rows; ++i) {
                float vals[10];
                if (!transposed) {
                    const float* p = out_blob.row(i);
                    for (int c = 0; c < cols; ++c) vals[c] = p[c];
                } else {
                    for (int c = 0; c < cols; ++c) vals[c] = out_blob.row(c)[i];
                }

                int best_label = -1;
                float best_score = 0.f;
                for (int c = 0; c < kNumClass; ++c) {
                    float s = vals[4 + c];
                    if (s > best_score) {
                        best_score = s;
                        best_label = c;
                    }
                }
                if (best_score < scoreThreshold) continue;

                float cx = vals[0];
                float cy = vals[1];
                float w = vals[2];
                float h = vals[3];
                float x1 = cx - w * 0.5f;
                float y1 = cy - h * 0.5f;
                float x2 = cx + w * 0.5f;
                float y2 = cy + h * 0.5f;

                x1 = (x1 - pad_left) / scale;
                y1 = (y1 - pad_top) / scale;
                x2 = (x2 - pad_left) / scale;
                y2 = (y2 - pad_top) / scale;
                x1 = std::max(0.f, std::min(x1, static_cast<float>(img_w - 1)));
                y1 = std::max(0.f, std::min(y1, static_cast<float>(img_h - 1)));
                x2 = std::max(0.f, std::min(x2, static_cast<float>(img_w - 1)));
                y2 = std::max(0.f, std::min(y2, static_cast<float>(img_h - 1)));
                if (x2 <= x1 || y2 <= y1) continue;

                proposals.push_back({x1, y1, x2, y2, best_score, best_label});
            }
        }
    }

    std::sort(proposals.begin(), proposals.end(), [](const Object& a, const Object& b) {
        return a.score > b.score;
    });

    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, nmsThreshold);

    std::vector<float> result;
    result.reserve(picked.size() * 6);
    for (int i : picked) {
        const Object& o = proposals[i];
        result.push_back(o.x1);
        result.push_back(o.y1);
        result.push_back(o.x2);
        result.push_back(o.y2);
        result.push_back(o.score);
        result.push_back(static_cast<float>(o.label));
    }

    jfloatArray j_out = env->NewFloatArray(static_cast<jsize>(result.size()));
    if (!result.empty()) {
        env->SetFloatArrayRegion(j_out, 0, static_cast<jsize>(result.size()), result.data());
    }
    return j_out;
}
