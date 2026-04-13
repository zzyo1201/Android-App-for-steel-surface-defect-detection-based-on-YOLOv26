package com.example.steeldefect

data class Detection(
    val x1: Float,
    val y1: Float,
    val x2: Float,
    val y2: Float,
    val score: Float,
    val classId: Int,
    val className: String
)
