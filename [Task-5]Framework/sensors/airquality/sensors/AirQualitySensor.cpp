#define LOG_TAG "AirQualitySensor"

#include "AirQualitySensor.h"
#include <log/log.h>
#include <cmath> 

/** @name Definições de Tipos Customizados
 * Conforme a documentação do Android, tipos de sensores não oficiais devem 
 * iniciar acima de SENSOR_TYPE_DEVICE_PRIVATE_BASE (0x10000).
 */
///@{
static const int TYPE_CUST_PM25   = 0x10001; 
static const int TYPE_CUST_PM10   = 0x10002;
static const int TYPE_CUST_CO     = 0x10003;
static const int TYPE_CUST_LPG    = 0x10004;
static const int TYPE_CUST_SOURCE = 0x10005;
///@}

// Uso do cast do enum oficial do Android para Temp/Humid
// SENSOR_TYPE_AMBIENT_TEMPERATURE = 13
// SENSOR_TYPE_RELATIVE_HUMIDITY = 12

/**
 * @brief Construtor especializado que configura as capacidades do sensor.
 * @details Define metadados como range, resolução e consumo de energia (mA) 
 * baseados no datasheet dos sensores SDS011, MQ e DHT.
 */
AirQualitySensor::AirQualitySensor(int32_t handle, Type type) 
    : mType(type), mActive(false) {
    
    // Configuração Genérica
    mInfo.sensorHandle = handle;
    mInfo.vendor = "Projeto AirStation (ESP32)";
    mInfo.version = 1;
    mInfo.fifoReservedEventCount = 0;
    mInfo.fifoMaxEventCount = 0;
    mInfo.requiredPermission = "";
    mInfo.maxDelay = 1000000; // 1 segundo (em microssegundos)
    mInfo.flags = 0;          // Modo de reporte contínuo/on-change

    // Configuração Específica por Perfil de Sensor
    switch (mType) {
        case SENSOR_PM25:
            mInfo.name = "Sensor de Particulas PM2.5";
            mInfo.type = (SensorType)TYPE_CUST_PM25; 
            mInfo.typeAsString = "com.airstation.sensor.pm25";
            mInfo.maxRange = 999.9f;
            mInfo.resolution = 0.1f;
            mInfo.power = 0.5f; 
            break;

        case SENSOR_PM10:
            mInfo.name = "Sensor de Particulas PM10";
            mInfo.type = (SensorType)TYPE_CUST_PM10;
            mInfo.typeAsString = "com.airstation.sensor.pm10";
            mInfo.maxRange = 999.9f;
            mInfo.resolution = 0.1f;
            mInfo.power = 0.5f;
            break;

        case SENSOR_CO:
            mInfo.name = "Sensor de Monoxido de Carbono (CO)";
            mInfo.type = (SensorType)TYPE_CUST_CO;
            mInfo.typeAsString = "com.airstation.sensor.co";
            mInfo.maxRange = 1000.0f;
            mInfo.resolution = 0.1f;
            mInfo.power = 0.8f; 
            break;

        case SENSOR_LPG:
            mInfo.name = "Sensor de Gas Inflamavel (LPG)";
            mInfo.type = (SensorType)TYPE_CUST_LPG;
            mInfo.typeAsString = "com.airstation.sensor.lpg";
            mInfo.maxRange = 10000.0f;
            mInfo.resolution = 1.0f;
            mInfo.power = 0.8f;
            break;

        case SENSOR_TEMP:
            mInfo.name = "Termometro Ambiente (DHT)";
            mInfo.type = SensorType::AMBIENT_TEMPERATURE; 
            mInfo.typeAsString = "android.sensor.ambient_temperature"; 
            mInfo.maxRange = 80.0f;
            mInfo.resolution = 0.1f;
            mInfo.power = 0.1f;
            break;

        case SENSOR_HUMID:
            mInfo.name = "Umidade Relativa (DHT)";
            mInfo.type = SensorType::RELATIVE_HUMIDITY;
            mInfo.typeAsString = "android.sensor.relative_humidity";
            mInfo.maxRange = 100.0f;
            mInfo.resolution = 0.1f;
            mInfo.power = 0.1f;
            break;

        case SENSOR_SOURCE:
            mInfo.name = "Air Quality Data Source";
            mInfo.type = (SensorType)TYPE_CUST_SOURCE;
            mInfo.typeAsString = "com.airstation.sensor.air_quality_source";
            mInfo.maxRange = 1.0f;
            mInfo.resolution = 1.0f;
            mInfo.power = 0.0f;
            mInfo.flags = 0x2; // SENSOR_FLAG_ON_CHANGE_MODE
            break;
    }
}

/**
 * @brief Recupera a estrutura de metadados estáticos do sensor.
 * @return Referência constante para SensorInfo, contendo nome, tipo e capacidades.
 */
const SensorInfo& AirQualitySensor::getSensorInfo() const {
    return mInfo;
}

/**
 * @brief Altera o estado de ativação do sensor.
 * @details Quando desativado, o sensor interrompe o processamento de inputs no método 
 * processInput(), economizando ciclos de CPU da HAL.
 * @param active True para habilitar o sensor, False para ignorar dados.
 */
void AirQualitySensor::setActive(bool active) {
    if (mActive != active) {
        mActive = active;
        ALOGD("Sensor %s (Handle %d) definido como: %s", 
              mInfo.name.c_str(), mInfo.sensorHandle, active ? "ATIVO" : "INATIVO");
    }
}

/// @note O batching não é suportado por este hardware; os dados são reportados em tempo real.
void AirQualitySensor::batch(int64_t /*samplingPeriodNs*/, int64_t /*maxReportLatencyNs*/) {
    // Implementação vazia intencional
}

/**
 * @brief Executa o processamento do payload e validação de limites físicos.
 * @details Realiza a extração do valor correto de AirData e descarta amostras 
 * que não condizem com a realidade física (ex: temperaturas abaixo do zero absoluto).
 */
bool AirQualitySensor::processInput(const AirData& data, Event* outEvent) {
    if (!mActive || !data.valid) return false;

    float value = -1.0f;
    bool updated = true;

    switch (mType) {
        case SENSOR_PM25:   value = data.pm25; break;
        case SENSOR_PM10:   value = data.pm10; break;
        case SENSOR_CO:     value = data.co_ppm; break;
        case SENSOR_LPG:    value = data.lpg_ppm; break;
        case SENSOR_TEMP:   value = data.temp_c; break;
        case SENSOR_HUMID:  value = data.humid_p; break;
        case SENSOR_SOURCE: value = (data.source == "wifi") ? 1.0f : 0.0f; break;
        default: updated = false; break;
    }

    if (!updated) return false;

    // Filtros de Sanidade (Evitar enviar lixo se o sensor falhar)
    // Temperatura absoluta zero é impossível, então é erro de leitura
    if (mType == SENSOR_TEMP && value < -273.0f) return false;
    // PM e Gás não podem ser negativos
    if (mType != SENSOR_TEMP && value < 0.0f) return false;

    outEvent->sensorHandle = mInfo.sensorHandle;
    outEvent->sensorType = mInfo.type;
    outEvent->timestamp = data.timestamp; 
    outEvent->u.scalar = value;
    
    return true;
}