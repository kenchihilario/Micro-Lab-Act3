const int TRIG_PIN = 5;
const int ECHO_PIN = 6; 
const int LDR_PIN  = A0;

const unsigned long SEND_INTERVAL = 2000;
unsigned long lastSend = 0;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1.0;
  return duration * 0.0343 / 2.0;
}

int readLightRaw() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(LDR_PIN);
    delay(2);
  }
  return sum / 10;
}

void setup() {
  Serial.begin(9600); 
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    float distance = readDistanceCm();
    int raw = readLightRaw();
    int lightPercent = map(raw, 0, 1023, 0, 100);

    Serial.print(distance, 1);
    Serial.print(",");
    Serial.print(lightPercent);
    Serial.print(",");
    Serial.println(raw);
  }
}
