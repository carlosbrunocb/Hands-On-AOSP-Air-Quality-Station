package com.airstation.app.service

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.Build
import android.os.IBinder
import androidx.core.app.NotificationCompat
import com.airstation.app.domain.repository.AirQualityRepository
import com.airstation.app.domain.usecase.SaveReadingUseCase
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import javax.inject.Inject

@AndroidEntryPoint
class DataCollectionService : Service() {

    @Inject lateinit var repository: AirQualityRepository
    @Inject lateinit var saveReadingUseCase: SaveReadingUseCase

    private val serviceScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    override fun onCreate() {
        super.onCreate()
        startForeground(2001, createNotification())
        startCollecting()
    }

    private fun startCollecting() {
        serviceScope.launch {
            // Fica rodando 24/7 enquanto o serviço estiver ativo
            repository.getReadings().collect { reading ->
                saveReadingUseCase(reading)
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        serviceScope.cancel() // Cancela a coleta se o serviço for morto
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun createNotification(): Notification {
        val channelId = "data_collection_channel"
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                channelId,
                "Monitoramento Contínuo",
                NotificationManager.IMPORTANCE_LOW // Low para não fazer som, apenas mostrar ícone
            )
            getSystemService(NotificationManager::class.java).createNotificationChannel(channel)
        }

        return NotificationCompat.Builder(this, channelId)
            .setContentTitle("AirStation Ativo")
            .setContentText("Monitorando e salvando dados em segundo plano...")
            .setSmallIcon(android.R.drawable.ic_menu_compass) // Ícone genérico (pode trocar depois)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()
    }
}