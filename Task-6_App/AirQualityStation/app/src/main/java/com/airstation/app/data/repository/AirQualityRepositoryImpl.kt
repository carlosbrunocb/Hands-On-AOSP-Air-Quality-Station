package com.airstation.app.data.repository

import com.airstation.app.data.mapper.SensorMapper
import com.airstation.app.data.source.SystemSensorDataSource
import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.repository.AirQualityRepository
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map
import javax.inject.Inject

class AirQualityRepositoryImpl @Inject constructor(
    private val dataSource: SystemSensorDataSource,
    private val mapper: SensorMapper
) : AirQualityRepository {

    override fun getReadings(): Flow<AirQualityReading> {
        return dataSource.getSensorStream()
            .map { event -> mapper.map(event) }
    }
}