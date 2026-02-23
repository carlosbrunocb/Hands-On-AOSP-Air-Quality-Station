#pragma once

#include <android/hardware/sensors/1.0/types.h>
#include <vector>
#include <string>
#include "../utils/AirData.h"

using android::hardware::sensors::V1_0::SensorInfo;
using android::hardware::sensors::V1_0::SensorType;
using android::hardware::sensors::V1_0::Event;

/**
 * Representa um sensor individual (físico ou virtual) gerenciado pela HAL.
 */
class AirQualitySensor {
public:
    // Enumeração interna para identificar qual dado este objeto processa
    enum Type {
        SENSOR_PM25,    // Customizado
        SENSOR_PM10,    // Customizado
        SENSOR_CO,      // Customizado
        SENSOR_LPG,     // Customizado
        SENSOR_TEMP,    // Oficial Android (AMBIENT_TEMPERATURE)
        SENSOR_HUMID,   // Oficial Android (RELATIVE_HUMIDITY)
        SENSOR_SOURCE   // <-- ADICIONADO: Fonte de Dados (Wi-Fi ou Serial)
    };

    /**
     * @param handle ID único do sensor (0, 1, 2...) gerado pela SubHAL.
     * @param type Qual métrica este sensor deve extrair do AirData.
     */
    AirQualitySensor(int32_t handle, Type type);
    
    ~AirQualitySensor() = default;

    const SensorInfo& getSensorInfo() const;
    bool processInput(const AirData& data, Event* outEvent);
    void setActive(bool active);
    bool isActive() const { return mActive; }
    void batch(int64_t samplingPeriodNs, int64_t maxReportLatencyNs);

private:
    Type mType;         // Tipo do sensor
    bool mActive;       // Estado atual (Ligado/Desligado)
    SensorInfo mInfo;   // Estrutura de metadados do Android
};