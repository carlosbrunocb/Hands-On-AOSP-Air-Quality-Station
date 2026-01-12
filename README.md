# Hands-On-AOSP-Air-Quality-Station

## Introdução
Repositório do projeto de desenvolvimento para criar uma **estação de monitoramento da qualidade do ar integrado ao sistema Android (AOSP)**. 
O projeto abrange o desenvolvimento de um protótipo básico de hardware baseado em **ESP32 com sensores de partículas (SDS011) e gases (MQ2)** e a sua integração com o AOSP (Android Open Source Project). 
O objetivo é que os dados dos sensores sejam reconhecidos nativamente pelo sistema, utilizando a arquitetura Multi-HAL de sensores do Android.

### Funcionalidade Principais (Versão Básica ou inicial)
- **Coleta de Dados:** O protótipo com ESP32 coleta continuamente dados de material particulado (PM2.5 e PM10) do sensor SDS011 e detecta a presença de gases como GLP, i-butano, propano, metano, álcool, hidrogênio e fumaça com o sensor MQ2.
- **Conexão de Hardware:** A conexão física entre o protótipo (com saída USB padrão) e o smartphone (com entrada USB-C) deve ser feita utilizando um adaptador apropriado (ex: USB-A para USB-C OTG).
- **Integração Nativa:** O sistema Android reconhecerá a estação como um novo provedor de sensores de ambiente. Os dados serão disponibilizados para o sistema através de uma implementação customizada que se acopla ao Multi-HAL de sensores do Android.
- **Aplicação de Visualização:** Uma aplicação de sistema simples será desenvolvida para exibir os dados da qualidade do ar em tempo real, consultando o **`SensorManager`** para demonstrar a correta integração.

## Camadas do AOSP Abarcadas
- **Kernel:** Suporte para a comunicação serial entre o hardware externo (ESP32) e o dispositivo Android.
- **Native (HAL):** Desenvolvimento de uma Hardware Abstraction Layer (HAL) de sensores que irá se integrar à arquitetura Multi-HAL do AOSP. Esta camada será responsável por ler os dados da comunicação serial, interpretá-los e expô-los ao framework do Android.
- **Framework:** Utilização do **`SensorService`** e **`SensorManager`** para reconhecer e gerenciar os novos sensores de qualidade do ar (ex: `TYPE_PM25`, `TYPE_GAS_LEVEL`) disponibilizados pela camada HAL.

## Colaboradores
<img height="256" alt="image" src="https://github.com/user-attachments/assets/79be56ad-49e6-4207-930e-3c3fdd5704a1" />
<img height="256" alt="image" src="https://github.com/user-attachments/assets/efb3c50c-4d58-49cc-b6cd-6cd740bac383" />
<img height="256" alt="image" src="https://github.com/user-attachments/assets/348b7659-b93e-44d7-93b9-fd7bd62048d1" />
<img height="256" alt="image" src="https://github.com/user-attachments/assets/f79040b6-8458-4164-ad29-27f1e0e9abd6" />


## Rercusos
- Computador com os requisitos descritos na seção de [Configuração de Hardware](#configuracao-de-hardware).
- AOSP na versão 14.0.0_r27 ou superior.
- Placa ESP32-WROOM com protoboard
- Sensor de partículas (SDS011) e sensor de gases (MQ2)

## Requisitos
- Sistema operacional: Linux (preferencialmente Ubuntu)
- Repo
- Git
- Ambiente de desenvolvimento configurado para AOSP
- IDE Arduino
- Editor VSCode (ou outro)
- Smartphone Motorola G100 ou superior

## Configuração de Hardware
Para compilar e testar o AOSP, é recomendável ter:
- CPU: 8 núcleos (16 recomendados)
- RAM: 16 GB (32 GB recomendados)
- Espaço em disco: 350 GB (500 GB recomendados)
- Conexão de internet de alta velocidade

---

## Bônus (_Extra Features_)
Implementar na estação a leitura de **Temperatura** e **Humidade** do ar por meio do **sensor DHT11 ou DHT22**, e a presença de **altas concentrações de Monóxido de Carbono** adicionando o **sensor de MQ7** no sistema.

### Rercusos
- Sensor DHT11 ou DHT22 para Temperatura e Humidade.
- Sensor MQ7 para Monóxido de Carbono.


