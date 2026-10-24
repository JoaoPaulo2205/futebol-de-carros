#pragma once
// ===========================================================================
// config.h - pinos e ajustes do robo. O mesmo sketch serve para os 4 robos:
// so muda ROBOT_ID (e o nome Bluetooth do modulo, ver README).
// ===========================================================================

// ---------------------------------------------------------------------------
// 1. IDENTIDADE DO ROBO  <-- ALTERE AQUI POR ROBO
// ---------------------------------------------------------------------------
#define ROBOT_ID  1     // 1 a 4 (so aparece no log serial)

// ---------------------------------------------------------------------------
// 2. PINOS
// ---------------------------------------------------------------------------
#if defined(ESP32)
  // Evitar 6-11 (flash) e 34-39 (so entrada). Bluetooth: UART2 (RX=16, TX=17).
  const uint8_t PIN_EN_A = 25;   // PWM motor A (esquerdo)
  const uint8_t PIN_EN_B = 26;   // PWM motor B (direito)
  const uint8_t PIN_IN1  = 27;
  const uint8_t PIN_IN2  = 14;
  const uint8_t PIN_IN3  = 12;
  const uint8_t PIN_IN4  = 13;
  #define BT_SERIAL Serial2
#else
  // Arduino Mega 2560. Pinos de PWM: 2-13 e 44-46.
  const uint8_t PIN_EN_A = 5;    // PWM motor A (esquerdo)
  const uint8_t PIN_EN_B = 6;    // PWM motor B (direito)
  const uint8_t PIN_IN1  = 23;
  const uint8_t PIN_IN2  = 22;
  const uint8_t PIN_IN3  = 21;
  const uint8_t PIN_IN4  = 25;
  // Modulo Bluetooth HC-05/HC-06 na Serial1: TX do modulo -> pino 19 (RX1),
  // RX do modulo -> pino 18 (TX1) via divisor de tensao (5 V -> 3,3 V).
  #define BT_SERIAL Serial1
#endif

const unsigned long BT_BAUD = 9600;   // padrao de fabrica do HC-05/HC-06

// Inverta se o robo andar para tras ao mandar "frente" (ou girar ao contrario).
const bool INVERTE_MOTOR_A = false;
const bool INVERTE_MOTOR_B = false;

// ---------------------------------------------------------------------------
// 3. MOTORES
// ---------------------------------------------------------------------------
// PWM maximo (0-255), usado quando a velocidade do app esta em 100 %.
// Motor N20 6 V com bateria 2S (8,4 V): nao passar muito de ~180 para nao
// exceder a tensao nominal.
const uint8_t PWM_MAX = 180;

// Zona morta: abaixo deste PWM o N20 nao vence o atrito (ajustar na bancada).
const uint8_t PWM_MIN_MOVE = 40;

// ---------------------------------------------------------------------------
// 4. SEGURANCA
// ---------------------------------------------------------------------------
// Sem nenhum byte do celular por este tempo -> motores param (ex.: Bluetooth caiu
// ou o celular saiu de alcance). Apps de controle reenviam o comando enquanto o
// botao esta pressionado; se o seu app envia so uma vez, aumente este valor.
const unsigned long WATCHDOG_MS = 600;

// Velocidade inicial (0-100 %) ate o app mandar um nivel de velocidade.
const uint8_t VELOCIDADE_INICIAL = 60;

// ---------------------------------------------------------------------------
// 5. DEBUG
// ---------------------------------------------------------------------------
#define DEBUG_SERIAL 1          // 0 desliga logs (USB, 115200)
// #define MODO_TESTE_MOTORES   // descomente: 5 s frente, 5 s tras, sem Bluetooth
