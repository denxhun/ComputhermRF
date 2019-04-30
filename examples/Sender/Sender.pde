#include <ComputhermRF.h>

ComputhermRF rf = ComputhermRF(255, 4);

bool on = true;
String address = "123AB";

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
}

void loop() {
  Serial.print(address + ": ");
  digitalWrite(2, LOW);
  rf.sendMessage(address, on);
  digitalWrite(2, HIGH);
  Serial.println(on?"ON":"OFF");
  on = !on;
  delay(5000);
}
