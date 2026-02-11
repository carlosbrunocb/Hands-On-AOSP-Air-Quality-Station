package com.airstation.app.di

import android.content.Context
import android.hardware.SensorManager
import com.airstation.app.data.repository.AirQualityRepositoryImpl
import com.airstation.app.domain.repository.AirQualityRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object SensorModule {

    @Provides
    @Singleton
    fun provideSensorManager(@ApplicationContext context: Context): SensorManager {
        return context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    }

    // Vincula a Interface à Implementação
    @Provides
    @Singleton
    fun provideAirQualityRepository(impl: AirQualityRepositoryImpl): AirQualityRepository {
        return impl
    }
}