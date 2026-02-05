#define LOG_TAG "AirQualitySerial"

#include "SerialReader.h"
#include "../utils/JsonParser.h" 

#include <log/log.h>
#include <fcntl.h>      
#include <errno.h>      
#include <termios.h>    
#include <unistd.h>     
#include <string.h>
#include <sys/stat.h>
#include <vector>

SerialReader::SerialReader(const std::string& /*devicePathIgnored*/)
    : mDevicePath(""), mRunThread(false), mPollingActive(false), mListener(nullptr) {
}

SerialReader::~SerialReader() {
    stop();
}

void SerialReader::setListener(IAirDataListener* listener) {
    std::lock_guard<std::mutex> lock(mListenerLock);
    mListener = listener;
}

// Controla se o loop envia "GET DATA" ou apenas dorme
void SerialReader::setPollingActive(bool enabled) {
    bool wasEnabled = mPollingActive.exchange(enabled);
    if (wasEnabled != enabled) {
        ALOGI("Status do Polling alterado: %s", enabled ? "ATIVO (Enviando GET DATA)" : "STANDBY (Silencioso)");
    }
}

void SerialReader::start() {
    if (mRunThread) return;
    mRunThread = true;
    mThread = std::thread(&SerialReader::workerThread, this);
}

void SerialReader::stop() {
    mRunThread = false;
    if (mThread.joinable()) {
        mThread.join();
    }
}

// Procura a porta USB automaticamente
std::string SerialReader::findSerialDevice() {
    // Tenta ttyUSB0 a ttyUSB9
    for (int i = 0; i < 10; i++) {
        std::string path = "/dev/ttyUSB" + std::to_string(i);
        if (access(path.c_str(), R_OK | W_OK) == 0) return path;
    }
    // Tenta ttyACM0 a ttyACM9 (Melhoria)
    for (int i = 0; i < 10; i++) {
        std::string path = "/dev/ttyACM" + std::to_string(i);
        if (access(path.c_str(), R_OK | W_OK) == 0) return path;
    }
    return "";
}

bool SerialReader::configureSerial(int fd) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        ALOGE("Erro tcgetattr: %s", strerror(errno));
        return false;
    }

    // --- 1. VELOCIDADE CORRETA (Igual ao seu Sketch Arduino) ---
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // --- 2. CONFIGURAÇÃO 8N1 ---
    tty.c_cflag &= ~PARENB; // Sem paridade
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8 bits
    tty.c_cflag &= ~CRTSCTS;// Sem controle de fluxo
    tty.c_cflag |= CREAD | CLOCAL; // Habilita leitura

    // --- 3. MODO RAW (Cru) ---
    // Importante: Desativamos ICANON para ler os bytes assim que chegarem
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; 
    tty.c_lflag &= ~ECHOE; 
    tty.c_lflag &= ~ISIG; 

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); 
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    tty.c_oflag &= ~OPOST;

    // --- 4. TIMEOUT DE LEITURA ---
    // Espera até 0.5 segundos (5 * 0.1s) por dados após enviar o comando
    tty.c_cc[VTIME] = 5; 
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        ALOGE("Erro tcsetattr: %s", strerror(errno));
        return false;
    }
    return true;
}

void SerialReader::workerThread() {
    int fd = -1;
    std::string lineBuffer;
    char rxBuffer[512];

    ALOGI("Thread Serial Iniciada. Aguardando ativação de sensores...");

    while (mRunThread) {
        
        // --- ESTADO 1: STANDBY ---
        // Se nenhum app pediu dados, não gastamos CPU nem USB.
        if (!mPollingActive) {
            // Se estiver conectado, mantemos aberto para resposta rápida
            sleep(1); 
            continue; 
        }

        // --- ESTADO 2: CONEXÃO ---
        if (fd < 0) {
            std::string path = findSerialDevice();
            
            if (path.empty()) {
                ALOGV("Nenhum dispositivo serial encontrado.");
                sleep(2);
                continue;
            }

            ALOGI("Dispositivo encontrado: %s. Tentando abrir...", path.c_str());
            fd = open(path.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
            
            if (fd < 0) {
                ALOGE("Falha ao abrir %s: %s", path.c_str(), strerror(errno));
                sleep(2);
                continue;
            }

            if (!configureSerial(fd)) {
                close(fd);
                fd = -1;
                sleep(1);
                continue;
            }

            mDevicePath = path;
            ALOGI(">>> CONECTADO A %s (115200 baud) <<<", mDevicePath.c_str());
            
            tcflush(fd, TCIOFLUSH);
            lineBuffer.clear();
        }

        // --- ESTADO 3: COMUNICAÇÃO (POLLING) ---

        // Verificar se devemos pedir dados
        if (mPollingActive) {
            // A. ESCREVER O COMANDO (Enviar o Pedido "GET DATA")
            // O ESP32 espera '\n' para processar (inputBuffer.trim no Arduino)
            // Se mandar sem \n, o ESP32 vai ficar esperando para sempre.
            const char* cmd = "GET DATA\n"; 
            ssize_t written = write(fd, cmd, strlen(cmd));
            
            if (written < 0) {
                // Erro: Cabo desconectado durante a escrita
                ALOGE("Erro de escrita (Cabo desconectado?): %s", strerror(errno));
                close(fd); 
                fd = -1;
                continue; // Volta para o loop de busca
            }
        }

        // B. LER A RESPOSTA (Loop de leitura com timeout)
        // O read() vai esperar até 0.5s (VTIME) pelos dados
        // Mesmo que mPollingActive seja false, podemos ler para limpar o buffer
        memset(rxBuffer, 0, sizeof(rxBuffer));
        int n = read(fd, rxBuffer, sizeof(rxBuffer) - 1);

        if (n > 0) {
            rxBuffer[n] = 0;
            lineBuffer += rxBuffer;

            // Debug Opcional: ver o que chegou cru
            ALOGD("[RAW] %s", rxBuffer);

            // Processar linhas completas
            size_t pos;
            while ((pos = lineBuffer.find('\n')) != std::string::npos) {
                std::string line = lineBuffer.substr(0, pos);
                lineBuffer.erase(0, pos + 1);

                // Limpeza de caracteres
                if (!line.empty() && line.back() == '\r') line.pop_back();

                if (!line.empty()) {
                    ALOGD("[JSON] %s", line.c_str());
                    AirData data = JsonParser::parse(line);
                    
                    if (data.valid) {
                        std::lock_guard<std::mutex> lock(mListenerLock);
                        if (mListener) mListener->onDataReceived(data);
                    }
                }
            }
        } 
        else if (n < 0) {
             ALOGE("Erro fatal de leitura. Reiniciando conexão...");
             close(fd);
             fd = -1;
        }

        // --- ESTADO 4: RITMO ---
        // Espera 1 segundo antes de pedir dados de novo (1Hz)
        sleep(1);
    }

    if (fd >= 0) close(fd);
    ALOGI("Thread Serial Finalizada.");
}