/*
  RF_Detection.h - AD8317 Logarithmic Amplifier Interface
  
  Measures 2.4 GHz RF emissions via ADC
  Converts voltage to dBm scale for anomaly detection
*/

#ifndef RF_DETECTION_H
#define RF_DETECTION_H

#include <Arduino.h>
#include <math.h>

class RF_Detection {
private:
  const int ADC_PIN = 34;  // GPIO34 (ADC1_CH6)
  const float AD8317_SENSITIVITY = 0.028;  // V/dB
  const float AD8317_INTERCEPT = 70.0;  // dBm offset
  const float AD8317_MIN_OUTPUT = 0.0;  // Volts
  const float AD8317_MAX_OUTPUT = 1.65;  // Volts (0 dBm)
  
  float rf_baseline = -50.0;  // Default baseline
  int sample_count = 100;
  
public:
  void init() {
    pinMode(ADC_PIN, INPUT);
    Serial.println("[RF] AD8317 initialized on GPIO34");
  }
  
  // Read single RF power level in dBm
  float readRFLevel() {
    int adc_raw = analogRead(ADC_PIN);
    float adc_voltage = (adc_raw / 4095.0) * 3.3;  // Convert to volts
    
    // Convert voltage to dBm using AD8317 calibration
    // Formula: dBm = (Vout / Sensitivity) - Intercept
    float dbm = (adc_voltage / AD8317_SENSITIVITY) - AD8317_INTERCEPT;
    
    // Clamp to valid range
    dbm = constrain(dbm, -70.0, 0.0);
    
    return dbm;
  }
  
  // Measure RF baseline in target room (1 second)
  float measureBaseline(int duration_ms) {
    Serial.println("[RF] Measuring baseline...");
    int num_samples = sample_count;
    float sum = 0;
    float max_val = -100;
    float min_val = 0;
    
    for (int i = 0; i < num_samples; i++) {
      float rf = readRFLevel();
      sum += rf;
      max_val = max(max_val, rf);
      min_val = min(min_val, rf);
      
      delay(duration_ms / num_samples);
    }
    
    rf_baseline = sum / num_samples;
    
    Serial.printf("[RF] Baseline: %.1f dBm (range: %.1f to %.1f)\n", 
                  rf_baseline, min_val, max_val);
    
    return rf_baseline;
  }
  
  // Scan for RF anomalies above baseline
  float scanAnomalies(int duration_ms, float baseline) {
    Serial.println("[RF] Scanning for anomalies...");
    rf_baseline = baseline;
    
    float max_deviation = 0;
    float peak_rf = baseline;
    bool anomaly_detected = false;
    int anomaly_count = 0;
    int stable_count = 0;
    
    int num_samples = sample_count;
    
    for (int i = 0; i < num_samples; i++) {
      float rf = readRFLevel();
      float deviation = rf - baseline;
      
      // Check for sustained deviation (>10 dB for 3+ samples)
      if (deviation > 10.0) {
        anomaly_count++;
        anomaly_detected = true;
        max_deviation = max(max_deviation, deviation);
        peak_rf = max(peak_rf, rf);
        
        Serial.printf("[RF] Anomaly detected: %.1f dBm (%.1f dB above baseline)\n", 
                      rf, deviation);
      } else {
        stable_count++;
      }
      
      delay(duration_ms / num_samples);
    }
    
    // Calculate RF score (0-100)
    float rf_score = 0;
    
    if (anomaly_count >= 3) {  // Sustained anomaly
      rf_score = 50 + (max_deviation * 3);  // Score based on dB deviation
      rf_score = constrain(rf_score, 0, 100);
      
      Serial.printf("[RF] Sustained anomaly confirmed. Score: %.1f/100\n", rf_score);
    } else if (anomaly_count > 0) {
      rf_score = 20 + (max_deviation * 2);  // Brief spike
      Serial.printf("[RF] Brief RF spike detected. Score: %.1f/100\n", rf_score);
    } else {
      Serial.println("[RF] No RF anomalies detected.");
    }
    
    return rf_score;
  }
  
  // Get current baseline
  float getBaseline() {
    return rf_baseline;
  }
  
  // Set custom threshold (for tuning)
  void setThreshold(float threshold_db) {
    rf_baseline = threshold_db;
    Serial.printf("[RF] Threshold set to %.1f dBm\n", threshold_db);
  }
  
  // Diagnostic: print raw ADC values for calibration
  void diagnosticCalibration() {
    Serial.println("\n=== RF DETECTOR CALIBRATION ===");
    for (int i = 0; i < 10; i++) {
      int adc_raw = analogRead(ADC_PIN);
      float adc_voltage = (adc_raw / 4095.0) * 3.3;
      float dbm = (adc_voltage / AD8317_SENSITIVITY) - AD8317_INTERCEPT;
      
      Serial.printf("Sample %d: ADC=%4d, V=%.2f, dBm=%.1f\n", 
                    i+1, adc_raw, adc_voltage, dbm);
      delay(100);
    }
    Serial.println("==============================\n");
  }
};

#endif
