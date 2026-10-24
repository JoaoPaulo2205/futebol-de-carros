// Futebol de Robos - firmware do robo (Competicao Interna, categoria prototipo)
// Placa: Arduino Mega (migravel para ESP32 - ver config.h)
// Biblioteca necessaria: "RF24" (TMRh20), instalar pelo Gerenciador de Bibliotecas.
//
// Fluxo: nRF24 recebe {robot_id, v, w} -> mistura diferencial limitada pelo edital
//        -> PWM limitado -> motores. Sem pacote por > WATCHDOG_MS -> stopMotors().

#include <SPI.h>
#include <RF24.h>
#include "config.h"
#include "motors.h"

static_assert(ROBOT_ID >= 0 && ROBOT_ID <= 2, "ROBOT_ID deve ser 0, 1 ou 2");

// Pacote enviado pela base. Mantenha identico no transmissor.
struct __attribute__((packed)) PacoteControle {
  uint8_t robot_id;  // 0..2, descarta se nao for o deste robo
  int8_t  v;         // -100..100 % da velocidade linear maxima (+ = frente)
  int8_t  w;         // -100..100 % da velocidade angular maxima (+ = anti-horario)
  uint8_t seq;       // contador, util para medir perdas
};

RF24 radio(PIN_RF_CE, PIN_RF_CSN);
const PerfilRF& perfil = TABELA_RF[TIME_DO_ROBO * 3 + ROBOT_ID];

unsigned long ultimoPacoteMs = 0;
bool recebeuAlgum = false;

#ifdef MODO_TESTE_MOTORES
// Sequencia do codigo inicial, mas com PWM limitado a PWM_MAX.
void testeMotores() {
  setMotors(PWM_MAX, PWM_MAX);
  delay(5000);
  setMotors(-PWM_MAX, -PWM_MAX);
  delay(5000);
}
#endif

// Converte (v%, w%) em PWM esquerdo/direito respeitando V_MAX e W_MAX do edital.
void misturar(int8_t vPct, int8_t wPct, int &pwmEsq, int &pwmDir) {
  float v = constrain(vPct, -100, 100) / 100.0f * V_MAX_MS;      // m/s
  float w = constrain(wPct, -100, 100) / 100.0f * W_MAX_RAD;     // rad/s

  float vEsq = v - w * ENTRE_RODAS_M / 2.0f;
  float vDir = v + w * ENTRE_RODAS_M / 2.0f;

  // Nenhuma roda pode passar de V_MAX: reduz as duas juntas, mantendo a curva.
  float maior = max(fabsf(vEsq), fabsf(vDir));
  if (maior > V_MAX_MS) {
    vEsq *= V_MAX_MS / maior;
    vDir *= V_MAX_MS / maior;
  }

  pwmEsq = (int)(vEsq / V_MAX_MS * PWM_MAX);
  pwmDir = (int)(vDir / V_MAX_MS * PWM_MAX);
}

void setup() {
#if DEBUG_SERIAL
  Serial.begin(115200);
#endif
  motorsBegin();   // motores parados desde o primeiro instante

#ifndef MODO_TESTE_MOTORES
#if !defined(ESP32)
  pinMode(53, OUTPUT);  // SS do Mega precisa ser saida para o SPI mestre funcionar
#endif
  if (!radio.begin()) {
#if DEBUG_SERIAL
    Serial.println(F("nRF24 nao encontrado - verifique fiacao"));
#endif
    while (true) { stopMotors(); delay(100); }   // falha segura
  }
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_2MBPS);           // edital: 2 Mbps
  radio.setChannel(perfil.canal);          // edital: canal exclusivo
  radio.setPayloadSize(sizeof(PacoteControle));
  radio.openReadingPipe(1, (const uint8_t*)perfil.endereco);
  radio.startListening();

#if DEBUG_SERIAL
  Serial.print(F("Robo "));
  Serial.print(TIME_DO_ROBO == TIME_AZUL ? F("Azul ") : F("Amarelo "));
  Serial.print(ROBOT_ID + 1);
  Serial.print(F(" | canal "));
  Serial.print(perfil.canal);
  Serial.print(F(" | end. "));
  Serial.println(perfil.endereco);
#endif
#endif
}

void loop() {
#ifdef MODO_TESTE_MOTORES
  testeMotores();
  return;
#else
  while (radio.available()) {          // drena a fila, usa o pacote mais recente
    PacoteControle p;
    radio.read(&p, sizeof(p));
    if (p.robot_id != ROBOT_ID) continue;

    int pwmEsq, pwmDir;
    misturar(p.v, p.w, pwmEsq, pwmDir);
    setMotors(pwmEsq, pwmDir);

    ultimoPacoteMs = millis();
    recebeuAlgum = true;
  }

  // Watchdog: nunca bloquear o loop (sem delay) para a checagem ser sempre rapida.
  if (!recebeuAlgum || millis() - ultimoPacoteMs > WATCHDOG_MS) {
    stopMotors();
  }
#endif
}
