
int BUTTON = 2;
int LED = 11;

bool testLEDState = false;

void setup() {
  pinMode(BUTTON, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly: 
  if (digitalRead(BUTTON) == HIGH) {
    Serial.print('D');
    delay(10);
  }
  
  if (Serial.available() > 0) {
    if (Serial.read() == 'K') {
      digitalWrite(LED, HIGH);
    }
  }
  
  if (digitalRead(BUTTON) == LOW) {
    digitalWrite(LED, LOW);
  }
}


