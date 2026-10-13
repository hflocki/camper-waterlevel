# Camper Waterlevel – Anleitung PlatformIO

Diese Anleitung bringt die Firmware mit **PlatformIO** (in VS Code oder im Terminal) auf den ESP32. Lies vorher in der [Übersicht](../README.md) die Abschnitte [Was du brauchst](../README.md#was-du-brauchst), [Welches Board?](../README.md#welches-board) und [Verkabelung](../README.md#verkabelung). Wenn du PlatformIO noch nie benutzt hast, ist die [Arduino-IDE-Anleitung](../_arduino/README.md) meist der einfachere Weg.

Gegenüber der Arduino IDE musst du hier nichts von Hand einstellen: Board, Partitionstabelle, Dateisystem und Libraries stehen fertig in der `platformio.ini` und werden beim ersten Build automatisch geladen.

> **Gemeinsamer Quellcode:** Dieser Ordner enthält nur die `platformio.ini`. Sie baut den Code aus `../_arduino/camper_waterlevel/`. Die Datei `config.h`, die du anpassen musst, liegt deshalb dort. Lade immer das **ganze Repo** herunter, nicht nur diesen Ordner.

## Inhalt

1. [PlatformIO installieren](#1-platformio-installieren)
2. [Projekt herunterladen und öffnen](#2-projekt-herunterladen-und-öffnen)
3. [config.h anpassen](#3-configh-anpassen)
4. [Board-Umgebung wählen](#4-board-umgebung-wählen)
5. [Firmware hochladen](#5-firmware-hochladen)
6. [Web-Oberfläche hochladen](#6-web-oberfläche-hochladen)
7. [Prüfen, ob der ESP läuft](#7-prüfen-ob-der-esp-läuft)
- [Befehle im Terminal](#befehle-im-terminal)
- [Updates per WLAN (OTA)](#updates-per-wlan-ota)
- [Fehlerbehebung](#fehlerbehebung)

## 1. PlatformIO installieren
1. [Visual Studio Code](https://code.visualstudio.com/) installieren.
2. In VS Code links die **Erweiterungen** öffnen (Symbol mit den vier Quadraten), nach **PlatformIO IDE** suchen und installieren.
3. Warten, bis PlatformIO fertig eingerichtet ist, und VS Code neu starten, wenn es dazu auffordert.

Wer lieber im Terminal arbeitet, kann stattdessen die [PlatformIO Core CLI](https://platformio.org/install/cli) nutzen (unter macOS z. B. `brew install platformio`), siehe [Befehle im Terminal](#befehle-im-terminal).

## 2. Projekt herunterladen und öffnen
1. Auf der GitHub-Startseite des Projekts **Code → Download ZIP** klicken und entpacken, oder `git clone <Repo-URL>`.
2. In VS Code **Datei → Ordner öffnen** und den Ordner **`_platformio`** wählen, also den Ordner, in dem die `platformio.ini` liegt. Nicht den Hauptordner des Repos und nicht `_arduino` öffnen.
3. PlatformIO erkennt das Projekt und lädt beim ersten Mal Plattform, Arduino-Core und Libraries herunter. Das dauert einige Minuten, unten rechts siehst du den Fortschritt.

## 3. config.h anpassen
Öffne `../_arduino/camper_waterlevel/config.h` (in VS Code über **Datei → Öffnen**) und passe an:

1. Eigene Passwörter statt der Platzhalter:
   ```cpp
   #define DEFAULT_AP_PASSWORD   "waterlevel_ap_PW"   // eigenes Passwort, mindestens 8 Zeichen
   #define DEFAULT_OTA_PASSWORD  "waterlevel_ota_PW"  // eigenes Passwort für Updates per WLAN
   ```
2. **Nur beim klassischen ESP32-DevKit:** den Mess-Pin ändern:
   ```cpp
   #define ADC_PIN               34     // classic ESP32: GPIO34 (ADC1)
   ```

WLAN- und MQTT-Zugangsdaten trägst du **nicht** hier ein, sondern später in der Web-Oberfläche.

## 4. Board-Umgebung wählen
Die `platformio.ini` enthält eine Umgebung pro Board:

| Board | Umgebung |
| --- | --- |
| ESP32-C3 (z. B. Super Mini) | **`esp32c3`** (Standard) |
| Klassisches ESP32-DevKit | **`esp32dev`** |

In VS Code unten in der blauen Statusleiste auf **`Default (_platformio)`** bzw. den Namen der Umgebung klicken und die passende auswählen. Beim ESP32-C3 musst du nichts tun, `esp32c3` ist voreingestellt.

## 5. Firmware hochladen
1. ESP per USB anschließen.
2. In der blauen Statusleiste auf den **Pfeil nach rechts (Upload)** klicken. Alternativ links das **PlatformIO-Symbol** (Ameisenkopf) → **Project Tasks → <Umgebung> → General → Upload**.
3. PlatformIO sucht den USB-Port selbst. Bricht der Upload mit `Wrong boot mode detected` oder `Failed to connect` ab, muss der ESP in den **Download-Modus**:
   1. **BOOT** gedrückt halten.
   2. **RST** (C3) bzw. **EN** (klassisches ESP32) kurz drücken und loslassen.
   3. **BOOT weiter gedrückt halten** und den Upload erneut starten.
   4. BOOT erst loslassen, wenn `Writing at …` erscheint.
4. Am Ende steht `Hash of data verified` und `SUCCESS`. Danach einmal **RST/EN** drücken.

## 6. Web-Oberfläche hochladen
1. Einen offenen Serial Monitor schließen, sonst ist der Port belegt.
2. Links das **PlatformIO-Symbol** → **Project Tasks → <Umgebung> → Platform → Upload Filesystem Image**.
3. Kommt `Wrong boot mode detected`: wie in [Schritt 5](#5-firmware-hochladen) den Download-Modus herstellen.
4. Fertig ist es bei `SUCCESS`. Dann einmal **RST/EN** drücken.

> Das Hochladen der Web-Oberfläche überschreibt das ganze Dateisystem und damit auch die gespeicherte Konfiguration. Bei späteren Updates reicht meist der Firmware-Upload.

## 7. Prüfen, ob der ESP läuft
In der blauen Statusleiste auf den **Stecker (Serial Monitor)** klicken. Nach einem Druck auf RST/EN solltest du unter anderem sehen:
```
[WiFi] AP OK: Camper-Waterlevel IP: 192.168.4.1 ...
[OTA] Ready
[MAIN] Setup done!
```
Ohne Serial Monitor: Nach etwa 20 Sekunden erscheint auf dem Handy in der WLAN-Liste das Netz **`Camper-Waterlevel`**.

**Fertig.** Weiter geht es in der Übersicht mit der [Ersten Einrichtung](../README.md#erste-einrichtung).

## Befehle im Terminal

Im Ordner `_platformio` ausführen. Für ein klassisches ESP32 jeweils `-e esp32dev` anhängen.

```
pio run                  # nur bauen
pio run -t upload        # Firmware per USB hochladen
pio run -t uploadfs      # Web-Oberfläche (data/) hochladen
pio device monitor       # Serial Monitor, beenden mit Strg+C
```

Mehrere USB-Geräte angeschlossen? Port festlegen mit `--upload-port`, z. B. `pio run -t upload --upload-port COM9` oder `--upload-port /dev/cu.usbmodem1201`.

## Updates per WLAN (OTA)

Nach dem ersten Upload per USB lässt sich die Firmware auch über WLAN aktualisieren. Rechner und ESP müssen im selben Netz sein. Das OTA-Passwort wird nicht in der `platformio.ini` gespeichert, sondern beim Aufruf übergeben:

macOS / Linux:
```
OTA_PASSWORD=dein-ota-passwort pio run -e ota -t upload
```
Windows (PowerShell):
```
$env:OTA_PASSWORD="dein-ota-passwort"; pio run -e ota -t upload
```

Die Umgebung `ota` baut für den ESP32-C3. Für ein klassisches ESP32 in der `platformio.ini` bei `[env:ota]` die Zeile `extends = env:esp32c3` in `extends = env:esp32dev` ändern. Wird `camper-waterlevel.local` nicht gefunden, bei `upload_port` die IP-Adresse des ESP eintragen.

OTA gilt nur für die Firmware. Die Web-Oberfläche (`data/`) wird weiter per USB hochgeladen.

## Fehlerbehebung

**`Wrong boot mode detected (0x13)` oder `Failed to connect to ESP32`**
Der ESP ist nicht im Download-Modus. BOOT gedrückt halten, RST/EN kurz drücken, BOOT halten und den Upload starten, BOOT erst bei `Writing at …` loslassen. Hilft das nicht, in der `platformio.ini` bei der Umgebung `upload_speed = 115200` ergänzen.

**Kein Port gefunden / `Please specify upload_port`**
Datenkabel verwenden und einen anderen USB-Anschluss probieren. Klassische ESP32-DevKits brauchen unter Windows oft einen Treiber: [CP210x (Silicon Labs)](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) oder [CH340 (WCH)](https://www.wch-ic.com/downloads/CH341SER_EXE.html). Mit `pio device list` siehst du alle erkannten Ports.

**Upload schlägt fehl: `port is busy` oder `Resource busy`**
Ein offener Serial Monitor belegt den Port. Schließen und erneut hochladen. Unter macOS zeigt `lsof /dev/cu.usbmodem*`, welches Programm den Port hält.

**`src_dir` oder Quelldateien werden nicht gefunden**
Es wurde nur der Ordner `_platformio` heruntergeladen oder verschoben. Die `platformio.ini` erwartet den Code in `../_arduino/camper_waterlevel/`. Das ganze Repo herunterladen und die Ordnerstruktur beibehalten.

**Falsches Board gebaut (`model esp32` statt `esp32c3` oder umgekehrt)**
Die falsche Umgebung ist gewählt, siehe [Schritt 4](#4-board-umgebung-wählen).

**Keine Ausgabe im Serial Monitor**
Beim ESP32-C3 sind die nötigen USB-Einstellungen in der Umgebung `esp32c3` gesetzt. Prüfe, ob du die richtige Umgebung hochgeladen hast, und drücke einmal RST.

**Download von Plattform oder Libraries schlägt fehl**
Beim ersten Build braucht PlatformIO Zugriff auf GitHub und die PlatformIO-Registry. Hinter einer Firewall oder einem Proxy muss dessen Zertifikat im System hinterlegt sein.

**Web-Oberfläche zeigt „Please upload index.html to LittleFS“**
Der Upload des Dateisystems fehlt, siehe [Schritt 6](#6-web-oberfläche-hochladen).

**OTA-Upload findet den ESP nicht**
Der erste Upload muss per USB erfolgen. ESP und Rechner müssen im selben Netz sein, und `camper-waterlevel.local` muss per mDNS auflösbar sein. Sonst die IP-Adresse bei `upload_port` eintragen.

Weitere Probleme im Betrieb (WLAN, MQTT, Messwerte) stehen in der [Übersicht](../README.md#fehlerbehebung-im-betrieb).
