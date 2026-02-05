#define LOG_TAG "AirQualityParser"

#include "JsonParser.h"
#include <json/json.h>          // libjsoncpp
#include <utils/SystemClock.h>  // Para android::elapsedRealtimeNano()
#include <log/log.h>            // Para ALOGE, ALOGD

AirData JsonParser::parse(const std::string& jsonLine) {
    AirData data;
    
    // Carimbar o tempo IMEDIATAMENTE (Requisito do Android SensorService)
    // Usa o relógio monotônico do kernel (boot time)
    data.timestamp = android::elapsedRealtimeNano();

    // Configuração do leitor JSON
    Json::CharReaderBuilder builder;
    Json::CharReader* reader = builder.newCharReader();
    Json::Value root;
    std::string errors;

    // Tentar fazer o parse da string
    bool parsingSuccessful = reader->parse(
        jsonLine.c_str(), 
        jsonLine.c_str() + jsonLine.size(), 
        &root, 
        &errors
    );
    delete reader;

    if (!parsingSuccessful) {
        ALOGE("Falha ao ler JSON: %s", errors.c_str());
        data.valid = false;
        return data;
    }

    // Validar o protocolo
    // Exemplo esperado: { "type": "data", "payload": { ... } }
    if (!root.isMember("type") || root["type"].asString() != "data") {
        // Se for um "ack" ou outro comando, ignoramos silenciosamente aqui
        data.valid = false;
        return data;
    }

    if (!root.isMember("payload")) {
        data.valid = false;
        return data;
    }

    Json::Value payload = root["payload"];

    // Extrair dados com segurança
    // O método .get("key", default) evita crash se a chave não existir
    
    // PM2.5 e PM10 (SDS011)
    if (payload.isMember("pm25")) data.pm25 = payload["pm25"].asFloat();
    if (payload.isMember("pm10")) data.pm10 = payload["pm10"].asFloat();

    // Gases (MQ2 / MQ7)
    if (payload.isMember("co_ppm"))  data.co_ppm  = payload["co_ppm"].asFloat();
    if (payload.isMember("lpg_ppm")) data.lpg_ppm = payload["lpg_ppm"].asFloat();

    // Clima (DHT)
    if (payload.isMember("temp_c"))  data.temp_c  = payload["temp_c"].asFloat();
    if (payload.isMember("humid_p")) data.humid_p = payload["humid_p"].asFloat();
    
    // Fonte (Opcional)
    if (root.isMember("src")) data.source = root["src"].asString();

    data.valid = true;
    return data;
}

