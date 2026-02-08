#pragma once

#include <V2_0/SubHal.h>
#include <hardware/sensors.h>
#include <vector>
#include <mutex>
#include <string>

#include "io/SerialReader.h"
#include "sensors/AirQualitySensor.h"

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
 * para dados brutos vindos da camada de hardware via SerialReader.
 */
class AirQualitySubHal : public ISensorsSubHal, public IAirDataListener {
public:
    AirQualitySubHal();
    ~AirQualitySubHal();

    /**
     * @brief Inicializa a sub-HAL e registra o callback de comunicação com o sistema.
     * @param halProxyCallback Ponteiro sp para o callback de eventos da HAL Proxy.
     * @return Result::OK se inicializado com sucesso, caso contrário, código de erro.
     */
    virtual Return<Result> initialize(const sp<IHalProxyCallback>& halProxyCallback) override;

    /**
     * @brief Solicita a lista de sensores de qualidade do ar suportados.
     * @param _hidl_cb Callback de retorno contendo o vetor de SensorInfo.
     */
    virtual Return<void>   getSensorsList(getSensorsList_cb _hidl_cb) override;

    /// @brief Injeta dados de sensores externos no framework (usado para testes/depuração).
    virtual Return<Result> injectSensorData(const Event& event) override;

    /// @brief Define o modo de operação (Normal ou Data Injection).
    virtual Return<Result> setOperationMode(OperationMode mode) override;

    /**
     * @brief Ativa ou desativa um sensor específico via seu handle.
     * @param sensorHandle Identificador numérico do sensor.
     * @param enabled Define o estado: true para ativo, false para inativo.
     */
    virtual Return<Result> activate(int32_t sensorHandle, bool enabled) override;

    /**
     * @brief Configura a taxa de amostragem e latência máxima para um sensor.
     * @param sensorHandle Identificador do sensor.
     * @param samplingPeriodNs Período de amostragem em nanossegundos.
     * @param maxReportLatencyNs Latência máxima permitida para o reporte dos dados.
     */
    virtual Return<Result> batch(int32_t sensorHandle, int64_t samplingPeriodNs, int64_t maxReportLatencyNs) override;

    /**
     * @brief Solicita o esvaziamento imediato (flush) do buffer de eventos do sensor.
     * @param sensorHandle Identificador do sensor.
     */
    virtual Return<Result> flush(int32_t sensorHandle) override;
    
    /// @brief Retorna o identificador único da implementação da Sub-HAL.
    virtual const std::string getName() override { return "AirQualitySubHal"; }

    /** @name HIDL Stubs
     * Métodos obrigatórios da interface HIDL com implementação padrão ou vazia.
     */
    ///@{
    virtual Return<void>   debug(const hidl_handle& fd, const hidl_vec<hidl_string>& args) override;
    virtual Return<void>   registerDirectChannel(const SharedMemInfo& mem, registerDirectChannel_cb _hidl_cb) override;
    virtual Return<Result> unregisterDirectChannel(int32_t channelHandle) override;
    virtual Return<void>   configDirectReport(int32_t sensorHandle, int32_t channelHandle, RateLevel rate, configDirectReport_cb _hidl_cb) override;
    ///@}

    /**
     * @brief Callback acionado quando novos dados de hardware são recebidos.
     * @param data Estrutura contendo os valores lidos (PM2.5, CO2, etc).
     */
    void onDataReceived(const AirData& data) override;

private:
    /// @brief Callback para enviar eventos de volta ao framework Android.
    sp<IHalProxyCallback> mCallback;

    /// @brief Lista interna de sensores configurados nesta Sub-HAL.
    std::vector<AirQualitySensor> mSensors;

    /// @brief Objeto responsável pela leitura física da porta serial.
    SerialReader mSerialReader;

    /// @brief Mutex para garantir thread-safety nas chamadas de callback.
    std::mutex mCallbackLock;
};