#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include "../utils/AirData.h"

class IAirDataListener {
public:
    virtual ~IAirDataListener() = default;
    virtual void onDataReceived(const AirData& data) = 0;
};

class SerialReader {
public:
    // Construtor
    SerialReader(const std::string& devicePath);
    ~SerialReader();

    // Controle da Thread
    void start();
    void stop();
    
    // NOVO: Define se devemos incomodar o ESP32 ou ficar em silêncio
    void setPollingActive(bool enabled);

    // Define quem recebe os dados (a SubHal)
    void setListener(IAirDataListener* listener);

private:
    void workerThread();
    bool configureSerial(int fd);
    std::string findSerialDevice();

    std::string mDevicePath;
    
    // Controles de thread
    std::atomic<bool> mRunThread;     // Thread viva?
    std::atomic<bool> mPollingActive; // Devemos enviar comandos?

    std::thread mThread;
    
    // Callback
    IAirDataListener* mListener;
    std::mutex mListenerLock;
};