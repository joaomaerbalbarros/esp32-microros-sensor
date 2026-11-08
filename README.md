# Light sensor with micro-ROS on ESP32

An ESP32 reads a light sensor (LDR module), turns on an LED when it is dark and publishes the sensor state to **ROS 2 Jazzy** topics through **micro-ROS**. Any ROS 2 node on the computer can subscribe to these topics as if the ESP32 were just another node on the network.

I built this project as the hands-on example for the sensor-integration class of the GAIA Robotics (UFTM) ROS 2 course.

## How it works

```text
LDR module ──► ESP32 (node "sensor_esp32") ──serial──► micro-ROS Agent ──► ROS 2
                  │                                                      ├─ /sensor/valor   (std_msgs/Int32)
                  └─► LED                                                └─ /sensor/alerta  (std_msgs/Bool)
```

Every **100 ms** the firmware:

1. reads the digital output of the LDR module (`1` = dark, `0` = light; the threshold is set with the module's trimpot);
2. sets the alert flag (`dark = alert`);
3. turns the LED on or off locally;
4. publishes the reading on `/sensor/valor` and the alert on `/sensor/alerta`.

The ESP32 does not speak ROS 2 directly: it sends its messages over serial to the **micro-ROS Agent**, which runs on the computer and bridges them into the ROS 2 network.

If any step of the micro-ROS initialization fails (for example, the agent is not running), the board's blue LED blinks rapidly.

Topic names and code comments are in Portuguese (`valor` = value, `alerta` = alert).

## Wiring

| Component | ESP32 pin |
|---|---|
| LDR module digital output (DO) | GPIO 18 |
| Red LED + 220 Ω resistor | GPIO 19 |
| On-board blue LED (error signal) | GPIO 2 |

## Running it

Requirements: [PlatformIO](https://platformio.org/), ROS 2 Jazzy and Docker (for the agent).

**1. Flash the firmware to the ESP32**

```bash
pio run -t upload
```

On the first build PlatformIO downloads and compiles the [micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio) library, which takes a few minutes.

**2. Start the micro-ROS Agent** (replace `/dev/ttyUSB0` with your ESP32's port)

```bash
docker run -it --rm --net=host --device=/dev/ttyUSB0 microros/micro-ros-agent:jazzy serial --dev /dev/ttyUSB0 -b 115200 -v6
```

If the agent starts after the ESP32, press the board's EN button so it restarts and connects.

**3. Read the data in ROS 2**

```bash
ros2 topic list
ros2 topic echo /sensor/alerta
ros2 topic echo /sensor/valor
```

Cover the sensor with your hand: the LED turns on and `/sensor/alerta` switches to `true`.

## Project layout

| Path | Contents |
|---|---|
| `src/main.cpp` | firmware: sensor reading, LED and micro-ROS publishing |
| `platformio.ini` | `esp32dev` board, Arduino framework, micro-ROS Jazzy over serial |

## Stack

C++ · ESP32 · Arduino framework · PlatformIO · micro-ROS (rclc) · ROS 2 Jazzy · Docker
