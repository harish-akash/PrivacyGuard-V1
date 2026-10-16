# PrivacyGuard V1 - Hardware Assembly Guide

---

## **1. TOOLS REQUIRED**

### **Soldering Equipment**
- Soldering iron (25-40W)
- Solder (lead-free recommended)
- Solder wick / desoldering pump
- Flux pen
- Wet sponge or brass wire cleaner

### **Hand Tools**
- Multimeter (continuity testing)
- Wire stripper / cutter
- Tweezers (component placement)
- Magnifying glass (PCB inspection)
- Screwdriver set

### **Test Equipment (Optional)**
- Oscilloscope (troubleshooting)
- RF detector (AD8317 calibration)
- Thermal camera (power supply testing)

---

## **2. COMPONENT PREPARATION**

### **2.1 Inventory Check**

Before assembly, verify all components from BOM.csv:

```
□ ESP32-WROOM-32 DevKit
□ AD8317 RF detector IC (SOIC-8)
□ BPW40 phototransistor
□ MCP6001 opamp (SOIC-8)
□ 0.96" OLED display (SSD1306)
□ 16× IR LED ring (WS2812B or generic)
□ Resistors (10Ω, 1kΩ, 10kΩ, 100kΩ, 1MΩ)
□ Capacitors (100nF, 1µF, 10µF ceramic & electrolytic)
□ Inductors & ferrite beads
□ Push buttons (4×)
□ Buzzer (5V)
□ Vibration motor (3.7V)
□ LiPo battery (1S, 2000mAh)
□ TP4056 charger module
□ SX1308 boost converter
□ PCB (2-layer)
□ Connectors (USB micro, JST-PH)
```

### **2.2 Sorting Components**

Organize by function:
- **Digital ICs**: AD8317, MCP6001 (keep in static bags)
- **Passives**: Resistors, capacitors (sort by value)
- **Power components**: Battery, charger, boost converter
- **Sensors**: BPW40, OLED, buttons
- **Connectors**: USB, JST

---

## **3. PCB ASSEMBLY**

### **3.1 Soldering Surface-Mount Components**

**Order of assembly** (smallest to largest):

#### **Step 1: AD8317 Logarithmic Amplifier (SOIC-8)**

1. Apply thin layer of solder paste to pads
2. Place AD8317 IC on pads using tweezers
3. Heat with soldering iron (2-3 seconds per pad)
4. Let cool; check for cold joints

**Orientation**: Check datasheet—pin 1 is marked with dot/line

**Test**: Use multimeter continuity to verify no shorts between pins

#### **Step 2: MCP6001 Opamp (SOIC-8)**

Same procedure as AD8317.

**Connections**:
- Pin 8: V+ (3.3V)
- Pin 4: GND
- Pin 1: non-inverting input (from BPW40)
- Pin 2: inverting input (feedback from pin 6)
- Pin 6: output (to ADC GPIO35)

#### **Step 3: Resistor/Capacitor Network**

Solder in order of size:
1. Small resistors (0603/0805)
2. Film capacitors (0603/0805)
3. Electrolytic capacitors (1206 or larger)
4. Inductors and ferrite beads

**Key points**:
- RF trace resistor (50Ω) near antenna: match impedance
- AD8317 bypass cap (100nF): place directly on pins
- Power supply caps: one per IC

#### **Step 4: BPW40 Phototransistor**

Through-hole component:
1. Bend leads to 90° angles
2. Insert through holes (check polarity)
3. Solder from back side
4. Trim leads flush

**Orientation**: Long leg (+) → pin marked on PCB

#### **Step 5: Connectors & Headers**

- **Antenna connector**: SMA or PCB trace
- **JST-PH (battery)**: Both pads soldered
- **USB micro (charging)**: All 5 pins
- **Header pins (sensors)**: Solder for modularity

### **3.2 Soldering Through-Hole Components**

**Push Buttons (4×)**:
1. Insert from top side
2. Solder both leads from back
3. Verify tactile response (push-click-release)

**Buzzer & Vibration Motor**:
1. Solder polarity-marked pins
2. Protect with heatshrink tubing

**IC Sockets** (optional, for replaceable ICs):
1. Solder 8-pin DIP socket for AD8317/MCP6001
2. Place ICs carefully with orientation marker

---

## **4. POWER SUPPLY WIRING**

### **4.1 LiPo Battery Connection**

```
LiPo (+red)  ─→  TP4056 input (BAT+)
LiPo (-black) ─→ TP4056 input (BAT-)
TP4056 OUT+ ─→  PCB VCC (3.3V)
TP4056 OUT- ─→  PCB GND
```

**CRITICAL**: Check polarity before connecting. Reverse polarity can damage ESP32.

### **4.2 USB Charging Setup**

TP4056 module includes:
- Micro-USB input for charging
- Integrated charge protection
- Charging LED indicator

**Wiring**:
- ESP32 3.3V: From TP4056 output (BAT+ side)
- All GND: Common rail to TP4056 BAT-

### **4.3 Boost Converter (3.7V → 5V)**

Powers IR LED ring (WS2812B requires 5V):

```
LiPo (3.7V) ─→ SX1308 IN+
GND         ─→ SX1308 IN-
SX1308 OUT+ ─→ WS2812B +5V
SX1308 OUT- ─→ WS2812B GND
```

**Adjustment**: SX1308 potentiometer tuned for 5.0V output (measure with multimeter)

---

## **5. ESP32 DEVKIT CONNECTIONS**

### **5.1 GPIO Mapping**

| Function | ESP32 GPIO | PCB Net |
|---|---|---|
| AD8317 Input | GPIO34 (ADC1_CH6) | AD8317_OUT |
| Phototransistor | GPIO35 (ADC1_CH7) | OPAMP_OUT |
| IR LED Control | GPIO32 | IR_LED_EN (PWM) |
| Button SCAN | GPIO25 | BTN_SCAN |
| Button MENU | GPIO26 | BTN_MENU |
| Buzzer | GPIO27 | BUZZER (PWM) |
| Vibration | GPIO33 | VIBE_PWM |
| I2C SDA (OLED/MLX) | GPIO21 | I2C_SDA |
| I2C SCL (OLED/MLX) | GPIO22 | I2C_SCL |
| GND | GND | GND (star point) |
| 3.3V | 3V3 | VCC_3V3 |

### **5.2 Soldering ESP32 to PCB**

**Option A: Header Pins (Recommended for prototyping)**
1. Insert male header pins into ESP32 holes
2. Solder onto PCB via female sockets
3. Advantage: Easy replacement/debugging

**Option B: Direct Soldering (Compact)**
1. Solder ESP32 directly onto PCB pads
2. Requires careful soldering (many pins)
3. Advantage: Smaller form factor

---

## **6. SENSOR INTEGRATION**

### **6.1 OLED Display (SSD1306)**

```
SSD1306      ← ESP32
GND          → GND
VCC          → 3.3V
SDA (D4)     → GPIO21
SCL (D5)     → GPIO22
```

Connection via I2C:
- 10kΩ pull-up resistors on SDA/SCL (built-in on most modules)

### **6.2 RF Antenna**

**Dipole antenna** (recommended for 2.4 GHz):
- Length: ~30 mm (quarter-wave = 31.25 mm at 2.4 GHz)
- Material: 24 AWG copper wire
- Connection: 50Ω matching network → AD8317 input

**PCB trace antenna** (alternative):
- Microstrip monopole: ~25 mm length
- Ground plane clearance: 5 mm minimum

### **6.3 IR LED Array (WS2812B Ring)**

Addressable LED ring (optional PWM control):

```
WS2812B
+5V  ────→ SX1308 OUT+ (boost converter)
GND  ────→ PCB GND
DIN  ────→ GPIO32 (PWM signal)
```

**Simple control** (no addressable features):
- Single GPIO output toggles all LEDs on/off
- Firmware: `digitalWrite(IR_LED_CONTROL, HIGH)` for pulse

---

## **7. MECHANICAL ASSEMBLY**

### **7.1 Enclosure Selection**

**Option A: 3D-printed enclosure** (custom)
- Download STL from [thingiverse/privacy-detector](https://thingiverse.com)
- Print in PLA/PETG (print time: 4-6 hours)
- Assemble with M3 screws and spacers

**Option B: Plastic enclosure** (off-the-shelf)
- Hammond 1554 or similar (60×40×25 mm)
- Drill holes for buttons, USB port, display window

**Option C: Handheld chassis**
- PCB mounted to aluminum frame
- Bare-PCB with protective coating (conformal coat)
- Cable organizer for power/antenna

### **7.2 Layout Inside Enclosure**

```
┌─────────────────────────────────────────┐
│          Front Face                     │
│  ┌─────────────────────────────────┐   │
│  │   OLED 0.96" Display (128×64)   │   │
│  │                                 │   │
│  └─────────────────────────────────┘   │
│                                         │
│  [SCAN] [MENU]  [UP] [DOWN] (buttons)  │
│                                         │
└─────────────────────────────────────────┘

┌─────────────────────────────────────────┐
│          Back Face                      │
│                                         │
│  [USB Micro] [Antenna Connector]        │
│                                         │
│  LED Status Indicator (charging)        │
│                                         │
└─────────────────────────────────────────┘

Side: IR LED array ring (exposed or translucent cover)
```

### **7.3 Cable Management**

- **Antenna**: SMA connector or PCB trace (no cable needed)
- **USB charging**: Strain relief grommet
- **Battery**: JST connector (secure with velcro/tape inside)
- **Buttons**: Soldered directly to PCB

---

## **8. POST-ASSEMBLY TESTING**

### **8.1 Power-Up Checklist**

**BEFORE connecting battery**:
- [ ] Visually inspect for solder bridges
- [ ] Multimeter continuity: 3.3V ↔ GND (should NOT be shorted)
- [ ] Check component polarity (batteries, capacitors, ICs)
- [ ] Verify no loose leads or cold joints

**POWER-UP TEST**:
1. Connect LiPo battery via TP4056 (safe: has protection)
2. Check LED indicator on TP4056 (should blink if charging)
3. Measure output voltage: 3.3V (±0.1V)
4. Measure boost converter output: 5.0V (±0.2V)

**FAILURE MODE**: If no power, check:
- Polarity reversal
- TP4056 protection circuit triggered (red LED)
- Solder bridge between VCC/GND

### **8.2 Firmware Upload**

1. **Install Arduino IDE**:
   - Download: [arduino.cc](https://arduino.cc/downloads)
   - Install ESP32 board package

2. **Compile & Upload**:
   ```bash
   # Arduino IDE
   - Board: ESP32 Dev Module
   - Upload Speed: 921600 baud
   - Port: COM3 (Windows) or /dev/ttyUSB0 (Linux)
   - Hit Upload
   ```

3. **Serial Monitor Test**:
   - Open Tools → Serial Monitor (115200 baud)
   - Should see boot message: "=== PrivacyGuard V1 Initialization ==="

### **8.3 Sensor Verification**

**OLED Display**:
- Should show "PrivacyGuard V1" on boot
- Test I2C: Check address 0x3C with I2C scanner

**Buttons**:
- Press SCAN: Should trigger scan
- Press MENU: Should show menu

**RF Detector** (AD8317):
- Normal baseline: ADC 800-1200 counts (−50 to −45 dBm in quiet room)
- Place WiFi router near PCB: ADC should increase to 2000+ counts

**IR LEDs**:
- GPIO32 = HIGH: LEDs light up (visible with IR camera)
- Phototransistor ADC should increase significantly

**Buzzer & Vibration**:
- Manual test code:
  ```cpp
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  ```

---

## **9. TROUBLESHOOTING**

| Problem | Cause | Solution |
|---|---|---|
| No power-up | Battery reversed | Check LiPo polarity |
| Weak WiFi detection | Poor antenna connection | Verify 50Ω matching; reflow solder |
| OLED blank | I2C address wrong | Scan with I2C debugger (0x3C) |
| AD8317 gives 0 or 4095 | ADC saturation or open | Check 100nF bypass cap; verify connections |
| Phototransistor no response | Opamp not powered | Check 3.3V on pin 8 of MCP6001 |
| Button unresponsive | GPIO pin not connected | Continuity test with multimeter |
| Battery drains fast | Boost converter inefficiency | Tune SX1308 potentiometer; check continuous load |

---

## **10. CALIBRATION**

### **10.1 RF Detector Calibration**

1. Place in RF-quiet environment (Faraday cage or outdoor)
2. Measure ADC value: should be ~0 counts (no signal)
3. Adjust AD8317 DC offset (feedback resistor) if needed

### **10.2 Phototransistor Gain Adjustment**

1. Measure ambient light ADC: ~500–1000 counts
2. Shine IR LED at 1cm: ADC should jump to 2000–3000 counts
3. If too low: increase feedback resistor (MCP6001) to 2MΩ
4. If too high: decrease to 500kΩ

### **10.3 WiFi Scanning Frequency**

Adjust in code:
```cpp
int num_networks = WiFi.scanNetworks();  // Scan all 14 channels (~1 sec)
```

---

## **11. FINAL CHECKLIST**

- [ ] All components soldered (visual inspection)
- [ ] No cold joints or solder bridges
- [ ] Power supply: 3.3V and 5.0V verified
- [ ] Firmware uploaded successfully
- [ ] All buttons functional
- [ ] OLED display shows boot message
- [ ] RF detector baseline measured
- [ ] WiFi scanner working (detects nearby networks)
- [ ] Optical LEDs light up
- [ ] Buzzer produces tone
- [ ] Vibration motor spins
- [ ] Enclosure assembled
- [ ] Cable management organized
- [ ] Battery charging verified (TP4056 LED blinks)

---

## **NEXT STEPS**

1. **Field Testing**: Run detection scans in your target environment
2. **Calibration**: Measure RF baseline in different rooms
3. **Tuning**: Adjust thresholds based on false-positive feedback
4. **Documentation**: Document any hardware modifications
5. **Optimization**: Optimize power consumption for extended battery life

---

**Assembly Complete!** ✓

Your PrivacyGuard V1 prototype is ready for deployment. For support, check the troubleshooting guide or refer to sensor datasheets.

**Estimated Assembly Time**: 2–4 hours (including testing)
