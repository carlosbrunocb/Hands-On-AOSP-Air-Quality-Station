#define LOG_TAG "AirQualitySubHal"

#include "AirQualitySubHal.h"
#include <log/log.h>
#include <hardware/sensors.h>
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

using android::hardware::sensors::V1_0::MetaDataEventType;
using android::hardware::sensors::V1_0::SensorType;

/** @name Identificadores Internos de Sensores (Handles)
 * @{ */
static const int32_t HANDLE_PM25  = 5001; ///< Material Particulado 2.5
static const int32_t HANDLE_PM10  = 5002; ///< Material Particulado 10
static const int32_t HANDLE_CO    = 5003; ///< Monóxido de Carbono
static const int32_t HANDLE_LPG   = 5004; ///< Gás Liquefeito de Petróleo
static const int32_t HANDLE_TEMP  = 5005; ///< Temperatura Ambiente
static const int32_t HANDLE_HUMID = 5006; ///< Humidade Relativa
static const int32_t HANDLE_SRC   = 5099; ///< Serial ou Wifi
/** @} */ 

/**
 * @brief Construtor: Inicializa leitores e mapeia sensores virtuais.
 */
AirQualitySubHal::AirQualitySubHal() 
    : mSerialReader("/dev/ttyACM0"),      // Mantive a serial que funcionou no Emulador
      mWifiReader("192.168.1.219", 8080) { // <-- ADICIONADO: IP do ESP32
    
    mSensors.emplace_back(HANDLE_PM25,  AirQualitySensor::SENSOR_PM25);
    mSensors.emplace_back(HANDLE_PM10,  AirQualitySensor::SENSOR_PM10);
    mSensors.emplace_back(HANDLE_CO,    AirQualitySensor::SENSOR_CO);
    mSensors.emplace_back(HANDLE_LPG,   AirQualitySensor::SENSOR_LPG);
    mSensors.emplace_back(HANDLE_TEMP,  AirQualitySensor::SENSOR_TEMP);
    mSensors.emplace_back(HANDLE_HUMID, AirQualitySensor::SENSOR_HUMID);
    mSensors.emplace_back(HANDLE_SRC,   AirQualitySensor::SENSOR_SOURCE);
}

AirQualitySubHal::~AirQualitySubHal() {
    mSerialReader.stop();
    mWifiReader.stop(); // <-- ADICIONADO
}

Return<Result> AirQualitySubHal::initialize(const sp<IHalProxyCallback>& halProxyCallback) {
    ALOGI("AirQualitySubHal: Inicializando Callback do Framework...");
    {
        std::lock_guard<std::mutex> lock(mCallbackLock);
        mCallback = halProxyCallback;
    }
    mSerialReader.setListener(this);
    mWifiReader.setListener(this); // <-- ADICIONADO

    mSerialReader.start();
    mWifiReader.start(); // <-- ADICIONADO
    
    return Result::OK;
}

Return<void> AirQualitySubHal::getSensorsList(getSensorsList_cb _hidl_cb) {
    std::vector<SensorInfo> sensors;
    for (const auto& sensor : mSensors) {
        sensors.push_back(sensor.getSensorInfo());
    }
    _hidl_cb(sensors);
    return Void();
}

Return<Result> AirQualitySubHal::activate(int32_t sensorHandle, bool enabled) {
    bool anyActive = false;
    for (auto& sensor : mSensors) {
        if (sensor.getSensorInfo().sensorHandle == sensorHandle) {
            sensor.setActive(enabled);
        }
        if (sensor.isActive()) anyActive = true;
    }
    mSerialReader.setPollingActive(anyActive);
    mWifiReader.setPollingActive(anyActive); // <-- ADICIONADO
    
    return Result::OK;
}

Return<Result> AirQualitySubHal::batch(int32_t sensorHandle, int64_t samplingPeriodNs, int64_t maxReportLatencyNs) {
    for (auto& sensor : mSensors) {
        if (sensor.getSensorInfo().sensorHandle == sensorHandle) {
            sensor.batch(samplingPeriodNs, maxReportLatencyNs);
            return Result::OK;
        }
    }
    return Result::BAD_VALUE;
}

void AirQualitySubHal::onDataReceived(const AirData& data) {
    std::vector<Event> events;
    for (auto& sensor : mSensors) {
        if (sensor.isActive()) {
            Event event;
            if (sensor.processInput(data, &event)) {
                events.push_back(event);
            }
        }
    }
    if (!events.empty()) {
        std::lock_guard<std::mutex> lock(mCallbackLock);
        if (mCallback != nullptr) {
            ScopedWakelock wakelock = mCallback->createScopedWakelock(false); // Ajustado para false
            mCallback->postEvents(events, std::move(wakelock));
        }
    }
}

Return<Result> AirQualitySubHal::injectSensorData(const Event& /*event*/) { return Result::INVALID_OPERATION; }

Return<Result> AirQualitySubHal::flush(int32_t sensorHandle) {
    Event event;
    event.sensorHandle = sensorHandle;
    event.sensorType   = SensorType::META_DATA;
    event.u.meta.what  = MetaDataEventType::META_DATA_FLUSH_COMPLETE;
    std::vector<Event> events;
    events.push_back(event);
    {
        std::lock_guard<std::mutex> lock(mCallbackLock);
        if (mCallback != nullptr) {
            ScopedWakelock wakelock = mCallback->createScopedWakelock(false); // Ajustado para false
            mCallback->postEvents(events, std::move(wakelock));
        }
    }
    return Result::OK;
}

Return<Result> AirQualitySubHal::setOperationMode(OperationMode) { return Result::OK; }
Return<void>   AirQualitySubHal::debug(const hidl_handle&, const hidl_vec<hidl_string>&) { return Void(); }
Return<void>   AirQualitySubHal::registerDirectChannel(const SharedMemInfo&, registerDirectChannel_cb _hidl_cb) { _hidl_cb(Result::INVALID_OPERATION, -1); return Void(); }
Return<Result> AirQualitySubHal::unregisterDirectChannel(int32_t) { return Result::INVALID_OPERATION; }
Return<void>   AirQualitySubHal::configDirectReport(int32_t, int32_t, RateLevel, configDirectReport_cb _hidl_cb) { _hidl_cb(Result::INVALID_OPERATION, 0); return Void(); }

extern "C" {
    __attribute__((visibility("default")))
    android::hardware::sensors::V2_0::implementation::ISensorsSubHal* sensorsHalGetSubHal(uint32_t* version) {
        ALOGD("AirQualitySubHal: Entry Point CHAMADO! Endereço version: %p", version);
        if (version != nullptr) {
            *version = SUB_HAL_2_0_VERSION;
            ALOGD("AirQualitySubHal: Versão reportada: %u", *version);
        } else {
            ALOGE("AirQualitySubHal: ERRO CRÍTICO - Ponteiro version é nulo!");
        }
        
        static AirQualitySubHal subHal;
        return &subHal;
    }
}