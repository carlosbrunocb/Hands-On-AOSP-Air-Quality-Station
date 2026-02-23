#pragma once

#include <stdint.h>
#include <string>

/**
 * Estrutura intermediária que representa uma leitura completa da estação.
 * Desacopla o formato JSON (ESP32) do formato Event (Android).
 */
struct AirData {
    // Timestamp do momento da recepção/parse (em nanosegundos, base de tempo do Android)
    int64_t timestamp;

    // Valores dos sensores (inicializados com -1.0 para indicar "sem leitura")
    float pm25;      // Partículas PM2.5 (ug/m3)
    float pm10;      // Partículas PM10 (ug/m3)
    float co_ppm;    // Monóxido de Carbono (ppm)
    float lpg_ppm;   // GLP / Gás Inflamável (ppm)
    float temp_c;    // Temperatura (Celsius)
    float humid_p;   // Umidade (%)

    // Metadados
    std::string source; // "serial" ou "wifi"
    bool valid;         // Flag para indicar se o parse foi bem sucedido

    // Construtor para inicialização limpa
    AirData() : 
        timestamp(0),
        pm25(-1.0f), 
        pm10(-1.0f),
        co_ppm(-1.0f), 
        lpg_ppm(-1.0f), 
        temp_c(-273.0f), // Zero absoluto como valor inválido para temp
        humid_p(-1.0f), 
        valid(false) {}
};

