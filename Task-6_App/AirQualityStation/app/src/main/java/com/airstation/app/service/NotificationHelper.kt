package com.airstation.app.service

import android.Manifest
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.content.pm.PackageManager
import android.media.Ringtone
import android.media.RingtoneManager
import android.os.Build
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.content.ContextCompat
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.*
import javax.inject.Inject

class NotificationHelper @Inject constructor(
    @ApplicationContext private val context: Context
) {
    companion object {
        const val CHANNEL_ID_PM25 = "air_quality_alert_channel_high"
        const val CHANNEL_ID_CO = "co_lethal_alert_channel"
        const val NOTIFICATION_ID_PM25 = 1001
        const val NOTIFICATION_ID_CO = 1002
    }

    private var currentRingtone: Ringtone? = null
    private var alarmJob: Job? = null

    init {
        createNotificationChannels()
    }

    private fun createNotificationChannels() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val notificationManager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

            // Canal PM2.5 (Aviso Forte)
            val channelPm25 = NotificationChannel(CHANNEL_ID_PM25, "Alertas PM2.5", NotificationManager.IMPORTANCE_HIGH).apply {
                description = "Notificações sobre poluição de partículas"
                enableVibration(true)
            }

            // Canal CO (Alarme Crítico - Ignora Não Perturbe se permitido)
            val channelCo = NotificationChannel(CHANNEL_ID_CO, "Alarme Gás Letal (CO)", NotificationManager.IMPORTANCE_MAX).apply {
                description = "Alarme sonoro para risco de asfixia por Monóxido de Carbono"
                enableVibration(true)
                setBypassDnd(true)
            }

            notificationManager.createNotificationChannel(channelPm25)
            notificationManager.createNotificationChannel(channelCo)
        }
    }

    fun showHighRiskNotification(pm25Value: Double) {
        if (!hasPermission()) return

        val builder = NotificationCompat.Builder(context, CHANNEL_ID_PM25)
            .setSmallIcon(android.R.drawable.stat_sys_warning)
            .setContentTitle("⚠️ ALERTA DE AR: PERIGO!")
            .setContentText("Nível médio de PM2.5 muito alto: %.1f µg/m³".format(pm25Value))
            .setPriority(NotificationCompat.PRIORITY_MAX)
            .setCategory(NotificationCompat.CATEGORY_ALARM)
            .setAutoCancel(true)
            .setVibrate(longArrayOf(0, 500, 200, 500))

        NotificationManagerCompat.from(context).notify(NOTIFICATION_ID_PM25, builder.build())
    }

    fun showCoLethalAlarmNotification(coValue: Double) {
        if (!hasPermission()) return

        val builder = NotificationCompat.Builder(context, CHANNEL_ID_CO)
            .setSmallIcon(android.R.drawable.ic_dialog_alert)
            .setContentTitle("🚨 EVACUE O LOCAL: GÁS LETAL 🚨")
            .setContentText("Nível CRÍTICO de Monóxido de Carbono: %.1f ppm!".format(coValue))
            .setPriority(NotificationCompat.PRIORITY_MAX)
            .setCategory(NotificationCompat.CATEGORY_ALARM)
            .setColor(android.graphics.Color.RED)
            .setVibrate(longArrayOf(0, 1000, 500, 1000, 500, 1000))
            .setAutoCancel(true)

        NotificationManagerCompat.from(context).notify(NOTIFICATION_ID_CO, builder.build())

        // Inicia a sirene de 20 segundos
        playAlarmSoundFor20Seconds()
    }

    private fun playAlarmSoundFor20Seconds() {
        try {
            val uri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_ALARM)
            currentRingtone?.stop()
            alarmJob?.cancel()

            currentRingtone = RingtoneManager.getRingtone(context, uri)
            currentRingtone?.play()

            // Coroutine para parar o som após 20 segundos
            alarmJob = CoroutineScope(Dispatchers.Main).launch {
                delay(20000)
                currentRingtone?.stop()
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun hasPermission(): Boolean {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            ContextCompat.checkSelfPermission(context, Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED
        } else true
    }
}