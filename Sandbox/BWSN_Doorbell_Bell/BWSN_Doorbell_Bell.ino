int BELL = 5;

void setup() {
  pinMode(BELL, OUTPUT);
  Serial.begin(9600);
}


void loop() {
  if (Serial.available() > 0) {
    if (Serial.read() == 'D') {
      Serial.print( 'K' );
      
      digitalWrite(BELL, HIGH);
      delay(10);
      digitalWrite(BELL, LOW);
    }
  }
}
