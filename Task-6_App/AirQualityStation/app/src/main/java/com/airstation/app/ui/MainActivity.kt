package com.airstation.app.ui

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

@AndroidEntryPoint
class MainActivity : ComponentActivity() {
    private val viewModel: DashboardViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
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
                    composable("dashboard") { DashboardScreen(viewModel) }
                    composable("sensors") { SensorsScreen(viewModel) } // Vamos criar
                    composable("history") { HistoryScreen(viewModel) } // Vamos criar
                }
            }
        }
    }
}