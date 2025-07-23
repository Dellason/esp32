#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Motor control pins
#define LEFT_REVERSE 16     
#define LEFT_FORWARD 17    
#define RIGHT_FORWARD 19    
#define RIGHT_REVERSE 18

Adafruit_MPU6050 mpu;

// Movement tracking
float velocity = 0.0;
float distance = 0.0;
unsigned long lastTime = 0;

// Control direction globally
int directionMultiplier = 1;  // 1 = forward, -1 = backward

void setup() {
  pinMode(LEFT_REVERSE, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_REVERSE, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  Serial.begin(115200);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected!");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  lastTime = micros();
}

void stopMotors() {
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(LEFT_REVERSE, LOW);
  digitalWrite(RIGHT_REVERSE, LOW);
}

void moveDistance(float targetDistance) {
  velocity = 0.0;
  distance = 0.0;
  lastTime = micros();

  // Set direction
  if (directionMultiplier == 1) {
    digitalWrite(LEFT_FORWARD, HIGH);
    digitalWrite(RIGHT_FORWARD, HIGH);
  } else {
    digitalWrite(LEFT_REVERSE, HIGH);
    digitalWrite(RIGHT_REVERSE, HIGH);
  }

  while (distance < targetDistance) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    unsigned long currentTime = micros();
    float dt = (currentTime - lastTime) / 1000000.0; // seconds
    lastTime = currentTime;

    float accX = a.acceleration.x;

    // Apply direction multiplier (forward = +, backward = -)
    accX *= directionMultiplier;

    // Subtract known bias here
    velocity += accX * dt;
    distance += abs(velocity * dt);  
    
    Serial.print("Distance: ");
    Serial.println(distance);
  }

  stopMotors();
  Serial.println("Target distance reached");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); 
    input.trim(); 
    if (input.length() == 0) return;

    char cmd = toupper(input.charAt(0));
    float value = input.substring(1).toFloat();

    stopMotors();

    switch (cmd) {
      case 'F': // Forward by distance
        directionMultiplier = 1;
        moveDistance(value);
        break;

      case 'B': // Backward by distance
        directionMultiplier = -1;
        moveDistance(value);
        break;

      case 'L': // Turn Left (fixed delay)
        digitalWrite(LEFT_REVERSE, HIGH);
        digitalWrite(RIGHT_FORWARD, HIGH);
        delay(600);
        stopMotors();
        break;

      case 'R': // Turn Right (fixed delay)
        digitalWrite(LEFT_FORWARD, HIGH);
        digitalWrite(RIGHT_REVERSE, HIGH);
        delay(600);
        stopMotors();
        break;

      case 'S': // Stop
        stopMotors();
        break;

      default:
        Serial.println("Unknown command. Use F#, B#, L, R, S.");
        break;
    }
  }
}
