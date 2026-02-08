#pragma once

#include <android/hardware/sensors/1.0/types.h>
#include <vector>
#include <string>
#include "../utils/AirData.h"

// Namespaces do Android HIDL para infraestrutura de sensores
using android::hardware::sensors::V1_0::SensorInfo;
using android::hardware::sensors::V1_0::SensorType;
using android::hardware::sensors::V1_0::Event;

/**
 * @class AirQualitySensor
 * @brief Representa uma unidade lógica de sensor (físico ou virtual) dentro da HAL.
 * * Esta classe é responsável por filtrar dados específicos de uma estrutura AirData
 * e convertê-los em eventos compatíveis com o Android Sensor Service.
 */
class AirQualitySensor {
public:
    /**
     * @enum Type
     * @brief Define a métrica de hardware que este objeto de sensor gerencia.
     */
    enum Type {
        SENSOR_PM25,    ///< Material Particulado 2.5
        SENSOR_PM10,    ///< Material Particulado 10
        SENSOR_CO,      ///< Monóxido de Carbono (CO)
        SENSOR_LPG,     ///< Gás Liquefeito de Petróleo / Gases Inflamáveis
        SENSOR_TEMP,    ///< Temperatura Ambiente (Mapeia para SENSOR_TYPE_AMBIENT_TEMPERATURE)
        SENSOR_HUMID,   ///< Umidade Relativa (Mapeia para SENSOR_TYPE_RELATIVE_HUMIDITY)
        SENSOR_SOURCE   ///< Serial ou Wifi
    };

    /**
     * @brief Construtor da unidade de sensor.
     * @param handle Identificador numérico único (ID) atribuído pela SubHAL.
     * @param type Tipo de métrica de hardware que o sensor representa.
     */
    AirQualitySensor(int32_t handle, Type type);
    
    // Destrutor padrão
    ~AirQualitySensor() = default;

    /// @brief Recupera os metadados estáticos do sensor (Nome, Fabricante, Consumo, etc).
    const SensorInfo& getSensorInfo() const;

    /**
     * @brief Filtra e converte o pacote de telemetria em um evento HIDL.
     * @param data Estrutura contendo todas as leituras da estação.
     * @param[out] outEvent Ponteiro para o evento que será preenchido para o Android.
     * @return true Se o sensor estiver ativo e os dados de entrada forem válidos.
     */
    bool processInput(const AirData& data, Event* outEvent);

    /// @brief Define se o sensor deve processar dados e gerar eventos.
    void setActive(bool active);
    
    /// @brief Verifica o estado de ativação atual do sensor.
    bool isActive() const { return mActive; }

    /**
     * @brief Atualiza os parâmetros de amostragem e latência do sensor.
     * @param samplingPeriodNs Intervalo entre amostras em nanossegundos.
     * @param maxReportLatencyNs Latência máxima permitida antes do reporte.
     */
    void batch(int64_t samplingPeriodNs, int64_t maxReportLatencyNs);

private:
    /// @brief Tipo interno do sensor para lógica de extração.
    Type mType;
    
    /// @brief Flag de controle de energia e processamento.
    bool mActive;
    
    /// @brief Estrutura SensorInfo preenchida conforme especificações do Android.
    SensorInfo mInfo;
};