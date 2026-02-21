package com.airstation.app.worker

import android.content.Context
import android.util.Log
import androidx.hilt.work.HiltWorker
import androidx.work.CoroutineWorker
import androidx.work.WorkerParameters
import com.airstation.app.data.repository.UserPreferencesRepository
import com.airstation.app.domain.model.SensorType
import com.airstation.app.domain.repository.AirQualityRepository
import com.airstation.app.service.NotificationHelper
import dagger.assisted.Assisted
import dagger.assisted.AssistedInject
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.withTimeoutOrNull

@HiltWorker
class AirQualityWorker @AssistedInject constructor(
    @Assisted appContext: Context,
    @Assisted workerParams: WorkerParameters,
    private val repository: AirQualityRepository,
    private val preferencesRepository: UserPreferencesRepository,
    private val notificationHelper: NotificationHelper
) : CoroutineWorker(appContext, workerParams) {

    override suspend fun doWork(): Result {
        Log.d("AirWorker", "👷 Worker Iniciado! Verificando CO (Gás Letal) e PM2.5...")

        return try {
            // 1. Verifica se o usuário permitiu notificações
            val isEnabled = preferencesRepository.notificationsEnabled.first()
            if (!isEnabled) {
                Log.d("AirWorker", "🚫 Notificações desativadas. Parando execução.")
                return Result.success()
            }

            // 2. Busca os limites configurados na tela de Settings
            val pm25Threshold = preferencesRepository.pm25Threshold.first()
            val coThreshold = preferencesRepository.coThreshold.first()
            val tenMinutesMillis = 10 * 60 * 1000L

            // Variáveis para guardar os valores finais que irão para o Log de Resumo
            var finalCoValue: Double? = null
            var finalPm25Value: Double? = null

            // ==========================================================
            // ALARME LETAL: MONÓXIDO DE CARBONO (CO)
            // ==========================================================
            Log.d("AirWorker", "📡 Lendo sensor de CO...")
            val liveCo = withTimeoutOrNull(10000) { // 10s timeout
                repository.getReadings().first { it.type == SensorType.CO }
            }

            if (liveCo != null) {
                finalCoValue = liveCo.value.toDouble()
                Log.d("AirWorker", "✅ CO Atual (Tempo Real): $finalCoValue ppm")
            } else {
                Log.w("AirWorker", "⚠️ Falha ao ler CO em tempo real. Tentando buscar último valor no banco...")
                // Fallback: Busca histórico dos últimos 10 minutos
                val coHistory = repository.getHistory(SensorType.CO, tenMinutesMillis).first()

                if (coHistory.isNotEmpty()) {
                    // Pega o último valor salvo na lista (o mais recente)
                    finalCoValue = coHistory.last().value.toDouble()
                    Log.w("AirWorker", "♻️ Fallback CO: Usando último valor salvo no banco ($finalCoValue ppm).")
                } else {
                    Log.e("AirWorker", "❌ Falha crítica: Sem leitura em tempo real e sem histórico de CO nos últimos 10 min.")
                }
            }

            // Verifica se dispara a sirene do CO
            if (finalCoValue != null && finalCoValue > coThreshold) {
                Log.e("AirWorker", "🚨 PERIGO LETAL! CO acima do limite ($finalCoValue > $coThreshold). Disparando sirene!")
                notificationHelper.showCoLethalAlarmNotification(finalCoValue)
            }

            // ==========================================================
            // FILTRO ANTI-FALSO POSITIVO: PM2.5 (Média de 10 minutos)
            // ==========================================================
            Log.d("AirWorker", "📊 Calculando média de PM2.5 (Últimos 10 min)...")
            val pm25History = repository.getHistory(SensorType.PM25, tenMinutesMillis).first()

            if (pm25History.isNotEmpty()) {
                // Se o app estava coletando dados, usa a média
                finalPm25Value = pm25History.map { it.value }.average()
                Log.d("AirWorker", "📈 Média Histórica PM2.5 calculada: %.1f µg/m³".format(finalPm25Value))
            } else {
                // Fallback: Sem banco, lê do sensor agora
                Log.d("AirWorker", "Sem histórico recente no DB. 📡 Lendo sensor PM2.5 agora...")
                val livePm25 = withTimeoutOrNull(10000) {
                    repository.getReadings().first { it.type == SensorType.PM25 }
                }
                finalPm25Value = livePm25?.value?.toDouble()
                if (finalPm25Value != null) {
                    Log.d("AirWorker", "✅ PM2.5 Atual (Tempo Real): $finalPm25Value µg/m³")
                } else {
                    Log.e("AirWorker", "❌ Falha ao ler PM2.5 em tempo real.")
                }
            }

            // Verifica se dispara a notificação de PM2.5
            if (finalPm25Value != null) {
                if (finalPm25Value > pm25Threshold) {
                    Log.w("AirWorker", "⚠️ Poluição PM2.5 Alta! Enviando notificação simples...")
                    notificationHelper.showHighRiskNotification(finalPm25Value)
                } else {
                    Log.d("AirWorker", "👍 Ar seguro para PM2.5. Nenhuma ação necessária.")
                }
            }

            // ==========================================================
            // RESUMO NO LOG
            // ==========================================================
            val coLog = finalCoValue?.let { "$it ppm" } ?: "FALHA/INEXISTENTE"
            val pm25Log = finalPm25Value?.let { "%.1f µg/m³".format(it) } ?: "FALHA/INEXISTENTE"

            Log.i("AirWorker", "=======================================")
            Log.i("AirWorker", "📋 RESUMO DO WORKER FINALIZADO")
            Log.i("AirWorker", "💨 CO Lido: $coLog")
            Log.i("AirWorker", "🌫️ PM2.5 Calculado: $pm25Log")
            Log.i("AirWorker", "=======================================")

            Result.success()
        } catch (e: Exception) {
            Log.e("AirWorker", "💥 Erro crítico no Worker", e)
            Result.retry()
        }
    }
}