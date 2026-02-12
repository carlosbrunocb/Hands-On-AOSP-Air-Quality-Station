package com.airstation.app.data.local.entity

import androidx.room.Entity
import androidx.room.PrimaryKey
import com.airstation.app.domain.model.SensorType

@Entity(tableName = "readings")
data class AirReadingEntity(
    @PrimaryKey(autoGenerate = true) val id: Long = 0,
    val type: SensorType,
    val value: Float,
    val timestamp: Long = System.currentTimeMillis()
)