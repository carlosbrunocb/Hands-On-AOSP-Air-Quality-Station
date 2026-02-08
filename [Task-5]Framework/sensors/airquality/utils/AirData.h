#pragma once

#include <stdint.h>
#include <string>

/**
 * @struct AirData
 * @brief Container intermediário para telemetria de qualidade do ar.
 * * Esta estrutura atua como uma camada de desacoplamento (DTO) entre o hardware 
 * (ESP32/Serial) e o formato Event exigido pelo Android Sensors Framework.
 */
struct AirData {
    /// @brief Timestamp sincronizado com a base de tempo do Android (nanossegundos).
    int64_t timestamp;

    // Valores dos sensores (inicializados com -1.0 para indicar "sem leitura")
    float pm25;      ///< Partículas finas PM2.5 [µg/m³].
    float pm10;      ///< Partículas inaláveis PM10 [µg/m³].
    float co_ppm;    ///< Concentração de Monóxido de Carbono [ppm].
    float lpg_ppm;   ///< Concentração de GLP / Gases Inflamáveis [ppm].
    float temp_c;    ///< Temperatura Ambiente [°C]. (Default: -273.0f para inválido).
    float humid_p;   ///< Umidade Relativa do Ar [%].

    // Metadados
    std::string source; ///< Origem da captura (ex: "serial", "wifi").
    bool valid;         ///< Flag de integridade (true se o parse foi bem-sucedido).

    /**
     * @brief Inicializa a estrutura com valores padrão de segurança.
     * Valores numéricos iniciam em -1.0f e temperatura em Zero Absoluto.
     */
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

