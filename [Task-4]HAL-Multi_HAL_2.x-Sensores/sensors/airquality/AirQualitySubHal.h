#pragma once

#include <V2_0/SubHal.h>
#include <hardware/sensors.h>
#include <vector>
#include <mutex>
#include <string>

#include "io/SerialReader.h"
#include "sensors/AirQualitySensor.h"

// Namespaces do Wrapper
using android::hardware::sensors::V2_0::implementation::ISensorsSubHal;
using android::hardware::sensors::V2_0::implementation::IHalProxyCallback;
using android::hardware::sensors::V2_0::implementation::ScopedWakelock;

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

class AirQualitySubHal : public ISensorsSubHal, public IAirDataListener {
public:
    AirQualitySubHal();
    ~AirQualitySubHal();

    // Métodos da Interface (V2.0 via Wrapper)
    virtual Return<Result> initialize(const sp<IHalProxyCallback>& halProxyCallback) override;
    virtual Return<void>   getSensorsList(getSensorsList_cb _hidl_cb) override;
    virtual Return<Result> injectSensorData(const Event& event) override;
    virtual Return<Result> setOperationMode(OperationMode mode) override;
    virtual Return<Result> activate(int32_t sensorHandle, bool enabled) override;
    virtual Return<Result> batch(int32_t sensorHandle, int64_t samplingPeriodNs, int64_t maxReportLatencyNs) override;
    virtual Return<Result> flush(int32_t sensorHandle) override;
    
    // Método getName (Necessário para o Wrapper)
    virtual const std::string getName() override { return "AirQualitySubHal"; }

    // Stubs obrigatórios
    virtual Return<void>   debug(const hidl_handle& fd, const hidl_vec<hidl_string>& args) override;
    virtual Return<void>   registerDirectChannel(const SharedMemInfo& mem, registerDirectChannel_cb _hidl_cb) override;
    virtual Return<Result> unregisterDirectChannel(int32_t channelHandle) override;
    virtual Return<void>   configDirectReport(int32_t sensorHandle, int32_t channelHandle, RateLevel rate, configDirectReport_cb _hidl_cb) override;

    // Callback interno
    void onDataReceived(const AirData& data) override;

private:
    sp<IHalProxyCallback> mCallback;
    std::vector<AirQualitySensor> mSensors;
    SerialReader mSerialReader;
    std::mutex mCallbackLock;
};