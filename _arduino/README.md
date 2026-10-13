# Camper Waterlevel – Anleitung Arduino IDE

Diese Anleitung bringt die Firmware mit der **Arduino IDE** auf den ESP32. Lies vorher in der [Übersicht](../README.md) die Abschnitte [Was du brauchst](../README.md#was-du-brauchst), [Welches Board?](../README.md#welches-board) und [Verkabelung](../README.md#verkabelung). Wenn du lieber mit VS Code arbeitest, nimm die [PlatformIO-Anleitung](../_platformio/README.md).

## Inhalt

1. [Arduino IDE installieren](#1-arduino-ide-installieren)
2. [ESP32-Board-Paket installieren](#2-esp32-board-paket-installieren)
3. [Libraries installieren](#3-libraries-installieren)
4. [LittleFS-Upload-Plugin installieren](#4-littlefs-upload-plugin-installieren)
5. [Projekt herunterladen](#5-projekt-herunterladen)
6. [Projekt öffnen und anpassen](#6-projekt-öffnen-und-anpassen)
7. [Board und Port einstellen](#7-board-und-port-einstellen)
8. [Firmware hochladen](#8-firmware-hochladen)
9. [Web-Oberfläche hochladen](#9-web-oberfläche-hochladen)
10. [Prüfen, ob der ESP läuft](#10-prüfen-ob-der-esp-läuft)
- [Updates per WLAN (OTA)](#updates-per-wlan-ota)
- [Fehlerbehebung](#fehlerbehebung)

## 1. Arduino IDE installieren
Lade die [Arduino IDE 2.x](https://www.arduino.cc/en/software) herunter und installiere sie.

## 2. ESP32-Board-Paket installieren
1. In der Arduino IDE links auf **Boardverwalter** (Board-Symbol) klicken.
2. Nach `esp32` suchen und **esp32 von Espressif Systems** in Version **3.x** installieren (getestet mit 3.3.12). Ohne dieses Paket gibt es in der Board-Auswahl kein ESP32.
3. Taucht das Paket nicht auf: unter **Datei → Einstellungen → Zusätzliche Boardverwalter-URLs** diese Adresse eintragen und erneut suchen:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

## 3. Libraries installieren
Links auf **Bibliotheksverwalter** (Bücher-Symbol) klicken und diese vier Libraries installieren. Achte auf den richtigen Autor:

| Library | Autor | Version |
| --- | --- | --- |
| PubSubClient | Nick O'Leary | aktuell |
| ArduinoJson | Benoit Blanchon | 7.x |
| ESP Async WebServer | **ESP32Async** | ab 3.12 |
| Async TCP | **ESP32Async** | ab 3.5 |

> Es gibt mehrere Libraries mit ähnlichem Namen. Nimm für den Webserver und TCP unbedingt die von **ESP32Async**. Ältere Versionen (z. B. von me-no-dev) passen nicht zum ESP32-Core 3.x, siehe [Fehlerbehebung](#fehlerbehebung).

## 4. LittleFS-Upload-Plugin installieren
Die Web-Oberfläche liegt im Ordner `data/` und muss zusätzlich zur Firmware in den Speicher des ESP. Die Arduino IDE 2.x kann das nicht von sich aus, dafür gibt es ein Plugin:
1. Die neueste `.vsix`-Datei von [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload/releases) herunterladen.
2. Die Datei in diesen Ordner legen (Ordner `plugins` bei Bedarf anlegen):
   - macOS / Linux: `~/.arduinoIDE/plugins/`
   - Windows: `C:\Users\<Benutzername>\.arduinoIDE\plugins\`
3. Arduino IDE neu starten.

## 5. Projekt herunterladen
1. Auf der GitHub-Startseite des Projekts **Code → Download ZIP** klicken und die ZIP-Datei entpacken.
2. Der Sketch liegt im Ordner **`_arduino/camper_waterlevel/`**. Der Ordnername passt bereits zur `.ino`-Datei, du musst nichts umbenennen oder verschieben:
   ```
   _arduino/camper_waterlevel/
   ├── camper_waterlevel.ino
   ├── config.h, sensor.cpp, … (alle .h- und .cpp-Dateien)
   └── data/
       └── index.html
   ```

## 6. Projekt öffnen und anpassen
1. In der Arduino IDE **Datei → Öffnen** und `_arduino/camper_waterlevel/camper_waterlevel.ino` wählen. Oben erscheinen alle Dateien des Projekts als Tabs.
2. Im Tab `config.h` die Platzhalter durch eigene Passwörter ersetzen:
   ```cpp
   #define DEFAULT_AP_PASSWORD   "waterlevel_ap_PW"   // eigenes Passwort, mindestens 8 Zeichen
   #define DEFAULT_OTA_PASSWORD  "waterlevel_ota_PW"  // eigenes Passwort für Updates per WLAN
   ```
3. **Nur beim klassischen ESP32-DevKit:** im selben Tab den Mess-Pin ändern:
   ```cpp
   #define ADC_PIN               34     // classic ESP32: GPIO34 (ADC1)
   ```
4. Speichern (`Strg+S` bzw. `Cmd+S`).

WLAN- und MQTT-Zugangsdaten trägst du **nicht** hier ein, sondern später in der Web-Oberfläche.

## 7. Board und Port einstellen
1. ESP per USB anschließen.
2. Oben in der Arduino IDE auf die Board-Auswahl klicken → **Anderes Board und Port auswählen**.
3. Links dein Board suchen und auswählen, rechts den Port wählen:
   - Windows: `COM…` (z. B. `COM9`). Steht dort nur `COM1` oder gar nichts, fehlt der USB-Treiber oder das Kabel ist ein reines Ladekabel.
   - macOS: `/dev/cu.usbmodem…` oder `/dev/cu.usbserial…`
4. Im Menü **Werkzeuge** prüfen:

| Einstellung | ESP32-C3 | Klassisches ESP32 |
| --- | --- | --- |
| Board | **ESP32C3 Dev Module** | **ESP32 Dev Module** |
| USB CDC On Boot | **Enabled** | – (gibt es nicht) |
| Flash Size | **4MB** | **4MB** |
| Partition Scheme | **Default 4MB with spiffs** | **Default 4MB with spiffs** |
| Upload Speed | 921600 (bei Problemen 115200) | 921600 (bei Problemen 115200) |

## 8. Firmware hochladen
1. Auf den **Pfeil nach rechts (Hochladen)** klicken. Der erste Durchlauf dauert einige Minuten.
2. Bricht der Upload mit `Wrong boot mode detected` oder `Failed to connect` ab, muss der ESP in den **Download-Modus**:
   1. **BOOT** gedrückt halten.
   2. **RST** (C3) bzw. **EN** (klassisches ESP32) kurz drücken und loslassen.
   3. **BOOT weiter gedrückt halten** und den Upload erneut starten.
   4. BOOT erst loslassen, wenn unten `Writing at …` erscheint.
3. Am Ende steht `Hash of data verified`. Danach einmal **RST/EN** drücken, damit die Firmware startet.

## 9. Web-Oberfläche hochladen
1. **Serial Monitor schließen**, sonst ist der Port belegt.
2. `Cmd+Shift+P` (macOS) bzw. `Strg+Umschalt+P` (Windows/Linux) drücken.
3. `LittleFS` eintippen und **Upload LittleFS to Pico/ESP8266/ESP32** auswählen.
4. Kommt wieder `Wrong boot mode detected`: wie in [Schritt 8](#8-firmware-hochladen) BOOT gedrückt halten, RST/EN kurz drücken und BOOT halten, bis `Writing at …` erscheint.
5. Fertig ist es bei `LittleFS upload completed!`. Dann einmal **RST/EN** drücken.

In der Ausgabe steht oben `Device: ESP32 series, model …`. Dort muss `esp32c3` bzw. `esp32` zu deinem Board passen, sonst ist in Schritt 7 das falsche Board gewählt.

> Das Hochladen der Web-Oberfläche überschreibt das ganze Dateisystem und damit auch die gespeicherte Konfiguration. Bei späteren Updates reicht meist der Firmware-Upload.

## 10. Prüfen, ob der ESP läuft
**Werkzeuge → Serieller Monitor** öffnen und **115200 Baud** einstellen. Nach einem Druck auf RST/EN solltest du unter anderem sehen:
```
[WiFi] AP OK: Camper-Waterlevel IP: 192.168.4.1 ...
[OTA] Ready
[MAIN] Setup done!
```
Ohne Serial Monitor: Nach etwa 20 Sekunden erscheint auf dem Handy in der WLAN-Liste das Netz **`Camper-Waterlevel`**.

**Fertig.** Weiter geht es in der Übersicht mit der [Ersten Einrichtung](../README.md#erste-einrichtung).

## Updates per WLAN (OTA)

Nach dem ersten Upload per USB lässt sich die Firmware auch über WLAN aktualisieren:
1. Rechner und ESP müssen im selben Netz sein.
2. Unter **Werkzeuge → Port** erscheint im Bereich *Network ports* der Eintrag **camper-waterlevel at …**. Diesen auswählen.
3. Normal hochladen. Beim ersten Mal fragt die IDE nach dem OTA-Passwort (`DEFAULT_OTA_PASSWORD` aus `config.h`).

OTA gilt nur für die Firmware. Die Web-Oberfläche (`data/`) wird weiter per USB hochgeladen.

## Fehlerbehebung

**In der Board-Auswahl gibt es kein ESP32**
Das Board-Paket fehlt, siehe [Schritt 2](#2-esp32-board-paket-installieren).

**Kein passender Port (Windows: nur `COM1` oder gar keiner)**
Datenkabel verwenden und einen anderen USB-Anschluss probieren. Klassische ESP32-DevKits brauchen unter Windows oft einen Treiber für den USB-Chip: [CP210x (Silicon Labs)](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) oder [CH340 (WCH)](https://www.wch-ic.com/downloads/CH341SER_EXE.html). Welcher Chip verbaut ist, steht auf dem kleinen Chip neben der USB-Buchse. Danach Arduino IDE neu starten.

**Beim Öffnen fragt die IDE, ob sie einen Ordner anlegen und die Datei verschieben soll**
Du hast die `.ino` aus einem falsch benannten Ordner geöffnet. Wähle **Abbrechen** und öffne `_arduino/camper_waterlevel/camper_waterlevel.ino`. Wenn du zustimmst, verschiebt die IDE nur die `.ino`, und die anderen Dateien fehlen beim Kompilieren.

**Kompilierfehler `'HTTP_GET' was not declared` / `'HTTP_ANY' was not declared` oder `passing 'const AsyncServer' as 'this' argument discards qualifiers`**
Im Ordner `Dokumente/Arduino/libraries` liegen veraltete Versionen von ESPAsyncWebServer oder AsyncTCP. Lösche die Ordner `ESPAsyncWebServer` und `AsyncTCP` und installiere die aktuellen Versionen von **ESP32Async** neu (Bibliotheksverwalter oder als ZIP von GitHub: [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer), [AsyncTCP](https://github.com/ESP32Async/AsyncTCP)). Danach die Arduino IDE neu starten. Getestet mit ESPAsyncWebServer 3.12.1 und AsyncTCP 3.5.0.

**Kompilierfehler `… No such file or directory` bei einer Library**
Eine der vier Libraries aus [Schritt 3](#3-libraries-installieren) fehlt.

**Warnung `Multiple libraries were found for "WiFi.h"`**
Harmlos, die IDE nimmt die WiFi-Library aus dem ESP32-Core. Zum Ausblenden einen eventuell vorhandenen Ordner `Dokumente/Arduino/libraries/WiFi` löschen.

**`Wrong boot mode detected (0x13)` oder `Failed to connect to ESP32`**
Der ESP ist nicht im Download-Modus. BOOT gedrückt halten, RST/EN kurz drücken, BOOT halten und den Upload starten, BOOT erst bei `Writing at …` loslassen. Hilft das nicht, unter **Werkzeuge → Upload Speed** `115200` wählen.

**Upload schlägt fehl: `port is busy` oder `Resource busy`**
Ein anderes Programm belegt den Port, meist der Serial Monitor oder Serial Plotter. Schließen, ggf. USB-Kabel neu einstecken. Unter macOS zeigt `lsof /dev/cu.usbmodem*`, welches Programm den Port hält.

**Keine Ausgabe im Serial Monitor**
115200 Baud wählen. Beim ESP32-C3 zusätzlich **USB CDC On Boot: Enabled** setzen und die Firmware neu hochladen.

**„Upload LittleFS to Pico/ESP8266/ESP32“ fehlt in der Befehlsliste**
Das Plugin liegt nicht im richtigen Ordner oder die IDE wurde danach nicht neu gestartet, siehe [Schritt 4](#4-littlefs-upload-plugin-installieren).

**Web-Oberfläche zeigt „Please upload index.html to LittleFS“**
Der Upload des Ordners `data/` fehlt, siehe [Schritt 9](#9-web-oberfläche-hochladen). Firmware und `data/` sind zwei getrennte Uploads.

**ESP erscheint nicht als Netzwerk-Port (OTA)**
Der erste Upload muss per USB erfolgen. ESP und Rechner müssen im selben Netz sein, und die Firewall muss eingehende Verbindungen der Arduino IDE erlauben. Bei Bedarf die IDE neu starten.

Weitere Probleme im Betrieb (WLAN, MQTT, Messwerte) stehen in der [Übersicht](../README.md#fehlerbehebung-im-betrieb).
