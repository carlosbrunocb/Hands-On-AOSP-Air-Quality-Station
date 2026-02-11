package com.airstation.app.domain.repository

import com.airstation.app.domain.model.AirQualityReading
import kotlinx.coroutines.flow.Flow

interface AirQualityRepository {
    fun getReadings(): Flow<AirQualityReading>
}