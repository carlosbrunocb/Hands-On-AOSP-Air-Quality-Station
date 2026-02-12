package com.airstation.app.ui.sensors

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.airstation.app.domain.model.SensorType
import com.airstation.app.ui.components.SensorCard
import com.airstation.app.ui.dashboard.DashboardViewModel

@Composable
fun SensorsScreen(viewModel: DashboardViewModel) {
    val sensorData by viewModel.uiState.collectAsState()
    val list = listOf(
        SensorType.PM25, SensorType.PM10, SensorType.CO,
        SensorType.LPG, SensorType.TEMPERATURE, SensorType.HUMIDITY
    )

    LazyColumn(contentPadding = PaddingValues(16.dp)) {
        item { Text("Monitoramento Detalhado", style = MaterialTheme.typography.headlineSmall) }
        item { Spacer(modifier = Modifier.height(16.dp)) }

        items(list) { type ->
            sensorData[type]?.let { reading ->
                SensorCard(reading = reading) // Reusa o card bonito com Gauge
            }
        }
    }
}