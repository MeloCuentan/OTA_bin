#include <Arduino.h>
#include <OTA_bin.h>

OTA_bin OTA_BIN;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println(F("\n📦 INICIO DEL SISTEMA"));
  Serial.println("🚀 Modo normal iniciado en " + String(CHIP_MODEL));
}

void loop() {
  OTA_BIN.updateData();  // Para mantener activo el servidor OTA si está activo

  if (Serial.available()) {
    String rx = Serial.readStringUntil('\n');
    rx.trim();
    if (rx == "update") {
      Serial.println(F("🔁 Entrando en modo actualización OTA_bin"));
      OTA_BIN.activateOTA_bin();
    } else if (rx == "exit" && OTA_BIN.isActive()) {
      Serial.println(F("⛔ Saliendo del modo actualización OTA_bin"));
      OTA_BIN.deactivateOTA_bin();
    }
  }
}
