#include <android/sensor.h>
#include <android/looper.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

/** * @brief Identificador de evento para o mecanismo de Looper do Android.
 * Utilizado para filtrar mensagens da fila de sensores no pollOnce.
 */
const int LOOPER_ID_SENSOR = 1;

/**
 * @file TestSanity.cpp
 * @brief Utilitário de teste de sanidade para validar a integração HAL <-> Framework.
 * * Este programa utiliza a API do NDK (Native Development Kit) para simular um app
 * consumindo os dados da Sub-HAL AirStation.
 */
int main() {
    printf("==========================================\n");
    printf("   TESTE DE SANIDADE - PROJETO AIRSTATION \n");
    printf("   (Versao NDK Moderno - Android 14)      \n");
    printf("==========================================\n");

    /** * @step 1: Acesso ao Sensor Manager.
     * @note Desde o Android 10+, é necessário fornecer um nome de pacote para rastreamento.
     */
    ASensorManager* mgr = ASensorManager_getInstanceForPackage("com.airstation.cli.test");
    if (!mgr) {
        printf("ERRO CRITICO: Nao consegui acessar o SensorManager.\n");
        return 1;
    }

    /** * @step 2: Descoberta de Hardware.
     * Varre todos os sensores registrados no sistema em busca do vendor "AirStation".
     */
    ASensorList list;
    int count = ASensorManager_getSensorList(mgr, &list);
    const ASensor* targetSensor = nullptr;

    printf("Varrendo %d sensores do sistema...\n", count);
    
    for (int i = 0; i < count; i++) {
        const char* name = ASensor_getName(list[i]);
        const char* vendor = ASensor_getVendor(list[i]);
        
        // Debug para ver o que está achando (opcional, pode comentar se poluir muito)
        // printf(" - %s (%s)\n", name, vendor);

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

    /** * @step 3: Preparação do Contexto de Execução (Looper).
     * O Android exige um Looper ativo para gerenciar a fila de eventos de forma não-bloqueante.
     */
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    
    // Passamos LOOPER_ID_SENSOR (1) como identificador
    ASensorEventQueue* queue = ASensorManager_createEventQueue(mgr, looper, LOOPER_ID_SENSOR, nullptr, nullptr);
    if (!queue) {
        printf("ERRO: Falha ao criar fila de eventos.\n");
        return 1;
    }

    /** * @step 4: Ativação e Configuração de Rate.
     * Solicita ao SensorService que inicie o polling na Sub-HAL.
     */
    printf("\n[1/3] Solicitando ativacao do sensor...\n");
    if (ASensorEventQueue_enableSensor(queue, targetSensor) < 0) {
        printf("ERRO: Falha ao ativar (enableSensor returned error).\n");
        return 1;
    }
    
    // Define a taxa de atualização para 1Hz (1.000.000 microssegundos)
    ASensorEventQueue_setEventRate(queue, targetSensor, 1000000); 

    printf("[2/3] Sensor ATIVO. Aguardando dados da Serial (20s)...\n");
    printf("      (Certifique-se que o ESP32 esta conectado e /dev/ttyUSB0 tem permissao 666)\n\n");

    /** * @step 5: Loop de Consumo de Eventos.
     * O pollOnce aguarda até 2000ms por um sinal do descriptor da fila de sensores.
     */
    int eventsReceived = 0;
    // Vamos tentar por 20 iterações de espera
    for (int i = 0; i < 20; i++) {
        int ident;
        int events;
        struct android_poll_source* source;

        // CORREÇÃO: Usando ALooper_pollOnce em vez de pollAll
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

    /** * @step 6: Cleanup.
     * Desativa o hardware e destrói a fila para evitar vazamento de memória (memory leak).
     */
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