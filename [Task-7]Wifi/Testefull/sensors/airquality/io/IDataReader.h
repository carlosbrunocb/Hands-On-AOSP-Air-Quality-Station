#pragma once
#include "../utils/AirData.h"

// Interface de Callback (Quem recebe os dados)
class IAirDataListener {
public:
    virtual ~IAirDataListener() = default;
    virtual void onDataReceived(const AirData& data) = 0;
};

// Interface Genérica de Leitura
class IDataReader {
public:
    virtual ~IDataReader() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setPollingActive(bool enabled) = 0; // Liga/Desliga envio de "GET DATA"
    virtual void setListener(IAirDataListener* listener) = 0;
};