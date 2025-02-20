#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "../../../WiringPi/wiringPi/wiringPi.h"

#define MOTOR_IN1 23 // GPIO Pin for L298N IN1
#define MOTOR_IN2 24 // GPIO Pin for L298N IN2
#define MOTOR_ENA 25 // GPIO Pin forL298N enable (PWM Speed Control)

/**
 * Initializes GPIO, hardware and sensors.
 * If compiled with gcc needs link
 *      Ex: gcc -o myapp myapp.c -l wiringPi
 * @return None
 */
void setupMotor() {
  LOG(INFO) << "Motor System Initialization...";

  // uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupGpio();

  // pin mode ..(INPUT, OUTPUT, PWM_OUTPUT, GPIO_CLOCK)
  // Setup DC Motor Driver Pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT); // PWM_OUTPUT?

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, 128); // Set inital speed to 50% (0-255 range)

  LOG(INFO) << "Motor System Initialized.";
}

/**
 * Controls platform rotation with L298N motor driver.
 * Rotates for a fixed duration.
 * According to current calculations in hardware-chat:
 * 10RPM = 4RPM -> ~ 1.875 sec for 45 degrees || 15 sec for full rotation.
 *
 * @return None
 */
void rotateMotor() {
  digitalWrite(MOTOR_IN1, HIGH); // HIGH,LOW == forward LOW,HIGH == backwards
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_ENA, 255); // Adjust speed
  usleep(1875000);             // Rotate for 1.875sec (roughly 45 degrees)
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW); // HIGH,HIGH || LOW,LOW == off
}