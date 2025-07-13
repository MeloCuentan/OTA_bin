Probada en ESP8266, ESP32C3 y ESP32
Crea la opción de actualizar el firmware vía WiFi con el archivo .bin.
En el ejemplo, hay que escribir "update" en el monitor serie para activar el modo OTA, y "exit" para salir.
Al activar se genera un AP que de forma predeterminada es "ESPx-OTA" con contraseña "12345678" (Se pueden modificar en el constructor)
Al conectarnos debería redirigirnos automáticamente.
