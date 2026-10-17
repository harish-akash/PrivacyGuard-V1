# PrivacyGuard V1: Portable Hidden Camera Detection System

**Version 1 (prototype) of PrivacyGuard X.** An ESP32-based handheld that checks a room for hidden cameras using three methods at the same time: RF detection, Wi-Fi scanning and IR (optical) reflection.

<!-- After you upload your photo, remove these comment marks:
![PrivacyGuard V1 hardware](Images/hardware.jpg)
-->

---

## Why this project

Hidden cameras in hotel rooms, hostels, rentals and changing rooms are a real privacy risk. One detection method alone is not reliable:

- RF detection misses cameras that only record to an SD card.
- Wi-Fi scanning misses cameras that never connect to a network.
- Optical detection can be fooled by mirrors and shiny surfaces.

PrivacyGuard combines all three into one risk score.

---

## PrivacyGuard X roadmap

PrivacyGuard X is the full idea. This repo is **Version 1**.

| Version | What it adds | Status |
|---|---|---|
| V1 | RF detection, Wi-Fi scan, IR optical detection, risk score on OLED | Prototype (this repo) |
| V2 | Mobile app, SOS, live location, detection history | Planned |
| V3 | Cyber alert, one-tap call, report portal | Planned |
| V4 | Edge-AI classification, anti-spoofing, advanced threat analysis | Planned |

V2, V3 and V4 are plans only. They are not built yet.

---

## How it works

The firmware runs one scan when you press the SCAN button. Each sensor gives a score from 0 to 100.

1. **RF detection (weight 0.35).** An AD8317 logarithmic RF detector is read by the ESP32 ADC. The firmware measures a baseline for 1 second, then flags readings more than 10 dB above that baseline.
2. **Wi-Fi scan (weight 0.40).** The ESP32 scans nearby networks in passive mode. It scores networks that match known camera SSID patterns or are unencrypted with a strong signal. Known safe networks can be whitelisted.
3. **Optical detection (weight 0.25).** An 850 nm IR LED is pulsed and a BPW40 phototransistor measures the reflection from a camera lens, over 4 angles.

**Fusion:**

```
RISK = 0.35 x RF + 0.40 x WiFi + 0.25 x Optical

RISK >= 70   -> CAMERA DETECTED (buzzer + vibration)
50 to 69     -> ANOMALY DETECTED (one beep)
below 50     -> No threats detected
```

A small filter lowers the score by 15% when a single high spike appears after quiet scans.

---

## Known limitations

- **Optical-only detection cannot raise an alert yet.** With these weights, a camera seen only by the optical sensor (for example an offline SD-card camera) can reach a risk of at most 25, which is below the alert threshold of 50. Improving the fusion logic is planned for the next version.
- Offline cameras give no RF or Wi-Fi signal, so only the optical sensor can see them, and only at short range.
- Metal enclosures weaken RF signals.
- Mirrors and shiny surfaces can cause false optical readings.
- Crowded Wi-Fi areas can cause false Wi-Fi and RF readings.
- The RF reading is converted to dBm with a fixed formula. It needs to be calibrated against the AD8317 datasheet and a known signal source before the dBm values can be trusted.

Measured test results will be added to `Detection_Testing.md` once they are recorded.

---

## Hardware

| Part | Purpose |
|---|---|
| ESP32-WROOM-32 | Main controller, Wi-Fi scanning, ADC |
| AD8317 | RF power detector (analog output to ADC) |
| 850 nm IR LED + BPW40 phototransistor | Optical retroreflection sensing |
| SSD1306 0.96" OLED (I2C) | Shows scan status and results |
| Buzzer + vibration motor | Alerts |
| Push buttons | SCAN and MENU |
| LiPo battery + charger module | Portable power |

Full parts list: see `BOM.csv`.

### Pin map (from the firmware)

| Signal | ESP32 pin |
|---|---|
| AD8317 output | GPIO34 (ADC1) |
| Phototransistor input | GPIO35 (ADC1) |
| IR LED control | GPIO32 |
| SCAN button | GPIO25 |
| MENU button | GPIO26 |
| Buzzer | GPIO27 |
| Vibration motor | GPIO33 |
| OLED | I2C, address 0x3C |

<!-- After you upload your block diagram, remove these comment marks:
### Block diagram
![Block diagram](Images/block_diagram.png)
-->

---

## Files in this repo

| File | What it does |
|---|---|
| `PrivacyGuard_V1.ino` | Main sketch: setup, button handling, scan sequence, alerts |
| `RF_Detection.h` | AD8317 reading, baseline and anomaly scoring |
| `WiFi_Scanner.h` | Passive Wi-Fi scan and network scoring |
| `Optical_Detection.h` | IR pulse and phototransistor measurement |
| `Risk_Scoring.h` | Weighted fusion and false-positive filter |
| `UI_Handler.h` | OLED screens and alert output |
| `BOM.csv` | Parts list |
| `Assembly_Guide.md` | Assembly steps |
| `Detection_Testing.md` | Test methodology and results |

---

## Build and flash

1. Install the Arduino IDE and the ESP32 board package.
2. Install the libraries **Adafruit SSD1306** and **Adafruit GFX**.
3. Put the `.ino` and all `.h` files in one folder named `PrivacyGuard_V1`, then open `PrivacyGuard_V1.ino`.
4. Select **ESP32 Dev Module** as the board and the correct COM port, then click Upload.
5. Power on and press **SCAN**. Press **MENU** to view the RF, Wi-Fi and optical details of the last scan.

---

## Disclaimer

This device is for personal privacy protection in places you own or have permission to check. Scanning other people's spaces without permission may break local laws.

## License

MIT. See `LICENSE`.

## Author

**Harish A B** | B.E. Electronics Engineering (VLSI Design & Technology), Rajalakshmi Institute of Technology (RIT), Chennai

[GitHub](https://github.com/harish-akash) | [LinkedIn](https://linkedin.com/in/harish-a-b-53557732b)
