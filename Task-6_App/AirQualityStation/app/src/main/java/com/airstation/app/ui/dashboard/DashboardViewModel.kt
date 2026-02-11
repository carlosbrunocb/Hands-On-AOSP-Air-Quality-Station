package com.airstation.app.ui.dashboard

import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.airstation.app.domain.repository.AirQualityRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class DashboardViewModel @Inject constructor(
    private val repository: AirQualityRepository
) : ViewModel() {

    init {
        viewModelScope.launch {
            repository.getReadings().collect { reading ->
                // SPRINT 1: Apenas Log para validar o fluxo completo
                Log.d("AirStationApp", "Fluxo Completo: ${reading.type} = ${reading.value} ${reading.unit}")
            }
        }
    }
}