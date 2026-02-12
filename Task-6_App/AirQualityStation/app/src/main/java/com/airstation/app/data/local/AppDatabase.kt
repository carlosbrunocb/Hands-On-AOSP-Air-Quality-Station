package com.airstation.app.data.local

import androidx.room.Database
import androidx.room.RoomDatabase
import androidx.room.TypeConverter
import androidx.room.TypeConverters
import com.airstation.app.data.local.entity.AirReadingEntity
import com.airstation.app.domain.model.SensorType

class Converters {
    @TypeConverter fun fromEnum(v: SensorType) = v.name
    @TypeConverter fun toEnum(v: String) = SensorType.valueOf(v)
}

// CORREÇÃO AQUI: Adicione exportSchema = false
@Database(entities = [AirReadingEntity::class], version = 1, exportSchema = false)
@TypeConverters(Converters::class)
abstract class AppDatabase : RoomDatabase() {
    abstract fun readingDao(): ReadingDao
}