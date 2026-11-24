/*
  CMPE 311 - Laboratory Activity 2: Utilizing Arduino's EEPROM MADE BY KENCHI HILARIO
*/
#include <EEPROM.h>
 
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int HISTORY_SIZE = 5;
const int ADDR_MAGIC = 0;
const int ADDR_HEAD  = 1; 
const int ADDR_MIN   = 2;
const int ADDR_MAX   = 4; 
const int ADDR_HIST  = 6; 
const byte MAGIC     = 0xA5;
 
const unsigned long INTERVAL_MS = 5000UL; 
unsigned long lastRead = 0;
 
uint16_t readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);  
  if (duration == 0) return 0;                             
  return (uint16_t)(duration / 58UL);                      
}
 
void initEeprom() {
  uint16_t zero = 0, minInit = 0xFFFF;
  EEPROM.update(ADDR_HEAD, 0);
  EEPROM.put(ADDR_MIN, minInit);
  EEPROM.put(ADDR_MAX, zero);
  for (int i = 0; i < HISTORY_SIZE; i++) EEPROM.put(ADDR_HIST + i * 2, zero);
  EEPROM.update(ADDR_MAGIC, MAGIC);
}
 
void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  if (EEPROM.read(ADDR_MAGIC) != MAGIC) initEeprom();  
  lastRead = millis() - INTERVAL_MS;                  
}
 
void loop() {
  if (millis() - lastRead < INTERVAL_MS) return;
  lastRead = millis();
 
  uint16_t current = readDistanceCm();
  if (current == 0) {
    Serial.println("No echo received (out of range). Reading skipped.");
    return;
  }
 
  byte head = EEPROM.read(ADDR_HEAD);
  EEPROM.put(ADDR_HIST + head * 2, current);
  EEPROM.update(ADDR_HEAD, (head + 1) % HISTORY_SIZE);
 
  uint16_t minV, maxV;
  EEPROM.get(ADDR_MIN, minV);
  EEPROM.get(ADDR_MAX, maxV);
  if (current < minV) { minV = current; EEPROM.put(ADDR_MIN, minV); }
  if (current > maxV) { maxV = current; EEPROM.put(ADDR_MAX, maxV); }
 
  Serial.println("************************************************************");
  Serial.print("Current Reading: ");
  Serial.print(current);
  Serial.println(" cm");
  Serial.println("Historical Value:");
  Serial.print("Last five sensor reading: ");
  byte newHead = EEPROM.read(ADDR_HEAD);          
  bool first = true;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    uint16_t v;
    EEPROM.get(ADDR_HIST + ((newHead + i) % HISTORY_SIZE) * 2, v);
    if (v == 0) continue;                          
    if (!first) Serial.print(", ");
    Serial.print(v);
    first = false;
  }
  Serial.println(" (oldest to newest, cm)");
  Serial.print("Lowest Level: ");  Serial.print(minV); Serial.println(" cm");
  Serial.print("Highest Level: "); Serial.print(maxV); Serial.println(" cm");
  Serial.println("************************************************************");
}
