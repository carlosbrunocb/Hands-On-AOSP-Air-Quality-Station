#pragma once

#include <string>
#include "AirData.h"

class JsonParser {
public:
    /**
     * Recebe uma linha de texto (JSON) e converte para AirData.
     * Retorna uma struct com .valid = false se o JSON for inválido.
     */
    static AirData parse(const std::string& jsonLine);
};

