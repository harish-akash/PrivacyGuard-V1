/*
  Risk_Scoring.h - Multi-Sensor Fusion & Decision Logic
  
  Combines RF, WiFi, and Optical scores into weighted risk assessment
  Implements threshold-based alerting with false-positive reduction
*/

#ifndef RISK_SCORING_H
#define RISK_SCORING_H

#include <Arduino.h>

class Risk_Scoring {
private:
  // Sensor weights (must sum to 1.0)
  const float WEIGHT_RF = 0.35;      // RF detection: 35%
  const float WEIGHT_WiFi = 0.40;    // WiFi scanning: 40%
  const float WEIGHT_OPTICAL = 0.25; // Optical detection: 25%
  
  // Alert thresholds
  const float THRESHOLD_HIGH = 70.0;    // High confidence alert
  const float THRESHOLD_MEDIUM = 50.0;  // Anomaly warning
  const float THRESHOLD_LOW = 30.0;     // Low confidence flag
  
  // Historical data for false-positive filtering
  const int HISTORY_SIZE = 10;
  float risk_history[10] = {0};
  int history_index = 0;
  
public:
  void init() {
    Serial.println("[Risk] Risk scoring engine initialized");
    Serial.printf("[Risk] Weights - RF: %.0f%%, WiFi: %.0f%%, Optical: %.0f%%\n",
                  WEIGHT_RF*100, WEIGHT_WiFi*100, WEIGHT_OPTICAL*100);
  }
  
  // Main scoring function
  float calculateRisk(float rf_score, float wifi_score, float optical_score) {
    Serial.println("[Risk] Calculating composite risk score...");
    
    // Weighted combination
    float risk = (WEIGHT_RF * rf_score) + 
                 (WEIGHT_WiFi * wifi_score) + 
                 (WEIGHT_OPTICAL * optical_score);
    
    Serial.printf("[Risk] Component breakdown:\n");
    Serial.printf("       RF:      %.1f × %.2f = %.1f\n", rf_score, WEIGHT_RF, rf_score * WEIGHT_RF);
    Serial.printf("       WiFi:    %.1f × %.2f = %.1f\n", wifi_score, WEIGHT_WiFi, wifi_score * WEIGHT_WiFi);
    Serial.printf("       Optical: %.1f × %.2f = %.1f\n", optical_score, WEIGHT_OPTICAL, optical_score * WEIGHT_OPTICAL);
    Serial.printf("[Risk] Composite Score: %.1f/100\n", risk);
    
    // Store in history for trend analysis
    storeHistory(risk);
    
    // Apply false-positive reduction filter
    float filtered_risk = applyFPReduction(risk);
    
    if (abs(filtered_risk - risk) > 5) {
      Serial.printf("[Risk] FP-filtered score: %.1f/100\n", filtered_risk);
    }
    
    return filtered_risk;
  }
  
  // False-positive reduction: look for consistent anomalies
  float applyFPReduction(float current_risk) {
    // Single high spike in otherwise normal history = likely FP
    float average_history = getHistoryAverage();
    
    // If current is anomalously high vs history, reduce confidence
    if (current_risk > 60 && average_history < 30) {
      // Single spike - reduce by 15%
      Serial.println("[Risk-FP] Single spike detected - reducing confidence");
      return current_risk * 0.85;
    }
    
    // Consistent elevation = likely real threat
    if (current_risk > 60 && average_history > 40) {
      Serial.println("[Risk-FP] Consistent elevation - high confidence");
      return current_risk;  // No reduction
    }
    
    return current_risk;
  }
  
  // Store risk score in circular history
  void storeHistory(float risk) {
    risk_history[history_index] = risk;
    history_index = (history_index + 1) % HISTORY_SIZE;
  }
  
  // Get average of historical scores
  float getHistoryAverage() {
    float sum = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
      sum += risk_history[i];
    }
    return sum / HISTORY_SIZE;
  }
  
  // Get alert status
  String getAlertStatus(float risk) {
    if (risk >= THRESHOLD_HIGH) {
      return "CAMERA_DETECTED";
    } else if (risk >= THRESHOLD_MEDIUM) {
      return "ANOMALY_WARNING";
    } else if (risk >= THRESHOLD_LOW) {
      return "LOW_CONFIDENCE";
    } else {
      return "SAFE";
    }
  }
  
  // Get alert confidence level
  float getConfidence(float risk) {
    // Normalize to 0-1 range
    if (risk >= THRESHOLD_HIGH) {
      return min(risk / 100.0, 1.0);
    } else if (risk >= THRESHOLD_MEDIUM) {
      return (risk - THRESHOLD_MEDIUM) / (THRESHOLD_HIGH - THRESHOLD_MEDIUM);
    } else {
      return 0;
    }
  }
  
  // Detailed analysis report
  void printDetailedReport(float rf, float wifi, float optical, float risk) {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║    DETECTION ANALYSIS REPORT           ║");
    Serial.println("╚════════════════════════════════════════╝");
    
    Serial.println("\n[SENSOR SCORES]");
    printScoreBar("RF Detection", rf);
    printScoreBar("WiFi Scanning", wifi);
    printScoreBar("Optical Detect", optical);
    
    Serial.println("\n[COMPOSITE RISK]");
    printScoreBar("Total Risk", risk);
    
    Serial.print("[STATUS] ");
    String status = getAlertStatus(risk);
    if (status == "CAMERA_DETECTED") {
      Serial.println("🚨 HIGH ALERT - Potential camera detected");
    } else if (status == "ANOMALY_WARNING") {
      Serial.println("⚠️  WARNING - Anomalous detection");
    } else if (status == "LOW_CONFIDENCE") {
      Serial.println("❓ LOW - Possible false positive");
    } else {
      Serial.println("✓ SAFE - No threats detected");
    }
    
    Serial.println("\n[TREND]");
    float trend = risk - risk_history[(history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE];
    if (trend > 10) {
      Serial.printf("Trend: ↑ Rising (Δ%.1f)\n", trend);
    } else if (trend < -10) {
      Serial.printf("Trend: ↓ Falling (Δ%.1f)\n", trend);
    } else {
      Serial.printf("Trend: → Stable\n");
    }
    
    Serial.println("\n╚════════════════════════════════════════╝\n");
  }
  
  // Helper: print visual score bar
  void printScoreBar(const char* label, float score) {
    Serial.printf("%-15s: ", label);
    
    // Score bar (20 characters)
    int bar_fill = (int)(score / 100.0 * 20);
    for (int i = 0; i < 20; i++) {
      if (i < bar_fill) Serial.print("█");
      else Serial.print("░");
    }
    
    // Score value and interpretation
    Serial.printf(" %.0f/100", score);
    
    if (score >= 70) Serial.print(" [CRITICAL]");
    else if (score >= 50) Serial.print(" [HIGH]");
    else if (score >= 30) Serial.print(" [MEDIUM]");
    else Serial.print(" [LOW]");
    
    Serial.println();
  }
  
  // Sensitivity adjustment (for user tuning)
  void setSensitivity(float level) {
    // level: 0.5 = low sensitivity, 1.0 = normal, 1.5 = high
    Serial.printf("[Risk] Sensitivity adjusted to %.1f×\n", level);
    // Could adjust weights here dynamically
  }
  
  // Get thresholds
  float getThresholdHigh() { return THRESHOLD_HIGH; }
  float getThresholdMedium() { return THRESHOLD_MEDIUM; }
  float getThresholdLow() { return THRESHOLD_LOW; }
};

#endif
