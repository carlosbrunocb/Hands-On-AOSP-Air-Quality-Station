#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include "../utils/AirData.h"

/**
 * @class IAirDataListener
 * @brief Interface de callback para o recebimento de dados de telemetria.
 * * Implementada pela Sub-HAL para processar as estruturas AirData enviadas
 * pela SerialReader de forma assíncrona.
 */
class IAirDataListener {
public:
    virtual ~IAirDataListener() = default;

    /**
     * @brief Evento disparado quando um novo pacote de dados é processado.
     * @param data Estrutura contendo as leituras validadas dos sensores.
     */
    virtual void onDataReceived(const AirData& data) = 0;
};

/**
 * @class SerialReader
 * @brief Gerencia a comunicação serial e o ciclo de vida da thread de leitura.
 * * Esta classe encapsula a lógica de baixo nível para leitura de dispositivos 
 * tty (como /dev/ttyUSB0), realizando o parse das mensagens em uma thread separada.
 */
class SerialReader {
public:
    /**
     * @brief Inicializa o leitor mas não abre o dispositivo imediatamente.
     * @param devicePath Caminho absoluto do dispositivo serial (ex: "/dev/ttyUSB0").
     */
    SerialReader(const std::string& devicePath);
    ~SerialReader();

    /// @brief Inicia a execução da thread de trabalho (workerThread).
    void start();

    /// @brief Sinaliza a parada da thread e aguarda o encerramento (join).
    void stop();
    
    /**
     * @brief Alterna o estado de polling da estação.
     * @details Permite silenciar o hardware (ESP32) quando não há sensores ativos na HAL,
     * economizando energia e tráfego de barramento.
     * @param enabled True para permitir o envio de comandos de solicitação de dados.
     */
    void setPollingActive(bool enabled);

    /**
     * @brief Registra o listener que processará os dados recebidos.
     * @param listener Ponteiro para a implementação da interface de callback.
     */
    void setListener(IAirDataListener* listener);

private:
    /**
     * @brief Função de loop da thread. Realiza a leitura contínua e o parse.
     */
    void workerThread();

    /**
     * @brief Aplica as configurações de termios (baudrate, parity, etc) ao file descriptor.
     * @param fd Descriptor do arquivo serial aberto.
     * @return true se a configuração foi aplicada com sucesso.
     */
    bool configureSerial(int fd);

    /// @brief Tenta localizar o dispositivo serial se o caminho original falhar.
    std::string findSerialDevice();

    // Atributos de Configuração
    std::string mDevicePath; ///< Caminho do dispositivo serial.
    
    // Controles de thread - Primitivas de Sincronização e Concorrência
    std::atomic<bool> mRunThread;     ///< Flag atômica para controle de execução da thread.
    std::atomic<bool> mPollingActive; ///< Flag atômica para controle de fluxo de comandos.
    std::thread mThread; ///< Flag atômica para controle de fluxo de comandos.
    
    // Callback e Proteção
    IAirDataListener* mListener; ///< Ponteiro para o objeto interessado nos dados.
    std::mutex mListenerLock;    ///< Mutex para proteção do acesso ao mListener.
};