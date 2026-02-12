package com.airstation.app.data.repository

import com.airstation.app.data.local.ReadingDao
import com.airstation.app.data.local.entity.AirReadingEntity
import com.airstation.app.data.mapper.SensorMapper
import com.airstation.app.data.source.SystemSensorDataSource
import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.model.SensorType
import com.airstation.app.domain.repository.AirQualityRepository
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map
import javax.inject.Inject

class AirQualityRepositoryImpl @Inject constructor(
    private val dataSource: SystemSensorDataSource,
    private val dao: ReadingDao,
    private val mapper: SensorMapper
) : AirQualityRepository {

    // 1. REALTIME: Vem do DataSource (HAL) -> Passa pelo Mapper -> Vai para UI
    override fun getReadings(): Flow<AirQualityReading> {
        return dataSource.getSensorStream()
            .map { event -> mapper.map(event) }
    }

    // 2. SALVAR: Vem do UseCase -> Converte para Entity -> Salva no Banco
    override suspend fun saveReading(reading: AirQualityReading) {
        dao.insertReading(
            AirReadingEntity(type = reading.type, value = reading.value)
        )
    }

    // 3. HISTÓRICO: Vem do Banco -> Converte para Domain -> Vai para Gráfico
    override fun getHistory(type: SensorType, lastMillis: Long): Flow<List<AirQualityReading>> {
        val cutoff = System.currentTimeMillis() - lastMillis

        return dao.getReadingsBySensor(type, cutoff).map { list ->
            list.map { entity ->
                AirQualityReading(
                    type = entity.type,
                    value = entity.value,
                    unit = getUnitForType(entity.type) // Helper simples abaixo
                )
            }
        }
    }

    // Pequena função auxiliar para recuperar a unidade correta ao ler do banco
    private fun getUnitForType(type: SensorType): String {
        return when (type) {
            SensorType.TEMPERATURE -> "°C"
            SensorType.HUMIDITY -> "%"
            SensorType.CO, SensorType.LPG -> "ppm"
            else -> "µg/m³"
        }
    }
}