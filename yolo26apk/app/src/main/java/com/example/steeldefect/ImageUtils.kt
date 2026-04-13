package com.example.steeldefect

import android.content.ContentValues
import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.provider.MediaStore

object ImageUtils {
    fun drawDetections(src: Bitmap, detections: List<Detection>): Bitmap {
        val out = src.copy(Bitmap.Config.ARGB_8888, true)
        val canvas = Canvas(out)
        val boxPaint = Paint().apply {
            style = Paint.Style.STROKE
            strokeWidth = 4f
            color = Color.RED
        }
        val textPaint = Paint().apply {
            color = Color.YELLOW
            textSize = 28f
            isAntiAlias = true
        }
        detections.forEach { det ->
            canvas.drawRect(det.x1, det.y1, det.x2, det.y2, boxPaint)
            canvas.drawText("${det.className} ${"%.2f".format(det.score)}", det.x1, (det.y1 - 10f).coerceAtLeast(20f), textPaint)
        }
        return out
    }

    fun saveToGallery(context: Context, bitmap: Bitmap, displayName: String = "steel_defect_${System.currentTimeMillis()}.jpg"): Boolean {
        val values = ContentValues().apply {
            put(MediaStore.Images.Media.DISPLAY_NAME, displayName)
            put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg")
            put(MediaStore.Images.Media.RELATIVE_PATH, "Pictures/SteelDefect")
        }
        val resolver = context.contentResolver
        val uri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values) ?: return false
        return runCatching {
            resolver.openOutputStream(uri)?.use { stream ->
                bitmap.compress(Bitmap.CompressFormat.JPEG, 95, stream)
            } ?: false
            true
        }.getOrElse {
            false
        }
    }
}
