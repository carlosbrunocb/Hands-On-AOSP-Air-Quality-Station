package com.airstation.app.ui.dashboard

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.model.SensorType
import com.airstation.app.domain.repository.AirQualityRepository
import com.airstation.app.domain.usecase.SaveReadingUseCase
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class DashboardViewModel @Inject constructor(
    private val repository: AirQualityRepository,
//    private val saveReadingUseCase: SaveReadingUseCase
) : ViewModel() {

    // 1. Estado da Tela Principal (Realtime)
    private val _uiState = MutableStateFlow<Map<SensorType, AirQualityReading>>(emptyMap())
    val uiState: StateFlow<Map<SensorType, AirQualityReading>> = _uiState.asStateFlow()

    // 2. Controle do Histórico (Qual sensor o usuário quer ver na aba de Detalhes?)
    private val _selectedHistorySensor = MutableStateFlow(SensorType.PM25)
    val selectedHistorySensor: StateFlow<SensorType> = _selectedHistorySensor.asStateFlow()

    // 3. O Fluxo de Histórico Individual (Reage à mudança da aba)
    @OptIn(ExperimentalCoroutinesApi::class)
    val historyState: StateFlow<List<AirQualityReading>> = _selectedHistorySensor
        .flatMapLatest { sensorType ->
            // Busca os últimos 10 minutos (600.000 ms)
            repository.getHistory(sensorType, 600_000L)
        }
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = emptyList()
        )

    // 4. NOVO: Fluxo Combinado para o Gráfico Geral (Dashboard)
    val allSensorsHistoryState: StateFlow<Map<SensorType, List<AirQualityReading>>> = combine(
        repository.getHistory(SensorType.PM25, 600_000L),
        repository.getHistory(SensorType.PM10, 600_000L),
        repository.getHistory(SensorType.TEMPERATURE, 600_000L),
        repository.getHistory(SensorType.HUMIDITY, 600_000L)
    ) { pm25, pm10, temp, hum ->
        mapOf(
            SensorType.PM25 to pm25,
            SensorType.PM10 to pm10,
            SensorType.TEMPERATURE to temp,
            SensorType.HUMIDITY to hum
        )
    }.stateIn(
        scope = viewModelScope,
        started = SharingStarted.WhileSubscribed(5000),
        initialValue = emptyMap()
    )

    init {
        // Coleta dados em tempo real e salva no banco
        viewModelScope.launch {
            repository.getReadings().collect { reading ->
                _uiState.update { it + (reading.type to reading) }
//                saveReadingUseCase(reading)
            }
        }
    }

    fun selectSensorForHistory(type: SensorType) {
        _selectedHistorySensor.value = type
    }
}