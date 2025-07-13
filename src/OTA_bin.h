#ifndef OTA_BIN_H
#define OTA_BIN_H

#include <Arduino.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266HTTPUpdateServer.h>
  #include <DNSServer.h>
  extern ESP8266WebServer server;
  extern ESP8266HTTPUpdateServer httpUpdater;
  #define RESET ESP.restart()
  #define CHIP_MODEL F("ESP8266")
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Update.h>
  #include <DNSServer.h>
  extern WebServer server;
  #define RESET esp_restart()
  #define CHIP_MODEL ESP.getChipModel()
#endif

class OTA_bin {
public:
  OTA_bin(const char *AP_SSID = "ESPx-OTA", const char *AP_PASS = "12345678");
  void activateOTA_bin();
  void deactivateOTA_bin();
  void updateData();
  bool isActive();

private:
  void principalPage();

  const char *_ap_ssid;
  const char *_ap_pass;
  IPAddress _ip;
  bool _modoOTA_activo = false;

#ifdef ESP8266
  DNSServer dnsServer;
#elif defined(ESP32)
  DNSServer dnsServer;
#endif

  const char* _updateForm =
      "<h2>Seleccione el archivo de firmware</h2>"
      "<p>Tipo de archivo admitido: <strong>.bin</strong></p>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='firmware' accept='.bin'>"
      "<input type='submit' value='Actualizar Firmware'>"
      "</form>";
};

#endif
