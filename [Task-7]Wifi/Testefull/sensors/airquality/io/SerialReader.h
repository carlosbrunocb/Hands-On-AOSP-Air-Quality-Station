#pragma once
#include "IDataReader.h" // <--- Mudança Principal
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

// Herda de IDataReader
class SerialReader : public IDataReader {
public:
    SerialReader(const std::string& devicePath);
    ~SerialReader();

    // Overrides obrigatórios
    void start() override;
    void stop() override;
    void setPollingActive(bool enabled) override;
    void setListener(IAirDataListener* listener) override;

private:
    void workerThread();
    bool configureSerial(int fd);
    std::string findSerialDevice();

    std::string mDevicePath;
    std::atomic<bool> mRunThread;
    std::atomic<bool> mPollingActive;
    std::thread mThread;
    IAirDataListener* mListener;
    std::mutex mListenerLock;
};