package com.airstation.app.domain.model

import androidx.compose.ui.graphics.Color

enum class QualityLevel(val color: Color, val label: String) {
    GOOD(Color(0xFF4CAF50), "Bom"),           // Verde
    MODERATE(Color(0xFFFFC107), "Moderado"),  // Amarelo
    CRITICAL(Color(0xFFF44336), "Crítico"),   // Vermelho
    UNKNOWN(Color.Gray, "--")
}

object AirQualityStandard {
    fun getStatus(type: SensorType, value: Float): QualityLevel {
        return when (type) {
            SensorType.PM25 -> when {
                value <= 25 -> QualityLevel.GOOD
                value <= 50 -> QualityLevel.MODERATE
                else -> QualityLevel.CRITICAL
            }
            SensorType.PM10 -> when {
                value <= 50 -> QualityLevel.GOOD
                value <= 100 -> QualityLevel.MODERATE
                else -> QualityLevel.CRITICAL
            }
            SensorType.CO -> when {
                value <= 9 -> QualityLevel.GOOD
                value <= 35 -> QualityLevel.MODERATE
                else -> QualityLevel.CRITICAL
            }
            SensorType.LPG -> when {
                value <= 500 -> QualityLevel.GOOD
                value <= 1000 -> QualityLevel.MODERATE
                else -> QualityLevel.CRITICAL
            }
            // Para Temp/Humid não aplicamos "risco" crítico da mesma forma, mas vamos padronizar como GOOD
            else -> QualityLevel.GOOD
        }
    }
}