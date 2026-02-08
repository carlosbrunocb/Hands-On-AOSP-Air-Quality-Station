#pragma once

#include <string>
#include "AirData.h"

/**
 * @class JsonParser
 * @brief Utilitário estático para desserialização de dados de telemetria.
 * * Responsável por converter strings formatadas em JSON, provenientes da 
 * estação de monitoramento, para a estrutura interna de dados AirData.
 */
class JsonParser {
public:
    /**
     * @brief Converte uma linha de texto JSON em um objeto AirData.
     * * O método espera um JSON formatado com as chaves correspondentes aos sensores.
     * Caso o JSON esteja malformado ou faltem campos essenciais, o objeto 
     * retornado terá o campo 'valid' definido como false.
     * * @param jsonLine String contendo a linha bruta recebida via hardware.
     * @return AirData Estrutura preenchida com os valores lidos ou marcada como inválida.
     */
    static AirData parse(const std::string& jsonLine);
};

