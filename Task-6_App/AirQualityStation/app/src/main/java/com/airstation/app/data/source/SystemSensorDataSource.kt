package com.airstation.app.data.source

import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.util.Log
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import javax.inject.Inject

class SystemSensorDataSource @Inject constructor(
    private val sensorManager: SensorManager
) {
    fun getSensorStream(): Flow<SensorEvent> = callbackFlow {
        Log.e("AirStationHAL", ">>> INICIANDO A BUSCA POR SENSORES <<<")
        val listener = object : SensorEventListener {
            override fun onSensorChanged(event: SensorEvent?) {
                event?.let { trySend(it) }
            }
            override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
        }

        val sensorsToWatch = listOf(
            SensorConfig.TYPE_PM25, SensorConfig.TYPE_PM10,
            SensorConfig.TYPE_CO, SensorConfig.TYPE_LPG,
            SensorConfig.TYPE_TEMP, SensorConfig.TYPE_HUMID
        )

        sensorsToWatch.forEach { type ->
            val sensor = sensorManager.getDefaultSensor(type)
            if (sensor != null) {
                Log.i("AirStationHAL", "✔ Sensor conectado: ${sensor.name} (ID: $type)")
                sensorManager.registerListener(listener, sensor, SensorManager.SENSOR_DELAY_UI)
            } else {
                // Mudei para Log.e (Erro) para destacar bem
                Log.e("AirStationHAL", "❌ Sensor NÃO encontrado no Android: ID $type")

                // TENTATIVA DE DEBUG: Listar o que existe
                if (type == SensorConfig.TYPE_PM25) {
                    Log.e("AirStationHAL", "Lista de sensores disponíveis no sistema:")
                    sensorManager.getSensorList(Sensor.TYPE_ALL).forEach {
                        Log.e("AirStationHAL", " - ${it.name} (Type: ${it.type})")
                    }
                }
            }
        }

        awaitClose { sensorManager.unregisterListener(listener) }
    }
}