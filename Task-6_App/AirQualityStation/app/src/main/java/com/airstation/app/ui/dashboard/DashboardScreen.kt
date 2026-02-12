package com.airstation.app.ui.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.airstation.app.domain.model.SensorType
import com.airstation.app.ui.components.SensorCard
import com.airstation.app.ui.components.SensorResources

@Composable
fun DashboardScreen(viewModel: DashboardViewModel) {
    val sensorData by viewModel.uiState.collectAsState()

    // Status Geral (Baseado no pior sensor - PM2.5 ou CO)
    val pm25 = sensorData[SensorType.PM25]
    val generalStatus = pm25?.let { com.airstation.app.domain.model.AirQualityStandard.getStatus(it.type, it.value) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp)
    ) {
        // --- CABEÇALHO ---
        Text("\uD83D\uDCCA Air Quality Station     \uD83C\uDF2C\uFE0F   ☁\uFE0F ☁\uFE0F ☁\uFE0F ", style = MaterialTheme.typography.labelLarge)
        Spacer(modifier = Modifier.height(16.dp))

        // --- BLOCO 1: STATUS GERAL DO AR ---
        Text("STATUS GERAL DO AR", style = MaterialTheme.typography.labelSmall, color = Color.Gray)
        Card(
            modifier = Modifier.fillMaxWidth().padding(vertical = 8.dp),
            colors = CardDefaults.cardColors(containerColor = generalStatus?.color?.copy(alpha=0.2f) ?: Color.LightGray),
            shape = RoundedCornerShape(16.dp)
        ) {
            Column(modifier = Modifier.padding(24.dp), horizontalAlignment = Alignment.CenterHorizontally) {
                Text(
                    text = generalStatus?.label?.uppercase() ?: "--",
                    style = MaterialTheme.typography.headlineMedium,
                    fontWeight = FontWeight.Bold,
                    color = generalStatus?.color ?: Color.Gray
                )
                Spacer(modifier = Modifier.height(16.dp))
                // Mini Resumo
                Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceAround) {
                    MiniInfo("PM2.5", sensorData[SensorType.PM25])
                    MiniInfo("PM10", sensorData[SensorType.PM10])
                    MiniInfo("CO", sensorData[SensorType.CO])
                }
            }
        }

        // --- BLOCO 2: AMBIENTE ---
        Spacer(modifier = Modifier.height(16.dp))
        Text("AMBIENTE", style = MaterialTheme.typography.labelSmall, color = Color.Gray)
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            // Card Temperatura
            Card(modifier = Modifier.weight(1f)) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text("🌡️ Temp", style = MaterialTheme.typography.labelMedium)
                    Text(
                        "${sensorData[SensorType.TEMPERATURE]?.value ?: "--"} °C",
                        style = MaterialTheme.typography.headlineSmall,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
            // Card Umidade
            Card(modifier = Modifier.weight(1f)) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text("💧 Umidade", style = MaterialTheme.typography.labelMedium)
                    Text(
                        "${sensorData[SensorType.HUMIDITY]?.value ?: "--"} %",
                        style = MaterialTheme.typography.headlineSmall,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
        }

        // --- BLOCO 3: DEBUG INFO ---
        Spacer(modifier = Modifier.height(24.dp))
        Card(colors = CardDefaults.cardColors(containerColor = Color.DarkGray)) {
            Text(
                "Fonte: HAL Local | Rate: ~1Hz\nApp Version: 1.0.0-Sprint3",
                color = Color.White,
                fontSize = 10.sp,
                modifier = Modifier.padding(12.dp)
            )
        }
    }
}

@Composable
fun MiniInfo(label: String, data: com.airstation.app.domain.model.AirQualityReading?) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(label, style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Bold)
        Text(
            text = if (data != null) "%.0f".format(data.value) else "--",
            style = MaterialTheme.typography.bodyLarge
        )
        Text(data?.unit ?: "", style = MaterialTheme.typography.labelSmall, color = Color.Gray)
    }
}