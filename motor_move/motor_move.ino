//esp32 Movement

#define LEFT_REVERSE 16     
#define LEFT_FORWARD 17    
#define RIGHT_FORWARD 19    
#define RIGHT_REVERSE 18  

void setup() {
  pinMode(LEFT_REVERSE, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_REVERSE, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  Serial.begin(115200);
}

void stopMotors() {
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(LEFT_REVERSE, LOW);
  digitalWrite(RIGHT_REVERSE, LOW);
}

void loop() {
  if (Serial.available() > 0)
   {
    char cmd = Serial.read();

    if (cmd == '\n' || cmd == '\r') {
      return;
    }

    cmd = toupper(cmd);

    stopMotors();

    switch (cmd) {
      case 'F': // Forward
        digitalWrite(LEFT_FORWARD, HIGH);
        digitalWrite(RIGHT_FORWARD, HIGH);
        break;

      case 'B': // Backward
        digitalWrite(LEFT_REVERSE, HIGH);
        digitalWrite(RIGHT_REVERSE, HIGH);
        break;

      case 'L': // Turn Left
        digitalWrite(LEFT_REVERSE, HIGH);
        digitalWrite(RIGHT_FORWARD, HIGH);
        break;

      case 'R': // Turn Right
        digitalWrite(LEFT_FORWARD, HIGH);
        digitalWrite(RIGHT_REVERSE, HIGH);
        break;

      case 'S': // Stop
        stopMotors();
        break;

      default:
        Serial.println("Unknown command. Use F, B, L, R, S.");
        break;
    }
  }
}
