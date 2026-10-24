# Futebol de Carros – Firmware do Robô

Firmware (Arduino C++) para robôs de futebol controlados **pelo celular via Bluetooth**. São 4 robôs, cada um com seu próprio Arduino Mega + módulo HC-05/HC-06 + ponte H + 2 motores N20. O mesmo sketch roda nos 4; só muda o `ROBOT_ID`.

```
Celular --Bluetooth--> HC-05/06 --Serial1--> Arduino Mega --PWM--> ponte H --> motores
                                                  ^
                          watchdog: sem byte > 600 ms => stopMotors()
```

## Protocolo (1 caractere por comando)

Compatível com apps de carrinho RC no modo botões (ex.: *Arduino Bluetooth Controller*, *Bluetooth RC Car*). 9600 baud.

| Caractere | Ação |
|---|---|
| `F` / `B` | frente / trás |
| `L` / `R` | gira anti-horário / horário (no lugar) |
| `G` / `I` | frente + esquerda / frente + direita |
| `H` / `J` | trás + esquerda / trás + direita |
| `S` | parar |
| `0`–`9` | velocidade 0–90 % |
| `q` | velocidade 100 % |

`\r`, `\n` e outros caracteres são ignorados. A velocidade inicial é `VELOCIDADE_INICIAL` (60 %).

## Segurança
- PWM limitado a `PWM_MAX` na camada dos motores.
- Sem nenhum comando por mais de `WATCHDOG_MS` (600 ms), os motores param (Bluetooth caiu, celular longe). Se o seu app envia o comando só uma vez ao apertar o botão, aumente esse valor.
- Motores parados desde o boot até o primeiro comando.

## Estrutura

```
sketch_oct24a/
  sketch_oct24a.ino   # setup/loop: Bluetooth, comandos, mistura, watchdog
  config.h            # pinos, ROBOT_ID, PWM, watchdog
  motors.h            # camada da ponte H (única parte dependente de plataforma: PWM)
```

Sem bibliotecas externas.

## Ligações (Arduino Mega)

| Sinal | Pino |
|---|---|
| Enable motor A (esquerdo, PWM) | 5 |
| Enable motor B (direito, PWM) | 6 |
| IN1 / IN2 (motor A) | 23 / 22 |
| IN3 / IN4 (motor B) | 21 / 25 |
| HC-05/06 TX → Mega | 19 (RX1) |
| HC-05/06 RX ← Mega | 18 (TX1), **com divisor de tensão** |

> O RX do HC-05/06 é 3,3 V; o TX do Mega é 5 V. Use um divisor (ex.: 1 kΩ em série + 2 kΩ para GND) no fio 18 → RX do módulo.
> Os enables ficam em 5 e 6 porque o PWM não existe nos pinos 18/19 do Mega (que agora são a Serial1).

## Como usar

### 1. Configurar cada robô
- `config.h`: `ROBOT_ID` (1 a 4) e `INVERTE_MOTOR_A/B` (inverta se andar para trás ou girar ao contrário).
- Dê um **nome Bluetooth diferente** a cada módulo para distinguir no celular (ex.: `ROBO1` … `ROBO4`). Com o módulo desconectado do Mega's Serial1, mande por AT (HC-05: modo AT com botão pressionado, 38400 baud; HC-06: 9600 baud): `AT+NAME=ROBO1` (HC-05) ou `AT+NAMEROBO1` (HC-06, sem sinal de igual, sem fim de linha).
- Pareie o celular com cada módulo (PIN padrão `1234` ou `0000`). Para dirigir os 4 ao mesmo tempo é preciso um celular por robô.

### 2. Calibrar
- `PWM_MAX`: a velocidade máxima desejada em 100 %. Valor inicial (180) é conservador para N20 6 V com bateria 2S.
- `PWM_MIN_MOVE`: PWM mínimo para o N20 vencer o atrito.

### 3. Compilar e gravar
```bash
arduino-cli core install arduino:avr
arduino-cli compile --fqbn arduino:avr:mega sketch_oct24a
arduino-cli upload  --fqbn arduino:avr:mega -p /dev/ttyACM0 sketch_oct24a
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```
Desconecte o fio do HC-05/06 durante o upload se der erro (a Serial1 não interfere no upload pela USB, mas facilita o diagnóstico).

### 4. Teste de bancada dos motores (sem Bluetooth)
Descomente `#define MODO_TESTE_MOTORES` em `config.h`: 5 s para frente, 5 s para trás.

## Limitações
- Ainda sem teste em hardware real.
- Bluetooth clássico (HC-05/06) funciona em Android; iPhone não pareia com ele. Para iPhone, use um módulo BLE (HM-10) ou migre para ESP32.
- O bloco `ESP32` de `config.h`/`motors.h` não foi compilado e o `BT_SERIAL` dele (UART2) ainda precisaria de módulo externo ou `BluetoothSerial`.
