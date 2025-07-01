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
  serial.Begin(115200);
}

void stopMotors() {
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(LEFT_REVERSE, LOW);
  digitalWrite(RIGHT_REVERSE, LOW);
}


void loop() {
  //FORWARD
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(RIGHT_FORWARD, HIGH);
  delay(1000);

  //STOP
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  delay(500);

  //REVERSE
  digitalWrite(LEFT_REVERSE, HIGH);
  digitalWrite(RIGHT_REVERSE, HIGH);
  delay(1000);

  //TURN LEFT
  digitalWrite(RIGHT_FORWARD, HIGH);
  digitalWrite(LEFT_REVERSE, HIGH);
  delay(600);

  //STOP
  digitalWrite(LEFT_REVERSE, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  delay(500);

   //TURN RIGHT
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(RIGHT_REVERSE, HIGH);
  delay(500);

  //STOP
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(RIGHT_REVERSE, LOW);
  delay(1000);
}
