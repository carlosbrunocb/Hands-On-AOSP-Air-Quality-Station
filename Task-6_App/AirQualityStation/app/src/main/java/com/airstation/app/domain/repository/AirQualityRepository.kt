package com.airstation.app.domain.repository

import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.model.SensorType
import kotlinx.coroutines.flow.Flow

interface AirQualityRepository {

    // Método da Sprint 1 (Leitura em Tempo Real)
    fun getReadings(): Flow<AirQualityReading>

    // --- NOVOS MÉTODOS DA SPRINT 3 ---

    // Salvar uma leitura no banco (Suspend porque é operação de I/O)
    suspend fun saveReading(reading: AirQualityReading)

    // Buscar histórico para gráficos
    fun getHistory(type: SensorType, lastMillis: Long): Flow<List<AirQualityReading>>
}