# Regras do edital relevantes para o firmware (Futebol de Robôs – Competição Interna)

Hardware: Arduino Mega (futuro: ESP32) + nRF24L01+ (2.4 GHz) + ponte H dupla (L298N ou TB6612FNG) + 2 motores N20 (6 V, ~400 RPM). Bateria LiPo 2S (7,4 V nominal, 8,4 V máx).

## Limites de velocidade (seção 4)
- Velocidade linear máxima: **0,55 m/s**
- Velocidade angular máxima: **13,75 rad/s**
- Trava de PWM: valor máximo limitado por software para que o robô **não passe de 60 cm/s** em teste retilíneo.

## Watchdog (obrigatório)
- Se o tempo desde a última mensagem de rádio for **> 300 ms**, chamar `stopMotors()` (zerar PWM).
- Teste de inspeção: desligam o emissor e o robô deve parar totalmente as rodas em até 300 ms.

## Rádio nRF24L01+ (seção 5)
- Frequência = 2400 MHz + canal; taxa **2 Mbps**.
- Cada robô: canal exclusivo e endereço de pipe fixo.

| Robô | robot_id | Canal | Endereço |
|---|---|---|---|
| Azul 1 | 0 | 10 | "VSS01" |
| Azul 2 | 1 | 20 | "VSS02" |
| Azul 3 | 2 | 30 | "VSS03" |
| Amarelo 1 | 0 | 40 | "VSS04" |
| Amarelo 2 | 1 | 50 | "VSS05" |
| Amarelo 3 | 2 | 60 | "VSS06" |

## Hardware que afeta o código
- Driver: L298N ou TB6612FNG; PWM deve ser limitado no firmware.
- Massa máx. 350 g (+10 g), dimensões 8 × 10 × 14 cm (sem impacto no código).
- Motores N20 6 V alimentados por até 8,4 V: limitar duty para não exceder a tensão nominal.
