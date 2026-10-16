# PrivacyGuard V1: Portable Hidden Camera Detection System

![Status](https://img.shields.io/badge/Status-Active_Development-brightgreen)
![License](https://img.shields.io/badge/License-MIT-blue)
![Hardware](https://img.shields.io/badge/Hardware-ESP32-orange)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)

---

## **Overview**

PrivacyGuard V1 is a multi-sensor embedded system designed to detect covert/hidden cameras in enclosed spaces (hotel rooms, rental properties, public facilities). It uses RF detection, WiFi anomaly analysis, and optical retroreflection to identify surveillance devices with minimal false positives.

**Key Stats**:
- **Detection Accuracy**: ~85% (streaming cameras)
- **False Positive Rate**: <15% (office environments)
- **Detection Range**: 0.5–10 meters
- **Battery Life**: 24+ hours continuous operation
- **Cost**: ₹1,200–₹1,500 (~$15–$20 per unit, 10-unit batch)

---

## **Technical Specifications**

| Specification | Value | Details |
|---|---|---|
| **Microcontroller** | ESP32-WROOM-32 | Dual-core, 240 MHz, WiFi + BLE |
| **RF Detector** | AD8317 Log-Amp | 0.1–2.5 GHz, −70 to 0 dBm range |
| **WiFi Scanning** | Built-in (ESP32) | 2.4 GHz passive scanning, SSID + MAC logging |
| **Optical Detection** | IR LED (850nm) + BPW40 | Retroreflection analysis via transimpedance amp |
| **Thermal (Optional)** | MLX90614 | Single-point IR thermometer, I2C |
| **Display** | 0.96" OLED (SSD1306) | 128×64 px, I2C interface |
| **Power** | 1S LiPo (2000–3000 mAh) | TP4056 charger, 3.3V regulation |
| **Scanning Loop Time** | ~5 seconds | RF (3s) + WiFi (1s) + Optical (1s) |

---

## **How It Works**

### **1. RF Detection (35% weight)**
- AD8317 measures 2.4 GHz WiFi/Bluetooth emissions
- Establishes 1-second RF baseline in target room
- Flags anomalies >10 dB above baseline
- **Advantage**: Detects streaming cameras easily
- **Limitation**: Offline SD-card cameras produce no RF

### **2. WiFi Anomaly Detection (40% weight)**
- Passive WiFi scanning identifies all networks
- Pattern matching: detects known camera SSIDs
- Flags unencrypted networks with strong signal
- **Advantage**: Lowest false-positive rate
- **Limitation**: Cameras can use generic SSID ("Guest WiFi")

### **3. Optical Retroreflection (25% weight)**
- IR LED pulse + phototransistor measurement
- Detects specular reflection from camera lens
- Scans 4 angles (0°, 90°, 180°, 270°)
- **Advantage**: Catches offline passive cameras
- **Limitation**: Mirrors/reflective surfaces cause false positives

### **4. Risk Scoring & Fusion**
```
RISK_SCORE = 0.35 × RF_score + 0.40 × WiFi_score + 0.25 × Optical_score

Alert Thresholds:
  ≥70  → "POTENTIAL CAMERA DETECTED" (high confidence)
  50–69 → "ANOMALOUS DETECTION" (manual inspection recommended)
  <50  → "No threats detected"
```

---

## **Hardware Architecture**

```
┌─────────────────────────────────────────────────┐
│          USER INTERFACE (OLED + Buttons)        │
│     Display: Risk score, WiFi SSID, Alert      │
└─────────────────────┬───────────────────────────┘
                      │
         ┌────────────▼──────────────┐
         │    ESP32-WROOM-32        │
         │  (Sensor Fusion & Logic) │
         └────────┬────────┬────────┘
                  │        │
    ┌─────────────┼────────┼─────────────┐
    │             │        │             │
    ▼             ▼        ▼             ▼
┌─────────┐ ┌─────────┐ ┌───────┐  ┌──────────┐
│ AD8317  │ │ WiFi    │ │ IR    │  │ MLX90614 │
│ RF Det  │ │ Scanner │ │ Optics│  │ Thermal  │
│ (ADC)   │ │ (Built) │ │(ADC)  │  │ (I2C)    │
└─────────┘ └─────────┘ └───────┘  └──────────┘
     │           │          │          │
     └───────────┴──────────┴──────────┘
              │
     ┌────────▼────────┐
     │ Risk Scoring &  │
     │ Decision Logic  │
     └────────┬────────┘
              │
         ┌────▼─────┐
         │ ALERT    │
         │ (Buzzer+ │
         │ Vibration│
         └──────────┘
```

---

## **Bill of Materials (BOM)**

| Component | Part Number | Qty | Cost (₹) | Cost (USD) | Source |
|---|---|---|---|---|---|
| ESP32-WROOM-32 DevKit | ESP32-DEVKIT | 1 | 400–500 | $5–$6 | Aliexpress |
| AD8317 RF Detector IC | AD8317 | 1 | 150–200 | $2–$3 | Aliexpress/Ebay |
| Antenna (Dipole/Monopole) | Custom/Buy | 1 | 50–100 | $1–$2 | DIY or Aliexpress |
| IR LED Ring (16×, 850nm) | WS2812B or generic | 1 | 200–300 | $3–$5 | Aliexpress |
| Phototransistor (BPW40) | BPW40 | 1 | 80–150 | $1–$2 | Aliexpress/Ebay |
| Transimpedance Opamp | MCP6001 | 1 | 30–50 | $0.5–$1 | Local/Aliexpress |
| OLED Display (0.96") | SSD1306 128×64 | 1 | 150–200 | $2–$3 | Aliexpress |
| LiPo Battery (2000mAh, 1S) | 3.7V JST-PH | 1 | 250–350 | $4–$5 | Aliexpress |
| LiPo Charger Module | TP4056 | 1 | 50–80 | $1–$1.5 | Aliexpress |
| Boost Converter (3.7V→5V) | SX1308 | 1 | 50–100 | $1–$2 | Aliexpress |
| Passive Components (R/C/L) | 1206/0805 assorted | ~30 | 100–150 | $1–$2 | Local stock |
| Push Buttons (4) | 6×6mm tactile | 4 | 20–40 | $0.5–$1 | Local |
| Buzzer (5V) | Passive piezo | 1 | 30–50 | $0.5–$1 | Local |
| Vibration Motor (3.7V) | 10mm coin motor | 1 | 80–120 | $1–$2 | Aliexpress |
| PCB (2-layer, 60×40mm) | JLCPCB/PCBWay | 10 | 200–400 | $3–$5 | JLCPCB |
| Enclosure (3D Print) | PLA/PETG | 1 | 150–300 | $2–$4 | DIY/Local service |
| **TOTAL (Single Unit)** | — | — | **₹2,400–₹3,300** | **$30–$42** | — |
| **TOTAL (10-unit batch)** | — | 10 | **₹12,000–₹15,000** | **$15–$20/unit** | — |

**Bulk supplier links**:
- **Aliexpress**: Direct vendor (search part numbers)
- **JLCPCB**: PCB fabrication + assembly
- **Local electronics shops**: Passive components, connectors (faster delivery)

---

## **Quick Start**

### **Prerequisites**
- Arduino IDE 1.8.x+ OR PlatformIO
- ESP32 board package installed
- USB cable (micro-B) for flashing

### **Setup**

1. **Clone repo**:
   ```bash
   git clone https://github.com/harish-akash/PrivacyGuard-V1.git
   cd PrivacyGuard-V1
   ```

2. **Install dependencies**:
   ```bash
   # Arduino IDE: Sketch → Include Library → Manage Libraries
   # Install: Adafruit SSD1306, Adafruit MLX90614
   
   # OR PlatformIO:
   pio lib install "Adafruit SSD1306" "Adafruit MLX90614"
   ```

3. **Flash firmware**:
   ```bash
   # Arduino IDE:
   # 1. Tools → Board → ESP32 Dev Module
   # 2. Tools → Port → COM3 (or /dev/ttyUSB0)
   # 3. Upload
   
   # OR PlatformIO:
   pio run -t upload
   ```

4. **Power on**:
   - LiPo battery connected to JST-PH connector
   - OLED displays "PrivacyGuard V1"
   - Press **SCAN** button to start detection

---

## **Usage**

### **Button Interface**
| Button | Function |
|---|---|
| **SCAN** | Initiate 5-second detection cycle |
| **MENU** | Cycle through results (RF, WiFi, Optical) |
| **UP/DOWN** | Adjust sensitivity threshold |

### **Display Output**
```
┌──────────────────┐
│ PrivacyGuard V1  │
│ Risk: 72% ⚠️    │
│ RF: 45dB WiFi: ✓ │
│ Cam detected!    │
└──────────────────┘
```

**Alert**: Buzzer + vibration when Risk ≥ 70%

---

## **Project Structure**

```
PrivacyGuard-V1/
├── README.md (this file)
├── LICENSE (MIT)
├── .gitignore
│
├── Firmware/
│   ├── PrivacyGuard_V1.ino (main sketch)
│   ├── RF_Detection.h (AD8317 sensor logic)
│   ├── WiFi_Scanner.h (WiFi anomaly detection)
│   ├── Optical_Detection.h (IR retroreflection)
│   ├── Risk_Scoring.h (multi-sensor fusion)
│   ├── UI_Handler.h (OLED display & buttons)
│   └── platformio.ini (build config)
│
├── Hardware/
│   ├── Schematic.pdf (KiCad exported)
│   ├── PCB_Layout.pdf (2-layer design)
│   ├── Gerber_Files/ (production-ready)
│   │   ├── *.gbr (layer files)
│   │   ├── *.gbl
│   │   └── *.txt (drill file)
│   ├── 3D_Enclosure.STL (for 3D printing)
│   └── Component_Placement.pdf
│
├── Documentation/
│   ├── Technical_Analysis.md (RF/WiFi/Optical theory)
│   ├── Detection_Testing.md (methodology + results)
│   ├── Troubleshooting.md (common issues & fixes)
│   └── Future_Improvements.md (V2 roadmap)
│
├── BOM/
│   ├── BOM.csv (parts list with links)
│   └── Assembly_Guide.md (step-by-step)
│
└── Images/
    ├── PCB_Top.jpg
    ├── PCB_Bottom.jpg
    ├── Prototype_Photo.jpg
    ├── Detection_Results.png
    └── Schematic_Diagram.png
```

---

## **Test Results**

### **Detection Accuracy (10 test runs)**
| Camera Type | Detected | Accuracy | Notes |
|---|---|---|---|
| WiFi streaming camera (720p) | 9/10 | **90%** | RF + WiFi signature |
| Pinhole camera (offline, SD card) | 7/10 | **70%** | Optical detection only |
| Embedded object camera (desk clock) | 8/10 | **80%** | WiFi setup phase |
| Smartphone (reference) | 0/10 | **0%** | Correctly ignored |

### **False Positive Rate**
- **Test environment**: Office with 5 WiFi networks, 3 mirrors, 2 monitors
- **100 scans performed**: 13 false positives
- **FP rate**: **13%** (within 15% target)
- **Causes**: Mirror reflections (8), overlapping WiFi (5)

### **Detection Range**
- **2.4 GHz WiFi camera**: 2–10 meters
- **Optical retroreflection**: 0.5–3 meters (depends on lens size)
- **Offline SD-card camera**: <0.5 meters (optical only)

---

## **Limitations & Known Issues**

1. **Offline SD-card cameras**: No RF/WiFi → optical detection only (limited range)
2. **Shielded cameras**: Metal enclosure attenuates RF by 20–40 dB
3. **Dense RF environments**: High false positive rate in WiFi-heavy areas
4. **Mirror/reflector confusion**: Optical sensor may flag shiny objects
5. **Pinhole lens detection**: Very small lenses (< 0.5mm) produce weak retroreflection

**Mitigation strategies**: See [Troubleshooting.md](Documentation/Troubleshooting.md)

---

## **Future Enhancements (V2 Roadmap)**

- [ ] Frequency-specific RF detection (differentiate WiFi from Bluetooth)
- [ ] Directional antenna for locating camera position
- [ ] Harmonic analysis for microcontroller clock emissions
- [ ] Machine learning classifier (train on real camera RF signatures)
- [ ] Mobile app integration (WiFi to phone via BLE)
- [ ] Battery indicator with % remaining on OLED
- [ ] Cloud logging of detections (anonymized, privacy-preserving)

---

## **Build & Test Guide**

See **[Hardware Assembly Guide](BOM/Assembly_Guide.md)** for step-by-step PCB assembly and firmware flashing.

Test methodology documented in **[Detection Testing Results](Documentation/Detection_Testing.md)**.

---

## **References & Resources**

- **AD8317 Datasheet**: [Analog Devices](https://www.analog.com/en/products/ad8317.html)
- **ESP32 Documentation**: [Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- **802.11 WiFi Standards**: [IEEE 802.11](https://en.wikipedia.org/wiki/IEEE_802.11)
- **RF Detection Theory**: [ARRL RF Safety](https://www.arrl.org/rf-safety)
- **KiCad Tutorial**: [KiCad Documentation](https://kicad.org/help/documentation/)

---

## **Contributing**

Found a bug? Have an improvement? Submit a pull request or open an issue.

**Development process**:
1. Fork the repo
2. Create feature branch: `git checkout -b feature/YourFeature`
3. Commit changes: `git commit -m "Add XYZ"`
4. Push: `git push origin feature/YourFeature`
5. Submit pull request

---

## **License**

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) file for details.

**Disclaimer**: This device is intended for personal privacy protection in locations you own or have explicit permission to scan. Unauthorized surveillance detection in others' spaces may violate local laws. Use responsibly.

---

## **Author**

**Harish Akash** | B.E. Electronics Engineering (VLSI) | Rajalakshmi Institute of Technology

**Contact**: [GitHub](https://github.com/harish-akash) | [LinkedIn](#)

---

## **Acknowledgments**

- Rajalakshmi Institute of Technology (RAIT) for lab facilities
- Open-source community (Arduino, Espressif, KiCad)
- Privacy advocates and security researchers who inspired this project

---

**Last Updated**: June 2026 | **Status**: Active Development
