#pragma once

#include <android/hardware/sensors/1.0/types.h>
#include <V2_0/SubHal.h>
#include <hardware/sensors.h>
#include <vector>
#include <mutex>
#include <string>

#include "io/SerialReader.h"
#include "io/WifiReader.h" // <-- ADICIONADO
#include "sensors/AirQualitySensor.h"
#include "../utils/AirData.h"

/**
 * Representa um sensor individual (físico ou virtual) gerenciado pela HAL.
 */
class AirQualitySensor {
public:
    // Enumeração interna para identificar qual dado este objeto processa
    enum Type {s
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

/** * @name Namespaces de Implementação (Wrapper)
 * @{ 
 * Definições de tipos para a estrutura de execução da Sub-HAL de sensores.
 */
using android::hardware::sensors::V2_0::implementation::ISensorsSubHal;
using android::hardware::sensors::V2_0::implementation::IHalProxyCallback;
using android::hardware::sensors::V2_0::implementation::ScopedWakelock;
/** @} */

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_handle;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::sp;

using android::hardware::sensors::V1_0::Result;
using android::hardware::sensors::V1_0::OperationMode;
using android::hardware::sensors::V1_0::SensorInfo;
using android::hardware::sensors::V1_0::RateLevel;
using android::hardware::sensors::V1_0::SharedMemInfo;
using android::hardware::sensors::V1_0::Event; 

/**
 * @class AirQualitySubHal
 * @brief Implementação da Sub-HAL de Sensores para monitoramento de qualidade do ar.
 * * Esta classe estende a interface ISensorsSubHal (V2.0) e atua como um listener
 * para dados brutos vindos da camada de hardware via SerialReader e WifiReader.
 */
class AirQualitySubHal : public ISensorsSubHal, public IAirDataListener {
public:
    AirQualitySubHal();
    ~AirQualitySubHal();

    virtual Return<Result> initialize(const sp<IHalProxyCallback>& halProxyCallback) override;
    virtual Return<void>   getSensorsList(getSensorsList_cb _hidl_cb) override;
    virtual Return<Result> injectSensorData(const Event& event) override;
    virtual Return<Result> setOperationMode(OperationMode mode) override;
    virtual Return<Result> activate(int32_t sensorHandle, bool enabled) override;
    virtual Return<Result> batch(int32_t sensorHandle, int64_t samplingPeriodNs, int64_t maxReportLatencyNs) override;
    virtual Return<Result> flush(int32_t sensorHandle) override;
    virtual const std::string getName() override { return "AirQualitySubHal"; }

    ///@{
    virtual Return<void>   debug(const hidl_handle& fd, const hidl_vec<hidl_string>& args) override;
    virtual Return<void>   registerDirectChannel(const SharedMemInfo& mem, registerDirectChannel_cb _hidl_cb) override;
    virtual Return<Result> unregisterDirectChannel(int32_t channelHandle) override;
    virtual Return<void>   configDirectReport(int32_t sensorHandle, int32_t channelHandle, RateLevel rate, configDirectReport_cb _hidl_cb) override;
    ///@}

    void onDataReceived(const AirData& data) override;

private:
    sp<IHalProxyCallback> mCallback;
    std::vector<AirQualitySensor> mSensors;
    
    SerialReader mSerialReader;
    WifiReader mWifiReader; // <-- ADICIONADO: O Leitor de Rede

    std::mutex mCallbackLock;
};