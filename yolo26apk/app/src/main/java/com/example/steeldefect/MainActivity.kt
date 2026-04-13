package com.example.steeldefect

import android.Manifest
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageCapture
import androidx.camera.core.ImageCaptureException
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import com.example.steeldefect.databinding.ActivityMainBinding
import java.io.File
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private var imageCapture: ImageCapture? = null
    private lateinit var cameraExecutor: ExecutorService
    private val detector = NcnnYoloDetector()

    private val permissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) {
            startCamera()
        } else {
            Toast.makeText(this, "需要相机权限", Toast.LENGTH_LONG).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        cameraExecutor = Executors.newSingleThreadExecutor()

        val ok = detector.init(assets)
        binding.statusText.text = if (ok) "模型加载成功" else "模型加载失败（请检查 assets 与 NCNN 配置）"

        binding.captureButton.setOnClickListener { captureAndDetect() }
        requestCameraPermissionIfNeeded()
    }

    override fun onDestroy() {
        super.onDestroy()
        cameraExecutor.shutdown()
    }

    private fun requestCameraPermissionIfNeeded() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            startCamera()
        } else {
            permissionLauncher.launch(Manifest.permission.CAMERA)
        }
    }

    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)
        cameraProviderFuture.addListener({
            val cameraProvider = cameraProviderFuture.get()
            val preview = Preview.Builder().build().also {
                it.setSurfaceProvider(binding.previewView.getSurfaceProvider())
            }
            imageCapture = ImageCapture.Builder().build()
            val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
            cameraProvider.unbindAll()
            cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageCapture)
        }, ContextCompat.getMainExecutor(this))
    }

    private fun captureAndDetect() {
        val ic = imageCapture ?: return
        val photoFile = File(cacheDir, "capture_${System.currentTimeMillis()}.jpg")
        val output = ImageCapture.OutputFileOptions.Builder(photoFile).build()

        binding.statusText.text = "拍照中..."
        ic.takePicture(output, cameraExecutor, object : ImageCapture.OnImageSavedCallback {
            override fun onError(exception: ImageCaptureException) {
                runOnUiThread { binding.statusText.text = "拍照失败: ${exception.message}" }
            }

            override fun onImageSaved(outputFileResults: ImageCapture.OutputFileResults) {
                val bitmap = BitmapFactory.decodeFile(photoFile.absolutePath)
                if (bitmap == null) {
                    runOnUiThread { binding.statusText.text = "读取照片失败" }
                    return
                }
                val result = runDetection(bitmap)
                runOnUiThread {
                    binding.resultImageView.setImageBitmap(result)
                    binding.resultImageView.visibility = android.view.View.VISIBLE
                    val saved = ImageUtils.saveToGallery(this@MainActivity, result)
                    binding.statusText.text = if (saved) "识别完成，结果已保存到相册/Pictures/SteelDefect" else "识别完成，但保存失败"
                }
            }
        })
    }

    private fun runDetection(bitmap: Bitmap): Bitmap {
        val detections = detector.detect(bitmap)
        return ImageUtils.drawDetections(bitmap, detections)
    }
}
