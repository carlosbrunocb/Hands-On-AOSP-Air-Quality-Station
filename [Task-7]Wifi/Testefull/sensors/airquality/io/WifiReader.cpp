#define LOG_TAG "AirQualityWifi"
#include "WifiReader.h"
#include "../utils/JsonParser.h"
#include <log/log.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

WifiReader::WifiReader(const std::string& ip, int port)
    : mTargetIp(ip), mTargetPort(port), mRunThread(false), mActive(false), mListener(nullptr) {}

WifiReader::~WifiReader() { stop(); }

void WifiReader::setListener(IAirDataListener* listener) {
    std::lock_guard<std::mutex> lock(mListenerLock);
    mListener = listener;
}

void WifiReader::setPollingActive(bool enabled) {
    mActive = enabled;
    ALOGD("WifiReader: Status %s", enabled ? "ATIVO" : "STANDBY");
}

void WifiReader::start() {
    if (mRunThread) return;
    mRunThread = true;
    mThread = std::thread(&WifiReader::workerThread, this);
}

void WifiReader::stop() {
    mRunThread = false;
    if (mThread.joinable()) mThread.join();
}

bool WifiReader::connectToServer(int& sockFd) {
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) return false;

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(mTargetPort);
    inet_pton(AF_INET, mTargetIp.c_str(), &serv_addr.sin_addr);

    // Timeout de 2s
    struct timeval tv; tv.tv_sec = 2; tv.tv_usec = 0;
    setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    ALOGI("Conectando a %s:%d...", mTargetIp.c_str(), mTargetPort);
    if (connect(sockFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ALOGE("Erro conexao: %s", strerror(errno));
        close(sockFd);
        return false;
    }
    ALOGI(">>> CONECTADO VIA WI-FI <<<");
    return true;
}

void WifiReader::workerThread() {
    int sockFd = -1;
    char rxBuffer[1024];
    std::string lineBuffer;

    while (mRunThread) {
        if (!mActive) {
            if (sockFd >= 0) { close(sockFd); sockFd = -1; }
            sleep(1); continue;
        }

        if (sockFd < 0) {
            if (!connectToServer(sockFd)) { sleep(2); continue; }
        }

        // Envia Polling
        const char* cmd = "GET DATA\n";
        if (send(sockFd, cmd, strlen(cmd), MSG_NOSIGNAL) < 0) {
            close(sockFd); sockFd = -1; continue;
        }

        // Lê Resposta
        memset(rxBuffer, 0, sizeof(rxBuffer));
        int n = recv(sockFd, rxBuffer, sizeof(rxBuffer)-1, 0);

        if (n > 0) {
            rxBuffer[n] = 0;
            lineBuffer += rxBuffer;
            size_t pos;
            while ((pos = lineBuffer.find('\n')) != std::string::npos) {
                std::string line = lineBuffer.substr(0, pos);
                lineBuffer.erase(0, pos + 1);
                if (!line.empty() && line.back() == '\r') line.pop_back();
                
                if (!line.empty()) {
                    AirData data = JsonParser::parse(line);
                    if (data.valid) {
                        std::lock_guard<std::mutex> lock(mListenerLock);
                        if (mListener) mListener->onDataReceived(data);
                    }
                }
            }
        } else if (n == 0) {
            close(sockFd); sockFd = -1;
        }
        sleep(1); // 1Hz Polling
    }
    if (sockFd >= 0) close(sockFd);
}