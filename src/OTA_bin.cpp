#include "OTA_bin.h"

#ifdef ESP8266
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
WebServer server(80);
#endif

OTA_bin::OTA_bin(const char *ap_ssid, const char *ap_pass) {
  _ap_ssid = ap_ssid;
  _ap_pass = ap_pass;
}

void OTA_bin::activateOTA_bin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(_ap_ssid, _ap_pass);
  _ip = WiFi.softAPIP();

  // Iniciar servidor DNS tipo Captive Portal (opcional)
  dnsServer.start(53, "*", _ip);

  principalPage();
  _modoOTA_activo = true;

  Serial.print(F("馃煝 OTA Web Server listo en: http://"));
  Serial.print(_ip.toString());
  Serial.println(F("/update"));
}

void OTA_bin::principalPage() {
  server.on("/", [this]() {
    server.send(200, "text/html",
                String("<h1>OTA - ") + CHIP_MODEL +
                "</h1><p><a href='/update'>Actualizar firmware</a></p>");
  });

  server.on("/update", HTTP_GET, [this]() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", _updateForm);
  });

#ifdef ESP8266
  httpUpdater.setup(&server);
#endif

#if defined(ESP32)
  server.on("/update", HTTP_POST,
    []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain",
                  (Update.hasError()) ? F("Fallo en la actualizaci贸n") : F("Actualizaci贸n exitosa. Reiniciando..."));
      delay(1000);
      RESET;
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Actualizando: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.println(F("Actualizaci贸n correcta"));
        } else {
          Update.printError(Serial);
        }
      }
    });
#endif

  // Redirecci贸n tipo portal cautivo
  server.onNotFound([this]() {
    server.sendHeader("Location", String("http://") + _ip.toString() + "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}

void OTA_bin::deactivateOTA_bin() {
  _modoOTA_activo = false;
  dnsServer.stop();
  server.stop();
  WiFi.softAPdisconnect(true);
  Serial.println(F("馃敶 OTA desactivado"));
}

void OTA_bin::updateData() {
  if (_modoOTA_activo) {
    dnsServer.processNextRequest();  // Redirecci贸n tipo Captive Portal
    server.handleClient();
  }
}

bool OTA_bin::isActive() {
  return _modoOTA_activo;
}
