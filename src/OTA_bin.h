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
  // Tipo de servidor y de updater segun la plataforma (los usa attachToServer)
  #define OTA_WEB_SERVER ESP8266WebServer
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Update.h>
  #include <HTTPUpdateServer.h>
  #include <DNSServer.h>
  #include <esp_system.h>
  extern WebServer server;
  extern HTTPUpdateServer httpUpdater;
  #define OTA_WEB_SERVER WebServer
#endif

// OJO: aqui NO se definen macros RESET ni CHIP_MODEL.
//
// Definirlas en esta cabecera rompia la compilacion: FastLED tiene miembros y
// enumerados llamados RESET (TIMING::RESET en timing_traits.h / led_timing.h)
// y el preprocesador los sustituia por "ESP.restart()" al incluir OTA_bin.h
// antes que FastLED.h. Las versiones con prefijo OTA_BIN_ viven en
// OTA_bin_macros.h y solo las usa OTA_bin.cpp.

class OTA_bin {
public:
  OTA_bin(const char *AP_SSID = "ESPx-OTA", const char *AP_PASS = "12345678");
  void activateOTA_bin();
  void deactivateOTA_bin();
  void updateData();
  bool isActive();

  // MODIFICACION: monta la pagina de subida y el updater sobre un servidor web
  // YA EXISTENTE, sin crear ningun AP ni cambiar de red. pagePath es la ruta de
  // la pagina con el formulario (la subida se hace al POST /update interno).
  // Compatible con ESP8266 (ESP8266WebServer) y ESP32 (WebServer).
  void attachToServer(OTA_WEB_SERVER *srv, const char *pagePath = "/ota");

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
