#pragma once

#include <android/hardware/sensors/1.0/types.h>
#include <vector>
#include <string>
#include "../utils/AirData.h"

// Namespaces do Android HIDL para facilitar
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
        SENSOR_HUMID    // Oficial Android (RELATIVE_HUMIDITY)
    };

    /**
     * @param handle ID único do sensor (0, 1, 2...) gerado pela SubHAL.
     * @param type Qual métrica este sensor deve extrair do AirData.
     */
    AirQualitySensor(int32_t handle, Type type);
    
    // Destrutor padrão
    ~AirQualitySensor() = default;

    // Retorna as características do sensor (Nome, Consumo, Range...)
    const SensorInfo& getSensorInfo() const;

    // Processa o pacote de dados vindo da Serial.
    // Retorna 'true' se gerou um evento válido para enviar ao Android.
    bool processInput(const AirData& data, Event* outEvent);

    // Controle de ativação (Ligado/Desligado)
    void setActive(bool active);
    
    // Getter rápido (inline) para saber se está ativo
    bool isActive() const { return mActive; }

    // Configuração de amostragem (batching) - Simplificado para esta HAL
    void batch(int64_t samplingPeriodNs, int64_t maxReportLatencyNs);

private:
    // Variáveis de Estado (Essenciais para o .cpp funcionar)
    Type mType;         // Tipo do sensor
    bool mActive;       // Estado atual (Ligado/Desligado)
    SensorInfo mInfo;   // Estrutura de metadados do Android (Nome, Vendor, Power, etc)
};