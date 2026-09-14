#include "OTA_bin.h"

// Pagina bonita usada por attachToServer() cuando se monta sobre un servidor
// existente (mismo estilo oscuro que el resto de la configuracion). Se declara
// fuera de los #ifdef porque la usan tanto el ESP8266 como el ESP32.
static const char OTA_INLINE_PAGE[] PROGMEM = R"rawinline(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate">
<title>Actualizar firmware</title>
<style>
body{background:#071018;color:#edf4ff;font-family:"Segoe UI",sans-serif;min-height:100vh;margin:0;display:flex;align-items:center;justify-content:center;}
.box{max-width:600px;margin:20px;padding:28px;background:#0d1722;border:1px solid #27415b;border-radius:20px;}
h1{font-size:1.4rem;margin:0 0 12px;}
p{line-height:1.7;color:#97a9c4;}
code{background:#172536;padding:2px 8px;border-radius:8px;color:#5fe0ff;}
input[type=file]{width:100%;padding:10px;margin:6px 0 16px;background:#172536;color:#edf4ff;border:1px solid #27415b;border-radius:12px;}
button{appearance:none;border:0;border-radius:12px;padding:12px 18px;background:linear-gradient(135deg,#5fe0ff,#86ffca);color:#071018;font-weight:700;font-size:1rem;cursor:pointer;}
a{color:#5fe0ff;}
</style>
</head>
<body>
<div class="box">
<h1>Actualizar firmware</h1>
<p>Selecciona el archivo <code>firmware.bin</code> y pulsa subir. No te desconectes hasta que termine; el dispositivo se reiniciara solo al terminar.</p>
<form method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="firmware" accept=".bin">
<button type="submit">Subir y actualizar</button>
</form>
<p style="margin-top:18px;"><a href="/">Volver a la configuracion</a></p>
</div>
</body>
</html>
)rawinline";

#ifdef ESP8266
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
WebServer server(80);
HTTPUpdateServer httpUpdater;
#endif

OTA_bin::OTA_bin(const char *ap_ssid, const char *ap_pass) {
  _ap_ssid = ap_ssid;
  _ap_pass = ap_pass;
}

void OTA_bin::attachToServer(OTA_WEB_SERVER *srv, const char *pagePath) {
  if (!srv) {
    return;
  }
  // Pagina del formulario de subida en la ruta indicada (sobre el servidor actual).
  srv->on(pagePath, HTTP_GET, [srv]() {
    srv->sendHeader("Connection", "close");
    srv->send_P(200, PSTR("text/html"), OTA_INLINE_PAGE);
  });
  // El updater oficial gestiona el POST /update (subida del .bin y reinicio).
  // En ESP8266 es ESP8266HTTPUpdateServer.
  //
  // En ESP32 NO se registra aqui: el sketch ya monta su propio POST /update
  // (para poder pausar los LEDs durante la subida) y lo registra ANTES que
  // esta funcion. Como el servidor del ESP32 atiende la primera ruta que
  // coincide, registrar otra aqui seria redundante y podria pisarla.
#ifndef ESP32
  httpUpdater.setup(srv);
#endif
}

void OTA_bin::activateOTA_bin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(_ap_ssid, _ap_pass);
  _ip = WiFi.softAPIP();

  // Iniciar servidor DNS tipo Captive Portal (opcional)
  dnsServer.start(53, "*", _ip);

  principalPage();
  _modoOTA_activo = true;

  Serial.print(F("🟢 OTA Web Server listo en: http://"));
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
                  (Update.hasError()) ? F("Fallo en la actualización") : F("Actualización exitosa. Reiniciando..."));
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
          Serial.println(F("Actualización correcta"));
        } else {
          Update.printError(Serial);
        }
      }
    });
#endif

  // Redirección tipo portal cautivo
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
  Serial.println(F("🔴 OTA desactivado"));
}

void OTA_bin::updateData() {
  if (_modoOTA_activo) {
    dnsServer.processNextRequest();  // Redirección tipo Captive Portal
    server.handleClient();
  }
}

bool OTA_bin::isActive() {
  return _modoOTA_activo;
}
