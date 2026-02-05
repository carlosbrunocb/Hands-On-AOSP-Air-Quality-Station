#define LOG_TAG "AirQualitySubHal"

#include "AirQualitySubHal.h"
#include <log/log.h>
#include <hardware/sensors.h>

using android::hardware::sensors::V1_0::MetaDataEventType;
using android::hardware::sensors::V1_0::SensorType;

// Handles Internos 
static const int32_t HANDLE_PM25  = 5001;
static const int32_t HANDLE_PM10  = 5002;
static const int32_t HANDLE_CO    = 5003;
static const int32_t HANDLE_LPG   = 5004;
static const int32_t HANDLE_TEMP  = 5005;
static const int32_t HANDLE_HUMID = 5006;

AirQualitySubHal::AirQualitySubHal() 
    : mSerialReader("/dev/ttyUSB0") { 
    
    // Instancia os sensores individuais
    mSensors.emplace_back(HANDLE_PM25,  AirQualitySensor::SENSOR_PM25);
    mSensors.emplace_back(HANDLE_PM10,  AirQualitySensor::SENSOR_PM10);
    mSensors.emplace_back(HANDLE_CO,    AirQualitySensor::SENSOR_CO);
    mSensors.emplace_back(HANDLE_LPG,   AirQualitySensor::SENSOR_LPG);
    mSensors.emplace_back(HANDLE_TEMP,  AirQualitySensor::SENSOR_TEMP);
    mSensors.emplace_back(HANDLE_HUMID, AirQualitySensor::SENSOR_HUMID);
}

AirQualitySubHal::~AirQualitySubHal() {
    mSerialReader.stop();
}

Return<Result> AirQualitySubHal::initialize(const sp<IHalProxyCallback>& halProxyCallback) {
    ALOGI("AirQualitySubHal: Inicializando Callback do Framework...");
    {
        std::lock_guard<std::mutex> lock(mCallbackLock);
        mCallback = halProxyCallback;
    }
    mSerialReader.setListener(this);
    mSerialReader.start();
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
            ScopedWakelock wakelock = mCallback->createScopedWakelock(true);
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
            ScopedWakelock wakelock = mCallback->createScopedWakelock(true);
            mCallback->postEvents(events, std::move(wakelock));
        }
    }
    return Result::OK;
}

// Stubs
Return<Result> AirQualitySubHal::setOperationMode(OperationMode) { return Result::OK; }
Return<void>   AirQualitySubHal::debug(const hidl_handle&, const hidl_vec<hidl_string>&) { return Void(); }
Return<void>   AirQualitySubHal::registerDirectChannel(const SharedMemInfo&, registerDirectChannel_cb _hidl_cb) { _hidl_cb(Result::INVALID_OPERATION, -1); return Void(); }
Return<Result> AirQualitySubHal::unregisterDirectChannel(int32_t) { return Result::INVALID_OPERATION; }
Return<void>   AirQualitySubHal::configDirectReport(int32_t, int32_t, RateLevel, configDirectReport_cb _hidl_cb) { _hidl_cb(Result::INVALID_OPERATION, 0); return Void(); }

// --- ENTRY POINT MODERNO (ANDROID 13/14/LINEAGE 20+) ---

// extern "C" {

//     // A assinatura DEVE ser sensorsHalGetSubHal_2_0
//     // Não recebe parâmetros. Retorna ponteiro direto para a Interface.
//     __attribute__((visibility("default")))
//     android::hardware::sensors::V2_0::ISensorsSubHal* sensorsHalGetSubHal_2_0() {
        
//         ALOGD("AirQualitySubHal: Entry Point 2.0 (MODERNO) chamado!");

//         // Singleton: A variável 'subHal' é criada apenas na primeira vez que essa função roda.
//         // Isso impede que a gente tente abrir a porta Serial (/dev/ttyUSB0) múltiplas vezes.
//         static AirQualitySubHal subHal;
        
//         // Retorna o endereço da instância estática
//         return &subHal;
//     }

// }

extern "C" {

    // A assinatura DEVE ser sensorsHalGetSubHal (sem sufixo _2_0)
    // Retorna ponteiro da IMPLEMENTATION (não da interface pura) para casar com o tipo da classe
    __attribute__((visibility("default")))
    android::hardware::sensors::V2_0::implementation::ISensorsSubHal* sensorsHalGetSubHal(uint32_t* version) {
        
        ALOGD("AirQualitySubHal: Entry Point CHAMADO! Endereço version: %p", version);

        // OBRIGATÓRIO: Escrever a versão 2 no ponteiro. 
        // O HalProxy verifica: if (version != 2) -> ERRO.
        if (version != nullptr) {
            // *version = 2; // SUB_HAL_2_0_VERSION
            *version = SUB_HAL_2_0_VERSION;
            ALOGD("AirQualitySubHal: Versão reportada: %u", *version);
        } else {
            ALOGE("AirQualitySubHal: ERRO CRÍTICO - Ponteiro version é nulo!");
        }
        
        // Singleton: Garante uma única instância (seguro para Serial)
        static AirQualitySubHal subHal;
        return &subHal;
    }

}

// #include <android/hardware/sensors/2.0/types.h>

// using android::hardware::sensors::V2_0::implementation::ISensorsSubHal;
// using android::hardware::sensors::V2_0::SUB_HAL_VERSION_2_0;

// static AirQualitySubHal gSubHal;

// extern "C"
// __attribute__((visibility("default")))
// ISensorsSubHal* sensorsHalGetSubHal(uint32_t* version) {

//     ALOGD("AirQualitySubHal: sensorsHalGetSubHal() chamado");

//     if (version != nullptr) {
//         *version = SUB_HAL_VERSION_2_0;
//     }

//     return &gSubHal;
// }