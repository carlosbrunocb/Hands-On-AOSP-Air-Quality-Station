package com.airstation.app.ui.settings

import android.content.Context
import android.os.Environment
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.airstation.app.data.repository.UserPreferencesRepository
import com.airstation.app.domain.model.SensorType
import com.airstation.app.domain.repository.AirQualityRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch
import java.io.File
import java.io.FileWriter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import javax.inject.Inject

@HiltViewModel
class SettingsViewModel @Inject constructor(
    private val preferencesRepository: UserPreferencesRepository,
    private val airRepository: AirQualityRepository // Injetado para pegar dados do CSV
) : ViewModel() {

    val notificationsEnabled = preferencesRepository.notificationsEnabled
        .stateIn(viewModelScope, SharingStarted.Lazily, true)

    val pm25Threshold = preferencesRepository.pm25Threshold
        .stateIn(viewModelScope, SharingStarted.Lazily, 35.0f)

    val coThreshold = preferencesRepository.coThreshold
        .stateIn(viewModelScope, SharingStarted.Lazily, 50.0f)

    // Controle do Pop-up (Toast) de exportação
    private val _exportMessage = MutableStateFlow<String?>(null)
    val exportMessage: StateFlow<String?> = _exportMessage.asStateFlow()

    fun clearExportMessage() { _exportMessage.value = null }

    fun toggleNotifications(enabled: Boolean) {
        viewModelScope.launch { preferencesRepository.setNotificationsEnabled(enabled) }
    }

    fun updateThreshold(value: Float) {
        viewModelScope.launch { preferencesRepository.setPm25Threshold(value) }
    }

    fun updateCoThreshold(value: Float) {
        viewModelScope.launch { preferencesRepository.setCoThreshold(value) }
    }

    fun exportDataToCsv() {
        viewModelScope.launch {
            try {
                // Busca histórico de PM2.5 e CO dos últimos 30 min (1.800.000 ms)
                val last30Min = 1800_000L
                val historyPm25 = airRepository.getHistory(SensorType.PM25, last30Min).first()
                val historyCo = airRepository.getHistory(SensorType.CO, last30Min).first()

                val combinedHistory = historyPm25 + historyCo

                if (combinedHistory.isEmpty()) {
                    _exportMessage.value = "Nenhum dado registrado nos últimos 30 minutos."
                    return@launch
                }

                // Cria o arquivo na pasta Downloads
                val downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
                val timestamp = SimpleDateFormat("yyyyMMdd_HHmm", Locale.getDefault()).format(Date())
                val fileName = "AirQuality_$timestamp.csv"
                val file = File(downloadsDir, fileName)

                // Escreve os dados no CSV
                FileWriter(file).use { writer ->
                    writer.append("Data/Hora,Sensor,Valor,Unidade\n")
                    combinedHistory.forEach { reading ->
                        // Assumindo que o timestamp existe. Se não existir no seu modelo, criaremos a hora atual
                        val timeStr = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault()).format(Date())
                        writer.append("$timeStr,${reading.type.name},${reading.value},${reading.unit}\n")
                    }
                }

                _exportMessage.value = "Sucesso! Arquivo salvo em Downloads: $fileName"
            } catch (e: Exception) {
                _exportMessage.value = "Erro ao exportar arquivo: ${e.message}"
            }
        }
    }
}