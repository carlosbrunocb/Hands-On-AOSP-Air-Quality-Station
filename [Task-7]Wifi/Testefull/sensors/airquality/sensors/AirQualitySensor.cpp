#define LOG_TAG "AirQualitySensor"

#include "AirQualitySensor.h"
#include <log/log.h>
#include <cmath> 

// IDs internos para identificar tipos customizados
static const int TYPE_CUST_PM25   = 0x10001; 
static const int TYPE_CUST_PM10   = 0x10002;
static const int TYPE_CUST_CO     = 0x10003;
static const int TYPE_CUST_LPG    = 0x10004;
static const int TYPE_CUST_SOURCE = 0x10005; // <-- ADICIONADO: ID do sensor de Fonte

AirQualitySensor::AirQualitySensor(int32_t handle, Type type) 
    : mType(type), mActive(false) {
    
    // Configuração Genérica
    mInfo.sensorHandle = handle;
    mInfo.vendor = "Projeto AirStation (ESP32)";
    mInfo.version = 1;
    mInfo.fifoReservedEventCount = 0;
    mInfo.fifoMaxEventCount = 0;
    mInfo.requiredPermission = "";
    mInfo.maxDelay = 1000000; // 1 segundo
    mInfo.flags = 0; // SensorMode::OnChange

    // Configuração Específica por Tipo
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

        // <-- ADICIONADO: Configuração do Sensor de Fonte
        case SENSOR_SOURCE:
            mInfo.name = "Fonte de Dados (Wi-Fi/Serial)";
            mInfo.type = (SensorType)TYPE_CUST_SOURCE; 
            mInfo.typeAsString = "com.airstation.sensor.source";
            mInfo.maxRange = 1.0f; // 0 = Serial, 1 = Wi-Fi
            mInfo.resolution = 1.0f;
            mInfo.power = 0.0f;
            break;
    }
}

const SensorInfo& AirQualitySensor::getSensorInfo() const {
    return mInfo;
}

void AirQualitySensor::setActive(bool active) {
    if (mActive != active) {
        mActive = active;
        ALOGD("Sensor %s (Handle %d) definido como: %s", 
              mInfo.name.c_str(), mInfo.sensorHandle, active ? "ATIVO" : "INATIVO");
    }
}

void AirQualitySensor::batch(int64_t /*samplingPeriodNs*/, int64_t /*maxReportLatencyNs*/) {
}

bool AirQualitySensor::processInput(const AirData& data, Event* outEvent) {
    if (!mActive || !data.valid) return false;

    float value = -1.0f;

    switch (mType) {
        case SENSOR_PM25: value = data.pm25; break;
        case SENSOR_PM10: value = data.pm10; break;
        case SENSOR_CO:   value = data.co_ppm; break;
        case SENSOR_LPG:  value = data.lpg_ppm; break;
        case SENSOR_TEMP: value = data.temp_c; break;
        case SENSOR_HUMID:value = data.humid_p; break;
        
        // <-- ADICIONADO: Traduz a string de origem para valor numérico
        case SENSOR_SOURCE: 
            value = (data.source == "wifi") ? 1.0f : 0.0f; 
            break;
    }

    if (mType == SENSOR_TEMP) {
        if (value < -273.0f) return false;
    } else if (mType != SENSOR_SOURCE) { // O sensor de fonte pode ser 0.0f (Serial)
        if (value < 0.0f) return false;
    }

    outEvent->sensorHandle = mInfo.sensorHandle;
    outEvent->sensorType = mInfo.type;
    outEvent->timestamp = data.timestamp; 
    outEvent->u.scalar = value;
    
    return true;
}