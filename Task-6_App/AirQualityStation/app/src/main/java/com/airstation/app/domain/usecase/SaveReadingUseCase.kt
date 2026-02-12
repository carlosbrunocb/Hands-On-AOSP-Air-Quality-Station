package com.airstation.app.domain.usecase

import com.airstation.app.domain.model.AirQualityReading
import com.airstation.app.domain.repository.AirQualityRepository
import javax.inject.Inject

class SaveReadingUseCase @Inject constructor(
    private val repository: AirQualityRepository
) {
    // Memória para saber quando foi o último salvamento de cada sensor
    private val lastSaved = mutableMapOf<String, Long>()
    private val INTERVAL = 5000L // Regra: Salvar a cada 5 segundos

    suspend operator fun invoke(reading: AirQualityReading) {
        val now = System.currentTimeMillis()
        val key = reading.type.name

        // Verifica se já passou o tempo
        if (now - (lastSaved[key] ?: 0L) > INTERVAL) {
            repository.saveReading(reading) // Chama o repositório para salvar
            lastSaved[key] = now // Atualiza o tempo
        }
    }
}