#pragma once
#include <Arduino.h>
#include "config.h"

// Camada de motores (ponte H L298N / TB6612FNG, 2 canais).
// Unica parte que depende da plataforma: o PWM. O resto do firmware so chama
// motorsBegin(), setMotors() e stopMotors().

#if defined(ESP32)
  const uint32_t PWM_FREQ_HZ = 20000;
  const uint8_t  PWM_BITS    = 8;
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    #define PWM_ATTACH(pin) ledcAttach((pin), PWM_FREQ_HZ, PWM_BITS)
    #define PWM_WRITE(pin, duty) ledcWrite((pin), (duty))
  #else
    // core 2.x: um canal LEDC por pino
    #define PWM_ATTACH(pin) do { \
        ledcSetup(((pin) == PIN_EN_A) ? 0 : 1, PWM_FREQ_HZ, PWM_BITS); \
        ledcAttachPin((pin), ((pin) == PIN_EN_A) ? 0 : 1); } while (0)
    #define PWM_WRITE(pin, duty) ledcWrite(((pin) == PIN_EN_A) ? 0 : 1, (duty))
  #endif
#else
  #define PWM_ATTACH(pin) pinMode((pin), OUTPUT)
  #define PWM_WRITE(pin, duty) analogWrite((pin), (duty))
#endif

// Ultima barreira: qualquer valor que chegue aqui e limitado a PWM_MAX.
static inline int clampPwm(int v) {
  return constrain(v, -(int)PWM_MAX, (int)PWM_MAX);
}

static void driveMotor(uint8_t en, uint8_t inA, uint8_t inB, int pwm, bool inverte) {
  pwm = clampPwm(pwm);
  if (inverte) pwm = -pwm;
  if (pwm > 0 && pwm < PWM_MIN_MOVE) pwm = PWM_MIN_MOVE;
  if (pwm < 0 && pwm > -(int)PWM_MIN_MOVE) pwm = -(int)PWM_MIN_MOVE;

  digitalWrite(inA, pwm > 0 ? HIGH : LOW);
  digitalWrite(inB, pwm < 0 ? HIGH : LOW);
  PWM_WRITE(en, abs(pwm));
}

void stopMotors() {
  PWM_WRITE(PIN_EN_A, 0);
  PWM_WRITE(PIN_EN_B, 0);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);
}

void motorsBegin() {
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  PWM_ATTACH(PIN_EN_A);
  PWM_ATTACH(PIN_EN_B);
  stopMotors();
}

// pwmA = esquerdo, pwmB = direito, -PWM_MAX..PWM_MAX
void setMotors(int pwmA, int pwmB) {
  driveMotor(PIN_EN_A, PIN_IN1, PIN_IN2, pwmA, INVERTE_MOTOR_A);
  driveMotor(PIN_EN_B, PIN_IN3, PIN_IN4, pwmB, INVERTE_MOTOR_B);
}
