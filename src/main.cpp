#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/bool.h>

// ---------- Pinos ----------
const int PINO_SENSOR = 18;   // DO do módulo LDR
const int PINO_LED    = 19;   // LED vermelho com resistor de 220 ohms
const int PINO_ERRO   = 2;    // LED azul da placa, pisca se der erro

// ---------- Objetos do micro-ROS ----------
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rcl_publisher_t pub_valor;
rcl_publisher_t pub_alerta;
rcl_timer_t timer;
rclc_executor_t executor;

std_msgs__msg__Int32 msg_valor;
std_msgs__msg__Bool  msg_alerta;

bool alerta = false;

// ---------- Tratamento de erro ----------
void loop_de_erro() {
  pinMode(PINO_ERRO, OUTPUT);
  bool estado = false;
  while (true) {
    estado = !estado;
    digitalWrite(PINO_ERRO, estado);
    delay(100);
  }
}

#define RCCHECK(fn)     { rcl_ret_t rc = fn; if (rc != RCL_RET_OK) { loop_de_erro(); } }
#define RCSOFTCHECK(fn) { rcl_ret_t rc = fn; (void)rc; }

// ---------- O que acontece a cada 100 ms ----------
void ao_tick(rcl_timer_t * t, int64_t ultima_chamada) {
  (void) ultima_chamada;
  if (t == NULL) return;

  // 1. Ler o sensor: 1 = escuro, 0 = claro
  int valor = digitalRead(PINO_SENSOR);

  // 2. Decidir: o limite já foi feito pelo trimpot do módulo
  alerta = (valor == HIGH);

  // 3. Agir localmente
  digitalWrite(PINO_LED, alerta ? HIGH : LOW);

  // 4. Avisar o resto do sistema
  msg_valor.data  = valor;
  msg_alerta.data = alerta;
  RCSOFTCHECK(rcl_publish(&pub_valor,  &msg_valor,  NULL));
  RCSOFTCHECK(rcl_publish(&pub_alerta, &msg_alerta, NULL));
}

void setup() {
  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_SENSOR, INPUT);
  digitalWrite(PINO_LED, LOW);

  // A Serial passa a ser do micro-ROS
  Serial.begin(115200);
  set_microros_serial_transports(Serial);
  delay(2000);

  allocator = rcl_get_default_allocator();
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  RCCHECK(rclc_node_init_default(&node, "sensor_esp32", "", &support));

  RCCHECK(rclc_publisher_init_default(
    &pub_valor, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "sensor/valor"));

  RCCHECK(rclc_publisher_init_default(
    &pub_alerta, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
    "sensor/alerta"));

  RCCHECK(rclc_timer_init_default2(
    &timer, &support, RCL_MS_TO_NS(100), ao_tick, true));

  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
}

void loop() {
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10)));
}