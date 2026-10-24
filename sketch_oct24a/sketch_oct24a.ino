// Futebol de Robos - firmware do robo controlado pelo celular (Bluetooth)
// Placa: Arduino Mega + modulo HC-05/HC-06 + ponte H + 2 motores N20.
// Sem bibliotecas externas. Gravar o MESMO sketch nos 4 robos, mudando ROBOT_ID.
//
// Protocolo (1 caractere por comando, compativel com apps tipo
// "Arduino Bluetooth Controller" / "Bluetooth RC Car", modo botoes):
//   F frente   B tras   L gira anti-horario   R gira horario
//   G frente+esquerda   I frente+direita   H tras+esquerda   J tras+direita
//   S parar
//   0..9 velocidade 0..90 %   q velocidade 100 %

#include "config.h"
#include "motors.h"

static_assert(ROBOT_ID >= 1 && ROBOT_ID <= 4, "ROBOT_ID deve ser de 1 a 4");

uint8_t velocidade = VELOCIDADE_INICIAL;  // 0..100 %
char comando = 'S';                       // ultimo comando de movimento
unsigned long ultimoByteMs = 0;
bool recebeuAlgum = false;

#ifdef MODO_TESTE_MOTORES
void testeMotores() {
  setMotors(PWM_MAX, PWM_MAX);
  delay(5000);
  setMotors(-PWM_MAX, -PWM_MAX);
  delay(5000);
}
#endif

// Converte (v%, w%) em PWM esquerdo/direito. w > 0 = anti-horario.
void misturar(int vPct, int wPct, int &pwmEsq, int &pwmDir) {
  int esq = vPct - wPct;
  int dir = vPct + wPct;

  // Nenhuma roda pode passar de 100 %: reduz as duas juntas, mantendo a curva.
  int maior = max(abs(esq), abs(dir));
  if (maior > 100) {
    esq = esq * 100 / maior;
    dir = dir * 100 / maior;
  }

  pwmEsq = esq * (int)PWM_MAX / 100;
  pwmDir = dir * (int)PWM_MAX / 100;
}

void aplicarComando() {
  int v = 0, w = 0;
  const int s = velocidade;
  switch (comando) {
    case 'F': v =  s;          break;
    case 'B': v = -s;          break;
    case 'L': w =  s;          break;
    case 'R': w = -s;          break;
    case 'G': v =  s; w =  s / 2; break;
    case 'I': v =  s; w = -s / 2; break;
    case 'H': v = -s; w = -s / 2; break;
    case 'J': v = -s; w =  s / 2; break;
    default:  break;   // 'S': parado
  }
  int pwmEsq, pwmDir;
  misturar(v, w, pwmEsq, pwmDir);
  if (pwmEsq == 0 && pwmDir == 0) stopMotors();
  else setMotors(pwmEsq, pwmDir);
}

void processarByte(char c) {
  if (c >= '0' && c <= '9') {
    velocidade = (c - '0') * 10;
  } else if (c == 'q') {
    velocidade = 100;
  } else if (strchr("FBLRGIHJS", c)) {
    comando = c;
  } else {
    return;   // \r, \n e lixo: ignora (nao conta para o watchdog)
  }
  ultimoByteMs = millis();
  recebeuAlgum = true;
  aplicarComando();
}

void setup() {
#if DEBUG_SERIAL
  Serial.begin(115200);
#endif
  motorsBegin();   // motores parados desde o primeiro instante

#ifndef MODO_TESTE_MOTORES
  BT_SERIAL.begin(BT_BAUD);
#if DEBUG_SERIAL
  Serial.print(F("Robo "));
  Serial.print(ROBOT_ID);
  Serial.println(F(" pronto - aguardando celular via Bluetooth"));
#endif
#endif
}

void loop() {
#ifdef MODO_TESTE_MOTORES
  testeMotores();
  return;
#else
  while (BT_SERIAL.available()) {
    processarByte((char)BT_SERIAL.read());
  }

  // Watchdog: nunca bloquear o loop (sem delay) para a checagem ser sempre rapida.
  if (!recebeuAlgum || millis() - ultimoByteMs > WATCHDOG_MS) {
    comando = 'S';
    stopMotors();
  }
#endif
}
