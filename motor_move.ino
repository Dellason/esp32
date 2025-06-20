#define LEFT_REAR 16     // IN3
#define LEFT_FRONT 17    // IN4
#define RIGHT_REAR 19    // IN1
#define RIGHT_FRONT 18   // IN2


void setup() {
  pinMode(LEFT_REAR, OUTPUT);
  pinMode(LEFT_FRONT, OUTPUT);
  pinMode(RIGHT_REAR, OUTPUT);
  pinMode(RIGHT_FRONT, OUTPUT);
}

void loop() {
  // FORWARD

  // RIGHT MOTOR: forward
  digitalWrite(RIGHT_REAR, HIGH);
  digitalWrite(RIGHT_FRONT, LOW);

  // LEFT MOTOR: reverse the logic to go forward
  digitalWrite(LEFT_REAR, LOW);
  digitalWrite(LEFT_FRONT, HIGH);

  delay(1000);

  // STOP
  digitalWrite(LEFT_REAR, LOW);
  digitalWrite(LEFT_FRONT, LOW);
  digitalWrite(RIGHT_REAR, LOW);
  digitalWrite(RIGHT_FRONT, LOW);

  delay(5000);
}
