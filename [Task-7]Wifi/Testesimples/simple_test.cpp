#include <android/sensor.h>
#include <android/looper.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// Identificador para o nosso evento no Looper
const int LOOPER_ID_SENSOR = 1;

int main() {
    printf("==========================================\n");
    printf("   TESTE DE SANIDADE - PROJETO AIRSTATION \n");
    printf("   (Versao NDK Moderno - Android 14)      \n");
    printf("==========================================\n");

    // 1. Obter o Manager do Android (Usando API moderna)
    ASensorManager* mgr = ASensorManager_getInstanceForPackage("com.airstation.cli.test");
    if (!mgr) {
        printf("ERRO CRITICO: Nao consegui acessar o SensorManager.\n");
        return 1;
    }

    // 2. Buscar o Sensor de PM2.5 na lista
    ASensorList list;
    int count = ASensorManager_getSensorList(mgr, &list);
    const ASensor* targetSensor = nullptr;

    printf("Varrendo %d sensores do sistema...\n", count);
    
    for (int i = 0; i < count; i++) {
        const char* name = ASensor_getName(list[i]);
        const char* vendor = ASensor_getVendor(list[i]);
        
        if (vendor && strstr(vendor, "AirStation") != nullptr) {
            if (strstr(name, "PM2.5") != nullptr) {
                targetSensor = list[i];
                printf("\n>>> SENSOR ENCONTRADO! <<<\n");
                printf("Nome:   %s\n", name);
                printf("Vendor: %s\n", vendor);
                printf("Handle: %d\n", ASensor_getHandle(list[i]));
                break;
            }
        }
    }

    if (!targetSensor) {
        printf("\nERRO: Sensor PM2.5 da AirStation nao encontrado na lista!\n");
        return 1;
    }

    // 3. Preparar a Fila de Eventos (Looper)
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    
    // Passamos LOOPER_ID_SENSOR (1) como identificador
    ASensorEventQueue* queue = ASensorManager_createEventQueue(mgr, looper, LOOPER_ID_SENSOR, nullptr, nullptr);
    if (!queue) {
        printf("ERRO: Falha ao criar fila de eventos.\n");
        return 1;
    }

    // 4. ATIVAR O SENSOR
    printf("\n[1/3] Solicitando ativacao do sensor...\n");
    if (ASensorEventQueue_enableSensor(queue, targetSensor) < 0) {
        printf("ERRO: Falha ao ativar (enableSensor returned error).\n");
        return 1;
    }
    
    // Taxa de 1 segundo (1.000.000 microssegundos)
    ASensorEventQueue_setEventRate(queue, targetSensor, 1000000); 

    printf("[2/3] Sensor ATIVO. Aguardando dados (20s)...\n");
    printf("      (A HAL agora vai buscar via Wi-Fi ou Cabo nativamente!)\n\n");

    // 5. Loop de Leitura
    int eventsReceived = 0;
    
    // Tenta por 20 iterações de espera
    for (int i = 0; i < 20; i++) {
        int ident;
        int events;
        struct android_poll_source* source;

        // Timeout de 2000ms
        ident = ALooper_pollOnce(2000, nullptr, &events, (void**)&source);

        // Se ident for igual ao nosso ID (1), temos dados!
        if (ident == LOOPER_ID_SENSOR) {
            ASensorEvent eventBuffer[1];
            // Tenta ler o evento da fila
            if (ASensorEventQueue_getEvents(queue, eventBuffer, 1) > 0) {
                float valor = eventBuffer[0].data[0];
                int64_t ts = eventBuffer[0].timestamp;
                
                printf("   -> [DADO RECEBIDO] PM2.5: %.2f (Timestamp: %lld)\n", valor, (long long)ts);
                eventsReceived++;
            }
        } else if (ident == ALOOPER_POLL_TIMEOUT) {
            printf("."); // Timeout, nada chegou ainda
            fflush(stdout);
        } else {
             printf("?"); // Outro evento ou erro
             fflush(stdout);
        }
    }

    // 6. Desligar e Limpar
    printf("\n\n[3/3] Desativando sensor e limpando recursos.\n");
    ASensorEventQueue_disableSensor(queue, targetSensor);
    ASensorManager_destroyEventQueue(mgr, queue);
    
    if (eventsReceived > 0) {
        printf("SUCESSO: Recebemos %d leituras do hardware!\n", eventsReceived);
    } else {
        printf("FALHA: O tempo acabou e nenhum dado chegou. Verifique o logcat.\n");
    }

    return 0;
}