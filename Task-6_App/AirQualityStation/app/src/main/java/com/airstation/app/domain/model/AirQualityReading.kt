package com.airstation.app.domain.model

data class AirQualityReading(
    val type: SensorType,
    val value: Float,
    val unit: String,
    val timestamp: Long = System.currentTimeMillis()
)

enum class SensorType {
    PM25, PM10, CO, LPG, TEMPERATURE, HUMIDITY, UNKNOWN
}