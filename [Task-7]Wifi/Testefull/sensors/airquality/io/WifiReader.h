#pragma once
#include "IDataReader.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

class WifiReader : public IDataReader {
public:
    WifiReader(const std::string& ip, int port);
    ~WifiReader();

    void start() override;
    void stop() override;
    void setPollingActive(bool enabled) override;
    void setListener(IAirDataListener* listener) override;

private:
    void workerThread();
    bool connectToServer(int& sockFd);

    std::string mTargetIp;
    int mTargetPort;
    std::atomic<bool> mRunThread;
    std::atomic<bool> mActive;
    
    std::thread mThread;
    IAirDataListener* mListener;
    std::mutex mListenerLock;
};