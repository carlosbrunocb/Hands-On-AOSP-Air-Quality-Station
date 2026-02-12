package com.airstation.app.ui.components

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.*
import androidx.compose.ui.graphics.vector.ImageVector
import com.airstation.app.domain.model.SensorType

object SensorResources {
    fun getIcon(type: SensorType): ImageVector {
        return when (type) {
            SensorType.PM25 -> Icons.Rounded.CloudQueue
            SensorType.PM10 -> Icons.Rounded.Cloud
            SensorType.CO -> Icons.Rounded.Air
            SensorType.LPG -> Icons.Rounded.LocalFireDepartment
            SensorType.TEMPERATURE -> Icons.Rounded.Thermostat
            SensorType.HUMIDITY -> Icons.Rounded.WaterDrop
            else -> Icons.Rounded.Sensors
        }
    }

    // Valor máximo para calcular a porcentagem da barra (0 a 100%)
    fun getMaxValue(type: SensorType): Float {
        return when (type) {
            SensorType.PM25 -> 100f   // µg/m³
            SensorType.PM10 -> 200f   // µg/m³
            SensorType.CO -> 50f      // ppm
            SensorType.LPG -> 2000f   // ppm
            SensorType.TEMPERATURE -> 50f
            SensorType.HUMIDITY -> 100f
            else -> 100f
        }
    }
}