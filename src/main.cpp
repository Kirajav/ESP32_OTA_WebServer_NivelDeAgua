#include "AppManager.h"

AppManager app;

void setup() {
  Serial.begin(115200);
  Serial.println("=== INICIANDO SISTEMA DIRECTO ===");
  app.initialize();
}

void loop() {
  app.loop();
}
