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

@Composable
fun HistoryScreen(viewModel: DashboardViewModel) {
    // Coleta os estados do ViewModel corrigido
    val selectedSensor by viewModel.selectedHistorySensor.collectAsState()
    val history by viewModel.historyState.collectAsState()

    Column(modifier = Modifier.padding(16.dp)) {
        Text("Histórico de Leituras", style = MaterialTheme.typography.headlineSmall)
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

        // 2. Área do Gráfico
        Card(
            modifier = Modifier.fillMaxWidth().height(300.dp),
            colors = CardDefaults.cardColors(containerColor = Color.White),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            // Só desenha o gráfico se tiver pelo menos 2 pontos
            if (history.size > 1) {
                // Prepara os dados para o Vico
                // Mapeamos apenas os valores (Y) -> O Vico calcula o X automaticamente por índice
                val chartEntryModel = entryModelOf(*history.map { it.value }.toTypedArray())

                Chart(
                    chart = lineChart(),
                    model = chartEntryModel,
                    startAxis = rememberStartAxis(),
                    bottomAxis = rememberBottomAxis(),
                    modifier = Modifier
                        .padding(16.dp)
                        .fillMaxSize()
                )
            } else {
                // Estado vazio (Enquanto coleta dados)
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        CircularProgressIndicator(modifier = Modifier.size(24.dp))
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = if (history.isEmpty()) "Aguardando dados..." else "Coletando mais amostras...",
                            style = MaterialTheme.typography.bodySmall,
                            color = Color.Gray
                        )
                        Text(
                            text = "(Aguarde alguns segundos)",
                            style = MaterialTheme.typography.labelSmall,
                            color = Color.LightGray
                        )
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

            Text("Estatísticas (Últimos 10 min):", style = MaterialTheme.typography.labelLarge)
            Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
                Text("Min: %.1f".format(min))
                Text("Méd: %.1f".format(avg))
                Text("Max: %.1f".format(max))
            }
        }
    }
}

// Função auxiliar para saber o índice da aba
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