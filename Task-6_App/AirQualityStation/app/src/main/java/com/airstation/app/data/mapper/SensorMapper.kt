package com.airstation.app.data.mapper

import android.hardware.SensorEvent
import com.airstation.app.data.source.SensorConfig
import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.model.SensorType
import javax.inject.Inject

class SensorMapper @Inject constructor() {
    fun map(event: SensorEvent): AirQualityReading {
        val type = when (event.sensor.type) {
            SensorConfig.TYPE_PM25 -> SensorType.PM25
            SensorConfig.TYPE_PM10 -> SensorType.PM10
            SensorConfig.TYPE_CO -> SensorType.CO
            SensorConfig.TYPE_LPG -> SensorType.LPG
            SensorConfig.TYPE_TEMP -> SensorType.TEMPERATURE
            SensorConfig.TYPE_HUMID -> SensorType.HUMIDITY
            else -> SensorType.UNKNOWN
        }

        val unit = when (type) {
            SensorType.TEMPERATURE -> "°C"
            SensorType.HUMIDITY -> "%"
            SensorType.CO, SensorType.LPG -> "ppm"
            else -> "µg/m³"
        }

        return AirQualityReading(
            type = type,
            value = event.values[0],
            unit = unit
        )
    }
}