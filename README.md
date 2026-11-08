# Sensor de luz com micro-ROS e ESP32

Um ESP32 lê um sensor de luz (módulo LDR), acende um LED quando está escuro e publica o estado do sensor em tópicos do **ROS 2 Jazzy** usando **micro-ROS**. Qualquer nó ROS 2 no computador pode assinar esses tópicos como se o ESP32 fosse mais um nó da rede.

Fiz este projeto como exemplo prático da aula de integração de sensores do minicurso de ROS 2 do GAIA Robótica (UFTM).

## Como funciona

```text
Módulo LDR ──► ESP32 (nó "sensor_esp32") ──serial──► micro-ROS Agent ──► ROS 2
                  │                                                     ├─ /sensor/valor   (std_msgs/Int32)
                  └─► LED                                               └─ /sensor/alerta  (std_msgs/Bool)
```

A cada **100 ms** o firmware:

1. lê a saída digital do módulo LDR (`1` = escuro, `0` = claro; o limite é ajustado no trimpot do módulo);
2. decide se há alerta (`escuro = alerta`);
3. acende ou apaga o LED localmente;
4. publica o valor lido em `/sensor/valor` e o alerta em `/sensor/alerta`.

O ESP32 não fala ROS 2 diretamente: ele envia as mensagens pela serial para o **micro-ROS Agent**, que roda no computador e repassa tudo para a rede ROS 2.

Se alguma etapa da inicialização do micro-ROS falhar (por exemplo, o agente não estar rodando), o LED azul da placa fica piscando rápido.

## Montagem

| Componente | Pino do ESP32 |
|---|---|
| Saída digital (DO) do módulo LDR | GPIO 18 |
| LED vermelho + resistor de 220 Ω | GPIO 19 |
| LED azul da placa (sinal de erro) | GPIO 2 |

## Como rodar

Requisitos: [PlatformIO](https://platformio.org/), ROS 2 Jazzy e Docker (para o agente).

**1. Gravar o firmware no ESP32**

```bash
pio run -t upload
```

Na primeira compilação o PlatformIO baixa e compila a biblioteca [micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio), o que demora alguns minutos.

**2. Subir o micro-ROS Agent** (troque `/dev/ttyUSB0` pela porta do seu ESP32)

```bash
docker run -it --rm --net=host --device=/dev/ttyUSB0 microros/micro-ros-agent:jazzy serial --dev /dev/ttyUSB0 -b 115200 -v6
```

Se o agente subir depois do ESP32, aperte o botão EN da placa para ela reiniciar e se conectar.

**3. Ver os dados no ROS 2**

```bash
ros2 topic list
ros2 topic echo /sensor/alerta
ros2 topic echo /sensor/valor
```

Cubra o sensor com a mão: o LED acende e `/sensor/alerta` passa para `true`.

## Estrutura

| Caminho | Conteúdo |
|---|---|
| `src/main.cpp` | firmware: leitura do sensor, LED e publicação micro-ROS |
| `platformio.ini` | placa `esp32dev`, framework Arduino, micro-ROS Jazzy via serial |

## Tecnologias

C++ · ESP32 · Arduino framework · PlatformIO · micro-ROS (rclc) · ROS 2 Jazzy · Docker
