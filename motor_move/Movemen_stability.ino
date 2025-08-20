#include <MPU6050_tockn.h>
#include <Wire.h>

// Motor pins (adjust for your driver)
const int in1 = 9, in2 = 5, in3 = 6, in4 = 3, led = 13;

float thetaDesired = 0.0f;
float error_tolerance = 4.0f; // degrees

char cmd = 's';   // current command
float buf = 0.0f;      // numeric part of command
bool gotData = false;

MPU6050 gyro(Wire);

// Timing
unsigned long tripTime = 0;  
float currentTheta = 0.0f;
int initAngleZ = 0;

String commandData = "";

// ---------------------- Setup ----------------------
void setup() {
  delay(1000);
  Serial.begin(115200);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(led, OUTPUT);

  Wire.begin();
  gyro.begin();
  gyro.calcGyroOffsets(true);

  // Blink LED on init
  digitalWrite(led, HIGH); delay(300);
  digitalWrite(led, LOW);  delay(300);
  digitalWrite(led, HIGH);

  Serial.println("Setup complete.");
}

// ---------------------- Loop ----------------------
void loop() {
  // ---------------- Serial Parsing ----------------
  while (Serial.available() > 0) {
    commandData = Serial.readStringUntil('\n');
    gotData = true;
    yield();
  }

  if (gotData) {
    if (commandData.length() >= 1) {
      cmd = commandData[0];
      if (commandData.length() >= 3) {
        String subCmd = commandData.substring(1); // everything after cmd
        buf = subCmd.toFloat();
      }
      else buf = 0;

      tripTime = 0;    // reset timing
      initAngleZ = 0;  // reset turning state

      Serial.print("Got command: ");
      Serial.print(cmd);
      Serial.print(" ");
      Serial.println(buf);
    }
    gotData = false;
  }

  // Always update gyro
  gyro.update();

  // ---------------- Forward / Backward (time-based) ----------------
  if (cmd == 'f' || cmd == 'b') {
    if (tripTime == 0) tripTime = millis();  // mark start time
    unsigned long elapsed = millis() - tripTime;
    unsigned long targetTime = (unsigned long)(buf * 1000.0f);          // seconds to ms

    int isFwd = (cmd == 'f');
    int baseSpeed = 120; // adjust as needed

    // drive both motors forward or backward
    analogWrite(in1, baseSpeed * !isFwd);
    analogWrite(in2, baseSpeed * isFwd);
    analogWrite(in3, baseSpeed * !isFwd);
    analogWrite(in4, baseSpeed * isFwd);

    if (elapsed >= targetTime) {
      stopMotors();
      cmd = 's'; tripTime = 0;
      Serial.println("Forward/backward complete, stopping.");
    }
  }

  // ---------------- Turn Left / Right (angle-based) ----------------
  else if (cmd == 'l' || cmd == 'r') {
    if (!initAngleZ) {
      currentTheta = gyro.getAngleZ();
      thetaDesired = currentTheta + (cmd == 'r' ? buf : -buf);
      initAngleZ = 1;
    }

    double thetaError = thetaDesired - gyro.getAngleZ();
    int turnSpeed = 120; // 

    if (fabs(thetaError) > error_tolerance) {
      if (thetaError > 0) {
        analogWrite(in1, 0);
        analogWrite(in2, turnSpeed);
        analogWrite(in3,turnSpeed);
        analogWrite(in4, 0);
        }

      else {
         analogWrite(in1, turnSpeed);
        analogWrite(in2, 0);
        analogWrite(in3, 0);
        analogWrite(in4, turnSpeed);
      }
           
      }
    
    else {
      stopMotors();
      initAngleZ = 0; cmd = 's';
      Serial.println("Turn complete, stopping.");
    }
  }

  // ---------------- Stop ----------------
  else {
    stopMotors();
  }
}

// ---------------------- Helpers ----------------------
void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}