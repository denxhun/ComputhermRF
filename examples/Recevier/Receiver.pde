#include <ComputhermRF.h>

ComputhermRF rf = ComputhermRF(5, 255);

void setup() {
  Serial.begin(115200);
  Serial.println();
  rf.startReceiver();
  Serial.println("Computherm receiver started.");
}

void loop() {
  if (rf.isDataAvailable()) {
    computhermMessage msg = rf.getData();
    Serial.println("Nem message caught. Address: " + msg.address + " command: "
        + msg.command);
  }
}
