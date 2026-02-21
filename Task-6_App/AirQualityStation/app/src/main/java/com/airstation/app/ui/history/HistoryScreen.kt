package com.airstation.app.ui.history

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.airstation.app.domain.model.SensorType
import com.airstation.app.ui.dashboard.DashboardViewModel
import com.patrykandpatrick.vico.compose.axis.horizontal.rememberBottomAxis
import com.patrykandpatrick.vico.compose.axis.vertical.rememberStartAxis
import com.patrykandpatrick.vico.compose.chart.Chart
import com.patrykandpatrick.vico.compose.chart.line.lineChart
import com.patrykandpatrick.vico.core.entry.entryModelOf
import com.patrykandpatrick.vico.core.axis.AxisPosition
import com.patrykandpatrick.vico.core.axis.formatter.AxisValueFormatter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import com.patrykandpatrick.vico.core.entry.FloatEntry

@Composable
fun HistoryScreen(viewModel: DashboardViewModel) {
    val selectedSensor by viewModel.selectedHistorySensor.collectAsState()
    val history by viewModel.historyState.collectAsState()

    Column(modifier = Modifier.padding(16.dp)) {
        Text("Análise Detalhada", style = MaterialTheme.typography.headlineSmall)
        Spacer(modifier = Modifier.height(16.dp))

        // 1. Abas para selecionar o Sensor
        ScrollableTabRow(
            selectedTabIndex = getTabIndex(selectedSensor),
            edgePadding = 0.dp,
            containerColor = MaterialTheme.colorScheme.surface
        ) {
            val sensors = listOf(SensorType.PM25, SensorType.PM10, SensorType.CO, SensorType.TEMPERATURE, SensorType.HUMIDITY)
            sensors.forEach { type ->
                Tab(
                    selected = selectedSensor == type,
                    onClick = { viewModel.selectSensorForHistory(type) },
                    text = { Text(type.name) }
                )
            }
        }

        Spacer(modifier = Modifier.height(24.dp))

        // 2. Área do Gráfico Profissional
        Card(
            modifier = Modifier.fillMaxWidth().height(320.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            if (history.size > 1) {
                // CORREÇÃO: Convertendo o histórico em uma lista de FloatEntry
                val entries = history.mapIndexed { index, reading ->
                    FloatEntry(x = index.toFloat(), y = reading.value.toFloat())
                }
                val chartEntryModel = entryModelOf(entries)

                // Formatador do Eixo X (Tempo)
                val bottomAxisFormatter = AxisValueFormatter<AxisPosition.Horizontal.Bottom> { value, _ ->
                    val index = value.toInt()
                    if (index >= 0 && index < history.size) {
                        val date = Date(System.currentTimeMillis() - (history.size - index) * 1000L)
                        SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(date)
                    } else ""
                }

                // Formatador do Eixo Y (Valores)
                val startAxisFormatter = AxisValueFormatter<AxisPosition.Vertical.Start> { value, _ ->
                    "%.1f".format(value)
                }

                Chart(
                    chart = lineChart(),
                    model = chartEntryModel,
                    startAxis = rememberStartAxis(
                        valueFormatter = startAxisFormatter,
                        title = "Valor" // Título do Eixo Y
                    ),
                    bottomAxis = rememberBottomAxis(
                        valueFormatter = bottomAxisFormatter,
                        title = "Tempo", // Título do Eixo X
                        labelRotationDegrees = 45f // Rotação para a hora não sobrepor
                    ),
                    modifier = Modifier.padding(16.dp).fillMaxSize()
                )
            } else {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        CircularProgressIndicator(modifier = Modifier.size(24.dp))
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(if (history.isEmpty()) "Aguardando dados..." else "Traçando eixos...", color = Color.Gray)
                    }
                }
            }
        }

        // 3. Estatísticas Rápidas
        Spacer(modifier = Modifier.height(16.dp))
        if (history.isNotEmpty()) {
            val min = history.minOf { it.value }
            val max = history.maxOf { it.value }
            val avg = history.map { it.value }.average()

            Text("Estatísticas da Amostra:", style = MaterialTheme.typography.labelLarge)
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                Text("📉 Min: %.1f".format(min))
                Text("📊 Méd: %.1f".format(avg))
                Text("📈 Max: %.1f".format(max))
            }
        }
    }
}

fun getTabIndex(type: SensorType): Int {
    return when(type) {
        SensorType.PM25 -> 0
        SensorType.PM10 -> 1
        SensorType.CO -> 2
        SensorType.TEMPERATURE -> 3
        SensorType.HUMIDITY -> 4
        else -> 0
    }
}