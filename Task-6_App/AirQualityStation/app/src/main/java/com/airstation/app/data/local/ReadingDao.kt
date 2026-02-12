package com.airstation.app.data.local

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.Query
import com.airstation.app.data.local.entity.AirReadingEntity
import com.airstation.app.domain.model.SensorType
import kotlinx.coroutines.flow.Flow

@Dao
interface ReadingDao {

    // O Repositório chama "insertReading", então o nome aqui deve ser igual
    @Insert
    suspend fun insertReading(reading: AirReadingEntity)

    // O Repositório chama "getReadingsBySensor", então o nome aqui deve ser igual
    @Query("SELECT * FROM readings WHERE type = :sensorType AND timestamp >= :minTimestamp ORDER BY timestamp ASC")
    fun getReadingsBySensor(sensorType: SensorType, minTimestamp: Long): Flow<List<AirReadingEntity>>
}