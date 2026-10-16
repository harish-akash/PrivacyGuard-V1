/*
  PrivacyGuard V1 - Portable Hidden Camera Detection System
  Author: Harish Akash
  RAIT, Electronics Engineering (VLSI)
  
  Multi-sensor fusion system:
  - RF Detection (AD8317 + ADC)
  - WiFi Anomaly Detection
  - Optical Retroreflection (IR LED + Phototransistor)
  - Risk Scoring Algorithm
  
  Hardware: ESP32-WROOM-32, AD8317, BPW40, IR LED array, OLED display
  License: MIT
*/

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <math.h>
#include "RF_Detection.h"
#include "WiFi_Scanner.h"
#include "Optical_Detection.h"
#include "Risk_Scoring.h"
#include "UI_Handler.h"

// ============ DISPLAY SETUP ============
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ============ GPIO PINS ============
#define AD8317_INPUT GPIO_NUM_34      // ADC1_CH6 (analog input)
#define IR_LED_CONTROL GPIO_NUM_32    // PWM output
#define PHOTOTRANSISTOR_INPUT GPIO_NUM_35  // ADC1_CH7
#define BUTTON_SCAN GPIO_NUM_25       // Active-low
#define BUTTON_MENU GPIO_NUM_26
#define BUZZER_PIN GPIO_NUM_27        // PWM for tone
#define VIBRATION_MOTOR GPIO_NUM_33   // PWM

// ============ SENSOR OBJECTS ============
RF_Detection rf_detector;
WiFi_Scanner wifi_scanner;
Optical_Detection optical_detector;
Risk_Scoring risk_calculator;
UI_Handler ui;

// ============ STATE VARIABLES ============
struct ScanResult {
  float rf_score;
  float wifi_score;
  float optical_score;
  float risk_total;
  bool alert_triggered;
  String detected_ssid;
  int detected_rssi;
  unsigned long scan_timestamp;
};

ScanResult last_result = {0, 0, 0, 0, false, "", -100, 0};

// ============ INTERRUPT HANDLERS ============
volatile bool scan_requested = false;
volatile bool menu_requested = false;

void IRAM_ATTR buttonScanISR() {
  scan_requested = true;
}

void IRAM_ATTR buttonMenuISR() {
  menu_requested = true;
}

// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== PrivacyGuard V1 Initialization ===");
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERROR: SSD1306 allocation failed");
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("PrivacyGuard V1");
  display.println("Initializing...");
  display.display();
  
  // Initialize GPIO
  pinMode(AD8317_INPUT, INPUT);
  pinMode(PHOTOTRANSISTOR_INPUT, INPUT);
  pinMode(IR_LED_CONTROL, OUTPUT);
  pinMode(BUTTON_SCAN, INPUT_PULLUP);
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_MOTOR, OUTPUT);
  
  digitalWrite(IR_LED_CONTROL, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(VIBRATION_MOTOR, LOW);
  
  // Configure button interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON_SCAN), buttonScanISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_MENU), buttonMenuISR, FALLING);
  
  // ADC configuration
  analogSetWidth(12);  // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db);  // Full range
  
  // Initialize sensor modules
  Serial.println("[RF] Initializing AD8317 detector...");
  rf_detector.init();
  
  Serial.println("[WiFi] Initializing WiFi scanner...");
  wifi_scanner.init();
  
  Serial.println("[Optical] Initializing IR photodetector...");
  optical_detector.init(IR_LED_CONTROL, PHOTOTRANSISTOR_INPUT);
  
  Serial.println("[Risk] Initializing risk calculator...");
  risk_calculator.init();
  
  Serial.println("[UI] Initializing display handler...");
  ui.init(&display, BUZZER_PIN, VIBRATION_MOTOR);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("READY");
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println("Press SCAN to start");
  display.display();
  
  Serial.println("=== Initialization Complete ===\n");
  delay(2000);
}

// ============ MAIN LOOP ============
void loop() {
  // Check button press
  if (scan_requested) {
    scan_requested = false;
    performScan();
    delay(100);  // Debounce
  }
  
  if (menu_requested) {
    menu_requested = false;
    displayMenu();
    delay(100);  // Debounce
  }
  
  // Show idle screen
  displayIdleScreen();
  delay(500);
}

// ============ SCANNING FUNCTION ============
void performScan() {
  Serial.println("\n>>> SCAN INITIATED <<<");
  ui.displayStatus("Scanning...");
  
  // Establish RF baseline (1 second in target room)
  Serial.println("[RF] Establishing baseline...");
  ui.displayStatus("RF baseline...");
  float rf_baseline = rf_detector.measureBaseline(1000);  // 1 second
  Serial.printf("RF Baseline: %.1f dBm\n", rf_baseline);
  
  // ========== RF DETECTION (2 seconds) ==========
  Serial.println("[RF] Scanning for anomalies...");
  ui.displayStatus("Scanning RF...");
  float rf_score = rf_detector.scanAnomalies(2000, rf_baseline);
  Serial.printf("RF Score: %.1f/100\n", rf_score);
  
  // ========== WiFi SCANNING (1 second) ==========
  Serial.println("[WiFi] Scanning networks...");
  ui.displayStatus("Scanning WiFi...");
  float wifi_score = wifi_scanner.scanAndScore();
  String detected_ssid = wifi_scanner.getDetectedSSID();
  int detected_rssi = wifi_scanner.getDetectedRSSI();
  Serial.printf("WiFi Score: %.1f/100\n", wifi_score);
  if (detected_ssid.length() > 0) {
    Serial.printf("Suspicious SSID: %s (RSSI: %d dBm)\n", 
                   detected_ssid.c_str(), detected_rssi);
  }
  
  // ========== OPTICAL DETECTION (1.5 seconds, 4 angles) ==========
  Serial.println("[Optical] Scanning retroreflection...");
  ui.displayStatus("Scanning Optical...");
  float optical_score = optical_detector.scanRetroReflection(1500);
  Serial.printf("Optical Score: %.1f/100\n", optical_score);
  
  // ========== RISK CALCULATION ==========
  Serial.println("[Risk] Calculating total risk...");
  float risk_total = risk_calculator.calculateRisk(rf_score, wifi_score, optical_score);
  Serial.printf("RISK SCORE: %.1f/100\n", risk_total);
  
  // Store results
  last_result.rf_score = rf_score;
  last_result.wifi_score = wifi_score;
  last_result.optical_score = optical_score;
  last_result.risk_total = risk_total;
  last_result.detected_ssid = detected_ssid;
  last_result.detected_rssi = detected_rssi;
  last_result.scan_timestamp = millis();
  
  // ========== DECISION & ALERT ==========
  if (risk_total >= 70) {
    Serial.println(">>> ALERT: POTENTIAL CAMERA DETECTED <<<");
    last_result.alert_triggered = true;
    ui.displayAlert("CAMERA DETECTED!", risk_total);
    activateAlert(3);  // 3 beeps + vibration
  } else if (risk_total >= 50) {
    Serial.println(">>> WARNING: Anomalous detection <<<");
    last_result.alert_triggered = false;
    ui.displayWarning("ANOMALY DETECTED", risk_total);
    activateAlert(1);  // 1 beep
  } else {
    Serial.println(">>> INFO: No threats detected <<<");
    last_result.alert_triggered = false;
    ui.displayResults(&last_result);
  }
  
  Serial.println(">>> SCAN COMPLETE <<<\n");
}

// ============ ALERT ACTIVATION ==========
void activateAlert(int beeps) {
  for (int i = 0; i < beeps; i++) {
    // Buzzer beep (1kHz, 200ms)
    for (int j = 0; j < 200; j++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(500);  // 1kHz frequency
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(500);
    }
    
    // Vibration pulse (100ms)
    analogWrite(VIBRATION_MOTOR, 255);
    delay(100);
    analogWrite(VIBRATION_MOTOR, 0);
    
    delay(200);  // Gap between beeps
  }
}

// ============ MENU DISPLAY ==========
void displayMenu() {
  static int menu_index = 0;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  if (last_result.risk_total == 0) {
    display.setCursor(0, 0);
    display.println("No scan results.");
    display.println("Press SCAN first.");
    display.display();
    delay(2000);
    return;
  }
  
  // Display different screens based on menu_index
  switch (menu_index % 4) {
    case 0:  // Risk summary
      displayRiskSummary();
      break;
    case 1:  // RF details
      displayRFDetails();
      break;
    case 2:  // WiFi details
      displayWiFiDetails();
      break;
    case 3:  // Optical details
      displayOpticalDetails();
      break;
  }
  
  menu_index++;
}

void displayRiskSummary() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("RISK SCORE");
  display.setTextSize(3);
  display.setCursor(30, 20);
  display.printf("%.0f%%", last_result.risk_total);
  
  display.setTextSize(1);
  display.setCursor(0, 50);
  if (last_result.alert_triggered) {
    display.println(">>> CAMERA DETECTED <<<");
  } else if (last_result.risk_total >= 50) {
    display.println(">>> ANOMALY DETECTED <<<");
  } else {
    display.println("No threats detected");
  }
  display.display();
  delay(3000);
}

void displayRFDetails() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RF DETECTION");
  display.println("─────────────");
  display.printf("Score: %.1f/100\n", last_result.rf_score);
  display.println("");
  display.println("WiFi streaming:");
  display.println("Detected: High confidence");
  display.println("");
  display.println("Range: 2-10 meters");
  display.display();
  delay(3000);
}

void displayWiFiDetails() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("WiFi SCANNING");
  display.println("─────────────");
  display.printf("Score: %.1f/100\n", last_result.wifi_score);
  display.println("");
  if (last_result.detected_ssid.length() > 0) {
    display.print("SSID: ");
    display.println(last_result.detected_ssid);
    display.printf("RSSI: %d dBm\n", last_result.detected_rssi);
  } else {
    display.println("No anomalies");
  }
  display.display();
  delay(3000);
}

void displayOpticalDetails() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("OPTICAL DETECTION");
  display.println("─────────────────");
  display.printf("Score: %.1f/100\n", last_result.optical_score);
  display.println("");
  display.println("IR retroreflection:");
  if (last_result.optical_score > 30) {
    display.println("Reflection detected");
  } else {
    display.println("No significant reflection");
  }
  display.println("");
  display.println("Range: 0.5-3 meters");
  display.display();
  delay(3000);
}

// ============ IDLE SCREEN ==========
void displayIdleScreen() {
  static unsigned long last_update = 0;
  static int animation_frame = 0;
  
  if (millis() - last_update < 500) return;  // Update every 500ms
  last_update = millis();
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("PrivacyGuard");
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Status: Ready");
  
  // Last scan info
  if (last_result.risk_total > 0) {
    display.printf("Last: %.0f%% risk\n", last_result.risk_total);
  } else {
    display.println("No scans yet");
  }
  
  // Animation
  display.setCursor(0, 40);
  String spinner[] = {"|", "/", "─", "\\"};
  display.print("Waiting ");
  display.println(spinner[animation_frame % 4]);
  
  display.setCursor(0, 55);
  display.println("Press SCAN button");
  
  display.display();
  animation_frame++;
}

// ============ UTILITY FUNCTIONS ==========
void logScanData() {
  Serial.println("\n=== SCAN LOG ===");
  Serial.printf("Timestamp: %lu ms\n", last_result.scan_timestamp);
  Serial.printf("RF Score: %.2f\n", last_result.rf_score);
  Serial.printf("WiFi Score: %.2f\n", last_result.wifi_score);
  Serial.printf("Optical Score: %.2f\n", last_result.optical_score);
  Serial.printf("Total Risk: %.2f%%\n", last_result.risk_total);
  Serial.printf("Alert: %s\n", last_result.alert_triggered ? "YES" : "NO");
  Serial.println("================\n");
}
