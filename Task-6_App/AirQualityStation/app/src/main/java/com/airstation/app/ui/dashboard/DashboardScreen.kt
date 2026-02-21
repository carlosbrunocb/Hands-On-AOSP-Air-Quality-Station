package com.airstation.app.ui.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.airstation.app.domain.model.SensorType
import com.patrykandpatrick.vico.compose.axis.horizontal.rememberBottomAxis
import com.patrykandpatrick.vico.compose.axis.vertical.rememberStartAxis
import com.patrykandpatrick.vico.compose.chart.Chart
import com.patrykandpatrick.vico.compose.chart.line.lineChart
import com.patrykandpatrick.vico.compose.chart.line.lineSpec
import com.patrykandpatrick.vico.core.entry.entryModelOf
import com.patrykandpatrick.vico.core.entry.FloatEntry

@Composable
fun DashboardScreen(
    viewModel: DashboardViewModel,
    onSettingsClick: () -> Unit
) {
    val sensorData by viewModel.uiState.collectAsState()
    val allHistory by viewModel.allSensorsHistoryState.collectAsState()

    val pm25 = sensorData[SensorType.PM25]
    val generalStatus = pm25?.let { com.airstation.app.domain.model.AirQualityStandard.getStatus(it.type, it.value) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp)
    ) {
        // --- CABEÇALHO ---
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text("📍 Air Monitoring 🌬️ ☁ ☁ ", style = MaterialTheme.typography.labelMedium)
                Text("📊 Air Quality Station", style = MaterialTheme.typography.titleLarge, fontWeight = FontWeight.Bold)
            }
            IconButton(onClick = onSettingsClick) {
                Icon(Icons.Default.Settings, contentDescription = "Configurações", tint = MaterialTheme.colorScheme.primary)
            }
        }

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
            Card(modifier = Modifier.weight(1f)) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text("🌡️ Temp", style = MaterialTheme.typography.labelMedium)

                    val temp = sensorData[SensorType.TEMPERATURE]?.value

                    Text(
                        text = temp?.let { String.format("%.1f", it) } ?: "--",
                        style = MaterialTheme.typography.headlineSmall,
                        fontWeight = FontWeight.Bold
                    )
//                    Text("${sensorData[SensorType.TEMPERATURE]?.value ?: "--"} °C", style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Bold)
                }
            }
            Card(modifier = Modifier.weight(1f)) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text("💧 Umidade", style = MaterialTheme.typography.labelMedium)

                    val hum = sensorData[SensorType.HUMIDITY]?.value

                    Text(
                        text = hum?.let { String.format("%.1f", it) } ?: "--",
                        style = MaterialTheme.typography.headlineSmall,
                        fontWeight = FontWeight.Bold
                    )
//                    Text("${sensorData[SensorType.HUMIDITY]?.value ?: "--"} %", style = MaterialTheme.typography.headlineSmall, fontWeight = FontWeight.Bold)
                }
            }
        }

        // --- BLOCO 4.1: NOVO GRÁFICO MULTI-LINHA ---
        Spacer(modifier = Modifier.height(24.dp))
        Text("TENDÊNCIA GERAL (10 MIN)", style = MaterialTheme.typography.labelSmall, color = Color.Gray)

        Card(
            modifier = Modifier.fillMaxWidth().height(250.dp).padding(vertical = 8.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White),
            elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
        ) {
            // Mapeando dados, com proteção contra listas vazias
            val pm25List = allHistory[SensorType.PM25]?.mapIndexed { index, reading ->
                FloatEntry(x = index.toFloat(), y = reading.value.toFloat())
            } ?: emptyList()

            val tempList = allHistory[SensorType.TEMPERATURE]?.mapIndexed { index, reading ->
                FloatEntry(x = index.toFloat(), y = reading.value.toFloat())
            } ?: emptyList()

            val humList = allHistory[SensorType.HUMIDITY]?.mapIndexed { index, reading ->
                FloatEntry(x = index.toFloat(), y = reading.value.toFloat())
            } ?: emptyList()

            if (pm25List.size > 1 && tempList.size > 1 && humList.size > 1) {
                Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
                    // Legenda
                    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceEvenly) {
                        LegendItem("PM2.5", Color.Red)
                        LegendItem("Temp", Color(0xFFFFA500)) // Laranja
                        LegendItem("Umid", Color.Blue)
                    }
                    Spacer(modifier = Modifier.height(8.dp))

                    // Gráfico com cores personalizadas (Agora o entryModelOf aceita as listas!)
                    val chartEntryModel = entryModelOf(pm25List, tempList, humList)
                    Chart(
                        chart = lineChart(
                            lines = listOf(
                                lineSpec(lineColor = Color.Red),
                                lineSpec(lineColor = Color(0xFFFFA500)),
                                lineSpec(lineColor = Color.Blue)
                            )
                        ),
                        model = chartEntryModel,
                        startAxis = rememberStartAxis(),
                        bottomAxis = rememberBottomAxis(),
                        modifier = Modifier.fillMaxSize()
                    )
                }
            } else {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Text("Coletando tendências...", color = Color.Gray, style = MaterialTheme.typography.bodySmall)
                }
            }
        }

        // --- BLOCO 3: DEBUG INFO ---
        Spacer(modifier = Modifier.height(24.dp))
        Card(
            colors = CardDefaults.cardColors(
                containerColor = Color.DarkGray
            )
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(12.dp),
                verticalArrangement = Arrangement.spacedBy(6.dp)
            ) {

                // Sistema
                Text(
                    text = "📱 Fonte: HAL Local | Rate: ~1Hz",
                    color = Color.White,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.Medium
                )

                Text(
                    text = "⚙️ App Version: 1.0.1",
                    color = Color.White,
                    fontSize = 10.sp
                )

                Divider(color = Color.White.copy(alpha = 0.3f))

                // Devs
                Text(
                    text = "👨‍💻 Developers:",
                    color = Color.White,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.Bold
                )

                Text(
                    text = "\t\t\t •  Carlos Bruno Oliveira Lopes",
                    color = Color.White,
                    fontSize = 10.sp
                )

                Text(
                    text = "\t\t\t •  Vicente Farias Neto",
                    color = Color.White,
                    fontSize = 10.sp
                )

                Text(
                    text = "\t\t\t •  Felipe Da Silva Vieira",
                    color = Color.White,
                    fontSize = 10.sp
                )

                Text(
                    text = "\t\t\t •  Josinel Da Silva Galucio",
                    color = Color.White,
                    fontSize = 10.sp
                )
            }
        }
    }
}

@Composable
fun MiniInfo(label: String, data: com.airstation.app.domain.model.AirQualityReading?) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(label, style = MaterialTheme.typography.labelSmall, fontWeight = FontWeight.Bold)
        Text(text = if (data != null) "%.0f".format(data.value) else "--", style = MaterialTheme.typography.bodyLarge)
        Text(data?.unit ?: "", style = MaterialTheme.typography.labelSmall, color = Color.Gray)
    }
}

@Composable
fun LegendItem(name: String, color: Color) {
    Row(verticalAlignment = Alignment.CenterVertically) {
        Box(modifier = Modifier.size(10.dp).background(color, CircleShape))
        Spacer(modifier = Modifier.width(4.dp))
        Text(name, fontSize = 10.sp, color = Color.DarkGray, fontWeight = FontWeight.Bold)
    }
}