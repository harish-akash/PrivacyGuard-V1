/*
  UI_Handler.h - OLED Display & Button Interface Management
  
  Manages user interface, result display, and alert feedback
*/

#ifndef UI_HANDLER_H
#define UI_HANDLER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// Forward declaration of ScanResult (defined in main sketch)
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

class UI_Handler {
private:
  Adafruit_SSD1306* display;
  int buzzer_pin = 27;
  int vibration_pin = 33;
  
public:
  void init(Adafruit_SSD1306* d, int buzzer, int vibration) {
    display = d;
    buzzer_pin = buzzer;
    vibration_pin = vibration;
    
    Serial.println("[UI] Display and feedback initialized");
  }
  
  void displayStatus(String status) {
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 20);
    display->println(status);
    
    display->setTextSize(1);
    display->setCursor(0, 50);
    display->println("Processing...");
    display->display();
  }
  
  void displayAlert(String message, float risk) {
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("!!! ALERT !!!");
    
    display->setTextSize(1);
    display->setCursor(0, 25);
    display->println(message);
    
    display->setTextSize(3);
    display->setCursor(20, 38);
    display->printf("%.0f%%", risk);
    
    display->display();
  }
  
  void displayWarning(String message, float risk) {
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("WARNING");
    
    display->setTextSize(1);
    display->setCursor(0, 25);
    display->println(message);
    
    display->setTextSize(2);
    display->setCursor(30, 40);
    display->printf("%.0f%%", risk);
    
    display->display();
  }
  
  void displayResults(ScanResult* result) {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("SCAN RESULTS");
    display->println("═══════════════════");
    
    display->printf("Risk:    %.0f%%\n", result->risk_total);
    display->printf("RF:      %.0f%%\n", result->rf_score);
    display->printf("WiFi:    %.0f%%\n", result->wifi_score);
    display->printf("Optical: %.0f%%\n", result->optical_score);
    
    display->println("═══════════════════");
    display->println("Press MENU for details");
    
    display->display();
  }
  
  void displayRiskBar(float risk) {
    // Visual bar graph for risk (0-128 pixels wide)
    int bar_width = (int)(risk / 100.0 * 100);
    bar_width = constrain(bar_width, 0, 100);
    
    display->fillRect(0, 50, bar_width, 10, SSD1306_WHITE);
    display->drawRect(0, 50, 100, 10, SSD1306_WHITE);
    
    display->setTextSize(1);
    display->setCursor(105, 50);
    display->printf("%.0f%%", risk);
  }
  
  void activateAudio(int count) {
    for (int i = 0; i < count; i++) {
      // 1kHz beep for 200ms
      for (int j = 0; j < 200; j++) {
        digitalWrite(buzzer_pin, HIGH);
        delayMicroseconds(500);
        digitalWrite(buzzer_pin, LOW);
        delayMicroseconds(500);
      }
      delay(200);  // Gap
    }
  }
  
  void activateVibration(int duration_ms) {
    analogWrite(vibration_pin, 255);
    delay(duration_ms);
    analogWrite(vibration_pin, 0);
  }
  
  void playAlertSequence() {
    // Alert pattern: 3 beeps + vibration
    for (int i = 0; i < 3; i++) {
      activateAudio(1);
      activateVibration(100);
    }
  }
};

#endif
