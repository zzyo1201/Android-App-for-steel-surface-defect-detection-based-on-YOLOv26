package com.example.steeldefect

import android.content.res.AssetManager
import android.graphics.Bitmap

class NcnnYoloDetector {
    companion object {
        private var libraryLoaded = false

        init {
            libraryLoaded = runCatching {
                System.loadLibrary("yolo26_ncnn")
                true
            }.getOrElse { false }
        }
    }

    private val labels = listOf("crazing", "inclusion", "patches", "pitted_surface", "rolled-in_scale", "scratches")

    external fun nativeInit(assetManager: AssetManager, paramPath: String, binPath: String): Boolean
    external fun nativeDetect(bitmap: Bitmap, scoreThreshold: Float, nmsThreshold: Float): FloatArray

    fun init(assetManager: AssetManager): Boolean {
        if (!libraryLoaded) return false
        return runCatching { nativeInit(
            assetManager = assetManager,
            paramPath = "model.ncnn.param",
            binPath = "model.ncnn.bin"
        ) }.getOrElse { false }
    }

    fun detect(bitmap: Bitmap, scoreThreshold: Float = 0.25f, nmsThreshold: Float = 0.45f): List<Detection> {
        if (!libraryLoaded) return emptyList()
        val raw = runCatching { nativeDetect(bitmap, scoreThreshold, nmsThreshold) }.getOrElse { floatArrayOf() }
        if (raw.isEmpty()) return emptyList()
        val one = 6
        if (raw.size % one != 0) return emptyList()

        return buildList {
            for (i in raw.indices step one) {
                val classId = raw[i + 5].toInt()
                add(
                    Detection(
                        x1 = raw[i],
                        y1 = raw[i + 1],
                        x2 = raw[i + 2],
                        y2 = raw[i + 3],
                        score = raw[i + 4],
                        classId = classId,
                        className = labels.getOrElse(classId) { "cls_$classId" }
                    )
                )
            }
        }
    }
}
