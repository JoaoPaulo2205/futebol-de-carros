# Futebol de Carros – Firmware do Robô

Firmware (Arduino C++) para os robôs da **Competição Interna de Futebol de Robôs** (categoria protótipo, inspirada no IEEE VSS). O robô recebe comandos de velocidade por rádio nRF24L01+ e aciona dois motores N20 por uma ponte H, respeitando os limites do edital (velocidade, PWM e watchdog).

As regras do edital que afetam o código estão resumidas em [`REGRAS_FIRMWARE.md`](REGRAS_FIRMWARE.md).

## Como funciona

```
Base (PC/transmissor) --nRF24 2 Mbps--> [Robô] receber pacote -> misturar(v, w) -> PWM limitado -> ponte H -> motores
                                                      ^
                                     watchdog: sem pacote > 250 ms => stopMotors()
```

1. **Rádio:** cada robô escuta num canal e endereço exclusivos (Tabela 3 do edital), a 2 Mbps.
2. **Pacote** (4 bytes, `PacoteControle`):

   | Campo | Tipo | Significado |
   |---|---|---|
   | `robot_id` | `uint8_t` | 0–2; o robô descarta pacotes de outro id |
   | `v` | `int8_t` | −100..100 % da velocidade linear máxima (+ = frente) |
   | `w` | `int8_t` | −100..100 % da velocidade angular máxima (+ = anti-horário) |
   | `seq` | `uint8_t` | contador, para medir perdas |

3. **Mistura diferencial:** `v` e `w` viram velocidade das rodas esquerda/direita em m/s. Se alguma roda passar de 0,55 m/s, as duas são reduzidas na mesma proporção (mantém a curva). Depois converte para PWM (0..`PWM_MAX`).
4. **Segurança:** o PWM é limitado de novo na camada dos motores; sem pacote por mais de 250 ms (edital: 300 ms) os motores são zerados; se o nRF24 não for detectado no boot, os motores ficam parados.

## Estrutura

```
sketch_oct24a/
  sketch_oct24a.ino   # setup/loop: rádio, mistura v/w, watchdog
  config.h            # pinos, limites do edital, tabela RF, identidade do robô
  motors.h            # camada da ponte H (única parte dependente de plataforma: PWM)
REGRAS_FIRMWARE.md    # regras do edital relevantes ao código
```

## Dependências

- **Hardware:** Arduino Mega 2560 (futuro: ESP32), módulo nRF24L01+, ponte H L298N ou TB6612FNG, 2 motores N20 6 V, bateria LiPo 2S (350–650 mAh).
- **Software:** [Arduino IDE](https://www.arduino.cc/en/software) ou `arduino-cli`.
- **Biblioteca:** [`RF24`](https://github.com/nRF24/RF24) (TMRh20), testada com a 1.6.2.
- **Core:** `arduino:avr` (Mega). Para ESP32, o core `esp32` (Espressif), 2.x ou 3.x.

## Ligações (Arduino Mega)

| Sinal | Pino |
|---|---|
| Enable motor A (esquerdo, PWM) | 5 |
| Enable motor B (direito, PWM) | 6 |
| IN1 / IN2 (motor A) | 23 / 22 |
| IN3 / IN4 (motor B) | 21 / 25 |
| nRF24 CE / CSN | 9 / 10 |
| nRF24 SCK / MOSI / MISO | 52 / 51 / 50 (SPI fixo do Mega) |

> O código inicial usava 18/19 nos enables, mas no Mega esses pinos **não têm PWM**, e o edital exige limitar o PWM por software. Por isso foram movidos para 5 e 6.
> O nRF24L01+ trabalha a 3,3 V; muitos módulos sofrem com ruído de alimentação, então um capacitor de 10 µF entre VCC e GND ajuda.

## Como rodar

### 1. Configurar o robô (`sketch_oct24a/config.h`)
- `TIME_DO_ROBO` (`TIME_AZUL` ou `TIME_AMARELO`) e `ROBOT_ID` (0, 1 ou 2) escolhem canal e endereço automaticamente.
- `INVERTE_MOTOR_A/B`: inverta se o robô andar para trás ou girar ao contrário.

### 2. Calibrar (obrigatório antes de jogar)
- `ENTRE_RODAS_M`: meça a distância entre as rodas no robô montado.
- `PWM_MAX`: com bateria cheia, ajuste até o robô andar **≤ 55 cm/s** em linha reta (o edital nunca permite > 60 cm/s). O valor inicial (140) é só um chute conservador.
- `PWM_MIN_MOVE`: PWM mínimo para o N20 vencer o atrito.

### 3. Compilar e gravar

Arduino IDE: abra `sketch_oct24a/sketch_oct24a.ino`, instale a biblioteca RF24 (Gerenciador de Bibliotecas), selecione *Arduino Mega or Mega 2560* e envie.

Pela linha de comando:
```bash
arduino-cli core install arduino:avr
arduino-cli lib install RF24
arduino-cli compile --fqbn arduino:avr:mega sketch_oct24a
arduino-cli upload  --fqbn arduino:avr:mega -p /dev/ttyACM0 sketch_oct24a
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200   # logs de debug
```

### 4. Teste de bancada dos motores (sem rádio)
Descomente `#define MODO_TESTE_MOTORES` em `config.h`. O robô roda a sequência do código inicial (5 s para frente, 5 s para trás) com o PWM já limitado. Comente de novo para voltar ao modo normal.

### 5. Teste do watchdog (inspeção do edital)
Com o robô ligado e recebendo comandos, desligue o transmissor: as rodas devem parar em até 300 ms.

## Migração para ESP32

Todo o código dependente de placa está em `config.h` (pinos) e `motors.h` (PWM via LEDC). Para migrar:
1. Instale o core `esp32` e selecione a placa.
2. Revise o bloco `#if defined(ESP32)` de `config.h`. Os pinos padrão do SPI do ESP32 (SCK 18, MISO 19, MOSI 23) conflitam com os pinos de motor do código inicial, então o bloco do ESP32 já usa outros pinos para os motores.
3. A lógica de rádio, mistura e watchdog não muda. **O bloco ESP32 ainda não foi compilado nem testado em hardware.**

## Limitações conhecidas
- Não há ainda o firmware da **base transmissora**; ela deve enviar `PacoteControle` com o mesmo layout, no canal/endereço do robô, com intervalo bem menor que 250 ms (ex.: a cada 16 ms).
- Ainda sem teste em hardware real; `PWM_MAX` e `ENTRE_RODAS_M` são estimativas.
