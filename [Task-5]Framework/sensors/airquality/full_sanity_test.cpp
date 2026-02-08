#define LOG_TAG "AirStationTest"

#include <android/sensor.h>
#include <android/looper.h>
#include <log/log.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

// IDs dos Sensores (Mesmos do AirQualitySensor.cpp)
const int TYPE_PM25   = 65537;
const int TYPE_PM10   = 65538;
const int TYPE_CO     = 65539;
const int TYPE_LPG    = 65540;
const int TYPE_SOURCE = 65541; // ID corrigido conforme dumpsys (verifique se é 65541 ou 0x10005)

const int LOOPER_ID_SENSOR = 1;

// Helper para nomear os sensores no print
const char* getSensorName(int type) {
    switch (type) {
        case TYPE_PM25: return "PM 2.5";
        case TYPE_PM10: return "PM 10 ";
        case TYPE_CO:   return "CO    ";
        case TYPE_LPG:  return "LPG   ";
        case TYPE_SOURCE: return "FONTE ";
        case ASENSOR_TYPE_AMBIENT_TEMPERATURE: return "TEMP  ";
        case ASENSOR_TYPE_RELATIVE_HUMIDITY:   return "UMID  ";
        default: return "UNK   ";
    }
}

// Helper para unidade
const char* getUnit(int type) {
    switch (type) {
        case TYPE_PM25: 
        case TYPE_PM10: return "ug/m3";
        case TYPE_CO:   
        case TYPE_LPG:  return "ppm";
        case ASENSOR_TYPE_AMBIENT_TEMPERATURE: return "C";
        case ASENSOR_TYPE_RELATIVE_HUMIDITY:   return "%";
        default: return "";
    }
}

int main() {
    printf("\033[1;32m=== AirStation: Teste Completo de Sensores (Nativo) ===\033[0m\n");

    // 1. Obter Gerenciador
    ASensorManager* mgr = ASensorManager_getInstanceForPackage("com.airstation.test");
    if (!mgr) {
        printf("ERRO: Falha ao obter ASensorManager.\n");
        return -1;
    }

    // 2. Listar Sensores
    ASensorList list;
    int count = ASensorManager_getSensorList(mgr, &list);
    std::vector<const ASensor*> mySensors;

    printf("Varrendo %d sensores do sistema...\n", count);
    for (int i = 0; i < count; i++) {
        const ASensor* s = list[i];
        const char* vendor = ASensor_getVendor(s);
        
        // Filtra pelo Vendor definido na HAL
        if (vendor && strstr(vendor, "AirStation") != NULL) {
            printf(" -> Encontrado: %s (Type: %d)\n", ASensor_getName(s), ASensor_getType(s));
            mySensors.push_back(s);
        }
    }

    if (mySensors.empty()) {
        printf("\033[1;31mERRO: Nenhum sensor AirStation encontrado! Verifique a HAL.\033[0m\n");
        return -1;
    }

    // 3. Criar Fila e Ativar Todos
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ASensorEventQueue* queue = ASensorManager_createEventQueue(mgr, looper, LOOPER_ID_SENSOR, NULL, NULL);

    printf("\nAtivando %zu sensores...\n", mySensors.size());
    for (const ASensor* s : mySensors) {
        // Ativa com taxa de 1Hz (1.000.000 microsegundos)
        ASensorEventQueue_enableSensor(queue, s);
        ASensorEventQueue_setEventRate(queue, s, 1000000);
    }

    printf("\n\033[1;33mAguardando dados... (Pressione Ctrl+C para sair)\033[0m\n");
    printf("--------------------------------------------------\n");

    // 4. Loop de Leitura
    int eventsReceived = 0;
    while (1) {
        int ident;
        int events;
        void* data;

        // Poll com timeout de 1s
        ident = ALooper_pollOnce(1000, &ident, &events, &data);

        if (ident == LOOPER_ID_SENSOR) {
            ASensorEvent eventBuffer[8]; // Buffer para múltiplos eventos
            ssize_t num = ASensorEventQueue_getEvents(queue, eventBuffer, 8);
            
            for (int i = 0; i < num; i++) {
                int type = eventBuffer[i].type;
                float val = eventBuffer[i].data[0];

                // Tratamento especial para o sensor de FONTE
                if (type == TYPE_SOURCE) {
                    printf("[%s] Fonte de Dados: %s\n", 
                        getSensorName(type), 
                        (val == 1.0f) ? "Wi-Fi" : "USB (Serial)");
                } else {
                    printf("[%s] Valor: %6.2f %s\n", 
                        getSensorName(type), val, getUnit(type));
                }
            }
            if (num > 0) fflush(stdout);
        }
    }

    return 0;
}