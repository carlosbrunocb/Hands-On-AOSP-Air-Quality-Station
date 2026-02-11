package com.airstation.app.ui

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels // Importante
import androidx.compose.material3.Text
import com.airstation.app.ui.dashboard.DashboardViewModel
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class MainActivity : ComponentActivity() {

    // Cria o ViewModel (Lazy)
    private val viewModel: DashboardViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Log.i("AirStationApp", "Activity onCreate: Iniciando UI...")

        // [CORREÇÃO] Força a criação do ViewModel acessando uma propriedade ou método dele.
        // Isso vai disparar o bloco init {} do ViewModel -> Repository -> DataSource
        Log.i("AirStationApp", "ViewModel HashCode: ${viewModel.hashCode()}")

        setContent {
            // UI Simples para Sprint 1
            Text(text = "AirStation Sprint 1\nSensores Ativos.\nVerifique o Logcat (AirStationApp)")
        }
    }
}