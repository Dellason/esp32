#include <MPU6050_tockn.h>
#include <Wire.h>
#include <Math.h>
#include "helpers.h"

// Motor pins (adjust depending on your wiring)
const int in1 = 9, in2 = 5, in3 = 4, in4 = 3, led = 13;

float thetaDesired = 0.0f;
float error_tolerance = 2.80f;

long prevT = 0;
int nominalSpeed = 255;

MPU6050 gyro(Wire);
Command command_u;

void setup() {
  delay(1000);
  Serial.begin(9600);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(led, OUTPUT);

  Wire.begin();
  gyro.begin();
  gyro.calcGyroOffsets(true);

  digitalWrite(led, HIGH);
  delay(300);
  digitalWrite(led, LOW);
  delay(300);
  digitalWrite(led, HIGH);

  prevT = micros();

  Serial.println("System ready...");
}

void loop() {
  // ✅ Check if a new command arrived
  if (Serial.available() > 0) {
    String commandData = Serial.readStringUntil('\n');
    command_u = parseCommand(commandData);

    if (command_u.desiredAction != CommandType::NONE &&
        command_u.desiredAction != CommandType::ERROR) {

      Serial.print("Executing: ");
      Serial.println(commandData);

      doMove(command_u.desiredAction, command_u.SubCommand);

      stopMotors();  // ✅ stop after finishing
      Serial.println("Command finished");
    } 
    else {
      Serial.println("Unknown/Invalid command received.");
    }
  }
}

/* ---------------- MOTOR CONTROL ---------------- */

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void doMove(CommandType action, float orientation, int distance = 0) {
  thetaDesired = orientation;

  // Get orientation from MPU
  gyro.update();
  float thetaZ = gyro.getGyroAngleZ();

  double thetaError = thetaDesired - thetaZ;

  // Simple proportional controller
  double kp = 0.025;
  double correction = kp * thetaError;

  int baseSpeed = 150;   // enough to move motors
  int leftSpeed = baseSpeed - correction;
  int rightSpeed = baseSpeed + correction;

  leftSpeed  = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  switch (action) {
    case CommandType::FORWARD:
      moveForward(leftSpeed, rightSpeed);
      break;

    case CommandType::BACKWARD:
      moveBackward(leftSpeed, rightSpeed);
      break;

    case CommandType::LEFT:
      turnInPlace(150, 0); // left
      delay(orientation * 10); // crude timing
      break;

    case CommandType::RIGHT:
      turnInPlace(150, 1); // right
      delay(orientation * 10);
      break;

    default:
      stopMotors();
      break;
  }
}

void moveForward(int lSpeed, int rSpeed) {
  analogWrite(in1, 0);
  analogWrite(in2, rSpeed);
  analogWrite(in3, 0);
  analogWrite(in4, lSpeed);
}

void moveBackward(int lSpeed, int rSpeed) {
  analogWrite(in1, rSpeed);
  analogWrite(in2, 0);
  analogWrite(in3, lSpeed);
  analogWrite(in4, 0);
}

void turnInPlace(uint8_t speed, uint8_t direction) {
  if (direction == 0) {
    // turn left
    analogWrite(in1, speed);
    analogWrite(in2, 0);
    analogWrite(in3, 0);
    analogWrite(in4, speed);
  } else {
    // turn right
    analogWrite(in1, 0);
    analogWrite(in2, speed);
    analogWrite(in3, speed);
    analogWrite(in4, 0);
  }
}
