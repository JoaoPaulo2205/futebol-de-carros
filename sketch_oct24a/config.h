#pragma once
// ===========================================================================
// config.h - tudo que muda entre placas (Mega -> ESP32) e entre robos fica aqui.
// Regulamento: Edital Futebol de Robos (Competicao Interna).
// ===========================================================================

// ---------------------------------------------------------------------------
// 1. IDENTIDADE DO ROBO  (Tabela 3 do edital)  <-- ALTERE AQUI POR ROBO
// ---------------------------------------------------------------------------
#define TIME_AZUL     0
#define TIME_AMARELO  1

#define TIME_DO_ROBO  TIME_AZUL   // TIME_AZUL ou TIME_AMARELO
#define ROBOT_ID      0           // 0, 1 ou 2 (robo dentro do time)

// ---------------------------------------------------------------------------
// 2. PINOS  (unico bloco a revisar na migracao para ESP32)
// ---------------------------------------------------------------------------
#if defined(ESP32)
  // Qualquer GPIO de saida serve para PWM (LEDC). Evitar 6-11 (flash) e 34-39 (so entrada).
  // SPI do nRF24 no ESP32 (VSPI padrao): SCK=18 MISO=19 MOSI=23.
  // ATENCAO: o codigo inicial usava 18/19/23 como motor; no ESP32 eles conflitam com o SPI.
  const uint8_t PIN_EN_A = 25;   // PWM motor A (esquerdo)
  const uint8_t PIN_EN_B = 26;   // PWM motor B (direito)
  const uint8_t PIN_IN1  = 27;
  const uint8_t PIN_IN2  = 14;
  const uint8_t PIN_IN3  = 12;
  const uint8_t PIN_IN4  = 13;
  const uint8_t PIN_RF_CE  = 4;
  const uint8_t PIN_RF_CSN = 5;
#else
  // Arduino Mega 2560. Pinos de PWM: 2-13 e 44-46.
  // Os pinos 18 e 19 do codigo inicial NAO tem PWM no Mega, e o edital exige limitar
  // o PWM por software -> os enables foram movidos para 5 e 6. In1..In4 mantidos.
  // Se a fiacao ja esta em 18/19, mova os fios dos enables para 5 e 6.
  const uint8_t PIN_EN_A = 5;    // PWM motor A (esquerdo)
  const uint8_t PIN_EN_B = 6;    // PWM motor B (direito)
  const uint8_t PIN_IN1  = 23;
  const uint8_t PIN_IN2  = 22;
  const uint8_t PIN_IN3  = 21;
  const uint8_t PIN_IN4  = 25;
  // SPI do Mega e fixo: MISO=50 MOSI=51 SCK=52 (SS=53 deve ficar como OUTPUT).
  const uint8_t PIN_RF_CE  = 9;
  const uint8_t PIN_RF_CSN = 10;
#endif

// Inverta se o robo andar para tras ao receber v > 0 (ou gire ao contrario).
const bool INVERTE_MOTOR_A = false;
const bool INVERTE_MOTOR_B = false;

// ---------------------------------------------------------------------------
// 3. LIMITES DO EDITAL (secao 4) - nao aumentar sem mudar o regulamento
// ---------------------------------------------------------------------------
const float V_MAX_MS = 0.55f;    // velocidade linear maxima [m/s]
const float W_MAX_RAD = 13.75f;  // velocidade angular maxima [rad/s]

// Distancia entre as rodas [m]. MEDIR no robo montado (limite do chassi: 8 cm).
const float ENTRE_RODAS_M = 0.065f;

// PWM maximo (0-255) que corresponde a V_MAX_MS em linha reta.
// CALIBRAR: com bateria cheia, ajuste ate o robo fazer <= 55 cm/s (teste do edital: nunca > 60).
// Valor inicial conservador; N20 6V alimentado por 8,4V + queda da ponte H.
const uint8_t PWM_MAX = 140;

// Zona morta: abaixo deste PWM o N20 nao vence o atrito (ajustar na bancada).
const uint8_t PWM_MIN_MOVE = 40;

// ---------------------------------------------------------------------------
// 4. WATCHDOG (secao 4 e teste 3 da inspecao)
// ---------------------------------------------------------------------------
// Edital: > 300 ms sem pacote -> motores zerados. Margem para o loop e a rampa de frenagem.
const unsigned long WATCHDOG_MS = 250;

// ---------------------------------------------------------------------------
// 5. RADIO nRF24L01+ (secao 5): Frequencia = 2400 MHz + canal, 2 Mbps
// ---------------------------------------------------------------------------
struct PerfilRF {
  uint8_t canal;
  const char* endereco;   // 5 bytes + '\0'
};

// Indice = TIME_DO_ROBO * 3 + ROBOT_ID  (ordem da Tabela 3)
const PerfilRF TABELA_RF[6] = {
  {10, "VSS01"},  // Azul 1
  {20, "VSS02"},  // Azul 2
  {30, "VSS03"},  // Azul 3
  {40, "VSS04"},  // Amarelo 1
  {50, "VSS05"},  // Amarelo 2
  {60, "VSS06"},  // Amarelo 3
};

// ---------------------------------------------------------------------------
// 6. DEBUG
// ---------------------------------------------------------------------------
#define DEBUG_SERIAL 1          // 0 desliga logs
// #define MODO_TESTE_MOTORES   // descomente: roda a sequencia do codigo inicial (sem radio), com PWM limitado
