# Steel Defect Android Demo (YOLO26 + NCNN)

这个工程已经接好以下流程：

1. 调用 `CameraX` 相机拍照  
2. 调用 `NCNN` 检测接口  
3. 在图片上绘制检测框  
4. 显示结果图  
5. 保存结果图到系统相册 `Pictures/SteelDefect`

## 1. 放置你的模型文件

把下面两个文件复制到：

`app/src/main/assets/`

- `model.ncnn.bin`
- `model.ncnn.param`

来源路径（你提供的）：

- `C:\Users\zhou\Desktop\project\Steel-surface-defect-detection-using-YOLO26\yolo26s-tune\runs\detect\train\weights\best_ncnn_model\model.ncnn.bin`
- `C:\Users\zhou\Desktop\project\Steel-surface-defect-detection-using-YOLO26\yolo26s-tune\runs\detect\train\weights\best_ncnn_model\model.ncnn.param`

## 2. 接入 NCNN Android SDK（Android Studio 新手版）

> 你只需要做一次，后面项目都能复用。

1. 打开 [NCNN Releases](https://github.com/Tencent/ncnn/releases)  
2. 下载 Android 预编译包（一般名字类似 `ncnn-xxxx-android-vulkan.zip`）  
3. 解压到一个固定目录，例如：`D:\ncnn-android-vulkan`  
4. 打开项目根目录下 `local.properties`（没有就新建）  
5. 增加一行（注意是你自己的实际路径）：

```properties
ncnn.dir=D:\\ncnn-android-vulkan\\arm64-v8a\\lib\\cmake\\ncnn
```

6. 回到 Android Studio，点击 `Sync Project with Gradle Files`

如果路径正确，`CMakeLists.txt` 会自动找到 `ncnn`，不需要你再手改 CMake。

## 3. JNI 推理代码

`app/src/main/cpp/yolo26_ncnn_jni.cpp` 已实现完成：

- `nativeInit(...)`：从 assets 加载 `model.ncnn.param/bin`
- `nativeDetect(...)`：`640` letterbox 预处理 + `out0` 解码 + NMS
- 返回格式：`[x1, y1, x2, y2, score, classId, ...]`

## 4. 运行

1. Android Studio 打开本目录  
2. 同步 Gradle  
3. 连接 Android 手机（arm64）  
4. 运行 app  
5. 点击“拍照并检测”

## 5. 你当前模型配置（已匹配）

- 输入尺寸：`640`
- 输出层：`out0`（已从 `.param` 自动确认）
- 类别：
  - crazing
  - inclusion
  - patches
  - pitted_surface
  - rolled-in_scale
  - scratches
