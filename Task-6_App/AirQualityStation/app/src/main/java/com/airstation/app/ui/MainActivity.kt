package com.airstation.app.ui

import android.content.Intent
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Dashboard
import androidx.compose.material.icons.filled.History
import androidx.compose.material.icons.filled.Sensors
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.airstation.app.ui.dashboard.DashboardScreen
import com.airstation.app.ui.dashboard.DashboardViewModel
import com.airstation.app.ui.history.HistoryScreen
import com.airstation.app.ui.sensors.SensorsScreen
import dagger.hilt.android.AndroidEntryPoint
// Imports do WorkManager
import androidx.work.ExistingPeriodicWorkPolicy
import androidx.work.PeriodicWorkRequestBuilder
import androidx.work.WorkManager
import com.airstation.app.service.DataCollectionService
import com.airstation.app.ui.settings.SettingsScreen
import com.airstation.app.worker.AirQualityWorker
import java.util.concurrent.TimeUnit

@AndroidEntryPoint
class MainActivity : ComponentActivity() {
    private val viewModel: DashboardViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Inicia o Serviço de Coleta Contínua
        Intent(this, DataCollectionService::class.java).also { intent ->
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                startForegroundService(intent)
            } else {
                startService(intent)
            }
        }

        // --- MUDANÇA AQUI: Inicia o agendamento em segundo plano ---
        scheduleBackgroundWork()
        // -----------------------------------------------------------

        setContent {
            val navController = rememberNavController()
            var selectedItem by remember { mutableIntStateOf(0) }

            Scaffold(
                bottomBar = {
                    NavigationBar {
                        NavigationBarItem(
                            icon = { Icon(Icons.Default.Dashboard, contentDescription = null) },
                            label = { Text("Dashboard") },
                            selected = selectedItem == 0,
                            onClick = { selectedItem = 0; navController.navigate("dashboard") }
                        )
                        NavigationBarItem(
                            icon = { Icon(Icons.Default.Sensors, contentDescription = null) },
                            label = { Text("Sensores") },
                            selected = selectedItem == 1,
                            onClick = { selectedItem = 1; navController.navigate("sensors") }
                        )
                        NavigationBarItem(
                            icon = { Icon(Icons.Default.History, contentDescription = null) },
                            label = { Text("Histórico") },
                            selected = selectedItem == 2,
                            onClick = { selectedItem = 2; navController.navigate("history") }
                        )
                    }
                }
            ) { innerPadding ->
                NavHost(
                    navController = navController,
                    startDestination = "dashboard",
                    modifier = Modifier.padding(innerPadding)
                ) {
                    // 1. Dashboard
                    composable("dashboard") {
                        DashboardScreen(
                            viewModel = viewModel,
                            onSettingsClick = { navController.navigate("settings") }
                        )
                    }

                    // 2. Sensores
                    composable("sensors") { SensorsScreen(viewModel) }

                    // 3. Histórico
                    composable("history") { HistoryScreen(viewModel) }

                    // 4. Configurações
                    composable("settings") {
                        SettingsScreen(onBack = { navController.popBackStack() })
                    }
                }
            }
        }
    }

    // --- MUDANÇA AQUI: A função que configura o WorkManager ---
    private fun scheduleBackgroundWork() {
        val workRequest = PeriodicWorkRequestBuilder<AirQualityWorker>(
            15, TimeUnit.MINUTES // Define intervalo de 15 min
        ).build()

        WorkManager.getInstance(this).enqueueUniquePeriodicWork(
            "AirMonitorWorker", // Nome único para não duplicar tarefas
            ExistingPeriodicWorkPolicy.KEEP, // Se já existir, mantém (não reinicia a contagem)
            workRequest
        )
    }
}