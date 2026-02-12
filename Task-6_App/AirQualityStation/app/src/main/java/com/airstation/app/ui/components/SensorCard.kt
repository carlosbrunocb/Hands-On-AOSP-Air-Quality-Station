package com.airstation.app.ui.components

import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.model.AirQualityStandard

@Composable
fun SensorCard(
    reading: AirQualityReading,
    modifier: Modifier = Modifier
) {
    val status = AirQualityStandard.getStatus(reading.type, reading.value)
    val maxVal = SensorResources.getMaxValue(reading.type)

    // Animação da barra
    val progress by animateFloatAsState(
        targetValue = (reading.value / maxVal).coerceIn(0f, 1f),
        label = "progress"
    )

    Card(
        modifier = modifier.padding(8.dp).fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 4.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surface),
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                // ÍCONE COLORIDO
                Icon(
                    imageVector = SensorResources.getIcon(reading.type),
                    contentDescription = null,
                    tint = status.color,
                    modifier = Modifier.size(32.dp)
                )
                Spacer(modifier = Modifier.width(16.dp))

                // TEXTOS
                Column(modifier = Modifier.weight(1f)) {
                    Text(text = reading.type.name, style = MaterialTheme.typography.titleSmall)
                    Text(text = status.label, color = status.color, style = MaterialTheme.typography.labelSmall)
                }

                // VALOR
                Text(
                    text = "%.1f %s".format(reading.value, reading.unit),
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Bold
                )
            }
            Spacer(modifier = Modifier.height(12.dp))

            // BARRA DE PROGRESSO (GAUGE LINEAR)
            LinearProgressIndicator(
                progress = { progress },
                modifier = Modifier.fillMaxWidth().height(6.dp).clip(RoundedCornerShape(3.dp)),
                color = status.color,
                trackColor = MaterialTheme.colorScheme.surfaceVariant,
            )
        }
    }
}