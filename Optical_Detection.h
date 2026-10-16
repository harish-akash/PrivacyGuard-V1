/*
  Optical_Detection.h - IR Retroreflection Analysis
  
  Uses IR LED pulses (850nm) and phototransistor to detect camera lens retroreflection
  Scans multiple angles and analyzes reflection peaks
*/

#ifndef OPTICAL_DETECTION_H
#define OPTICAL_DETECTION_H

#include <Arduino.h>
#include <math.h>

class Optical_Detection {
private:
  int ir_led_pin = 32;
  int phototransistor_pin = 35;
  
  // Retroreflection detection parameters
  const float PEAK_THRESHOLD = 1500;  // ADC threshold for significant reflection
  const int PULSE_DURATION = 50;  // milliseconds
  const int ANGLES = 4;  // Number of scanning angles
  const int SAMPLES_PER_ANGLE = 100;
  
  // Baseline reflection (from environment)
  float baseline_reflection = 0;
  bool baseline_measured = false;
  
public:
  void init(int led_pin, int photo_pin) {
    ir_led_pin = led_pin;
    phototransistor_pin = photo_pin;
    
    pinMode(ir_led_pin, OUTPUT);
    pinMode(phototransistor_pin, INPUT);
    
    digitalWrite(ir_led_pin, LOW);
    
    Serial.println("[Optical] IR LED array initialized");
    Serial.println("[Optical] Phototransistor sensor initialized");
    
    // Measure baseline without LED
    measureBaseline();
  }
  
  // Measure ambient light (baseline without LED)
  void measureBaseline() {
    Serial.println("[Optical] Measuring baseline reflection...");
    
    int sum = 0;
    for (int i = 0; i < SAMPLES_PER_ANGLE; i++) {
      digitalWrite(ir_led_pin, LOW);  // LED off
      delayMicroseconds(100);
      sum += analogRead(phototransistor_pin);
    }
    
    baseline_reflection = sum / SAMPLES_PER_ANGLE;
    baseline_measured = true;
    
    Serial.printf("[Optical] Baseline: %.0f ADC counts\n", baseline_reflection);
  }
  
  // Main scanning function - checks multiple angles
  float scanRetroReflection(int duration_ms) {
    Serial.println("[Optical] Scanning for retroreflection...");
    
    float max_peak = 0;
    int detections = 0;
    
    for (int angle = 0; angle < ANGLES; angle++) {
      float peak = measureAngle(angle, duration_ms / ANGLES);
      
      if (peak > PEAK_THRESHOLD) {
        detections++;
        max_peak = max(max_peak, peak);
        Serial.printf("[Optical] Angle %d: Peak detected (%.0f)\n", angle * 90, peak);
      } else {
        Serial.printf("[Optical] Angle %d: No significant reflection (%.0f)\n", 
                      angle * 90, peak);
      }
    }
    
    // Calculate optical score
    float optical_score = calculateScore(max_peak, detections);
    
    return optical_score;
  }
  
  // Measure reflection at specific angle
  float measureAngle(int angle, int duration_ms) {
    // In a real implementation, this would rotate the LED array
    // For now, we measure with fixed geometry
    
    float max_reflection = 0;
    float peak_value = 0;
    
    // Pulse LED and sample phototransistor response
    int num_samples = SAMPLES_PER_ANGLE;
    
    for (int i = 0; i < num_samples; i++) {
      // LED pulse cycle
      digitalWrite(ir_led_pin, HIGH);
      delayMicroseconds(100);  // Let light propagate
      
      int reflection = analogRead(phototransistor_pin);
      
      digitalWrite(ir_led_pin, LOW);
      delayMicroseconds(100);  // Dark period for comparison
      
      int ambient = analogRead(phototransistor_pin);
      
      // Retroreflection signature: sharp peak when LED is on
      float peak = abs(reflection - ambient);
      max_reflection = max(max_reflection, peak);
      
      delayMicroseconds(500);
    }
    
    return max_reflection;
  }
  
  // Calculate optical score (0-100)
  float calculateScore(float max_peak, int detections) {
    float score = 0;
    
    // Strong retroreflection at multiple angles
    if (detections >= 2) {
      score = 60 + (detections * 10);  // Multiple angle confirmation
      Serial.printf("[Optical] Strong retroreflection (confidence: high)\n");
    }
    // Retroreflection at single angle
    else if (detections == 1) {
      score = 40 + (max_peak - PEAK_THRESHOLD) / 100;
      Serial.printf("[Optical] Single angle reflection detected\n");
    }
    // Weak reflection from shiny materials
    else if (max_peak > PEAK_THRESHOLD * 0.5) {
      score = 15;  // Low confidence - could be mirror/reflector
      Serial.printf("[Optical] Weak reflection (possible mirror)\n");
    }
    // No significant reflection
    else {
      score = 0;
      Serial.println("[Optical] No retroreflection detected");
    }
    
    return constrain(score, 0, 100);
  }
  
  // Advanced: Fourier analysis of retroreflection signature
  // (Optional enhancement for future version)
  float analyzeSignaturePattern(int num_samples) {
    Serial.println("[Optical] Analyzing reflection pattern...");
    
    // Sample reflected light at high frequency
    float* samples = new float[num_samples];
    
    for (int i = 0; i < num_samples; i++) {
      digitalWrite(ir_led_pin, HIGH);
      delayMicroseconds(10);
      samples[i] = analogRead(phototransistor_pin);
      digitalWrite(ir_led_pin, LOW);
      delayMicroseconds(10);
    }
    
    // Calculate statistics
    float mean = 0, variance = 0, peak = 0;
    
    for (int i = 0; i < num_samples; i++) {
      mean += samples[i];
      peak = max(peak, samples[i]);
    }
    mean /= num_samples;
    
    for (int i = 0; i < num_samples; i++) {
      variance += pow(samples[i] - mean, 2);
    }
    variance /= num_samples;
    
    float std_dev = sqrt(variance);
    
    // Lens retroreflection has characteristic signature:
    // - High peak relative to baseline
    // - Sharp rise/fall time
    // - Low variance around peak
    
    float sharpness = peak / (std_dev + 1);  // Higher = sharper peak
    
    Serial.printf("[Optical] Peak: %.0f, StdDev: %.0f, Sharpness: %.2f\n",
                  peak, std_dev, sharpness);
    
    delete[] samples;
    
    // Score based on sharpness (lens reflection is sharp)
    if (sharpness > 10) {
      return 50;  // High confidence lens
    } else if (sharpness > 5) {
      return 25;  // Medium confidence
    } else {
      return 0;   // Low confidence
    }
  }
  
  // Diagnostic: measure reflection at specific distance
  void diagnosticDistanceTest(int max_distance_cm) {
    Serial.println("\n=== OPTICAL DISTANCE TEST ===");
    Serial.println("Position object at various distances");
    Serial.println("Measured reflection (ADC counts):\n");
    
    for (int dist = 10; dist <= max_distance_cm; dist += 10) {
      Serial.printf("Distance: %d cm | Reflection: ", dist);
      
      // Pulse LED and measure
      digitalWrite(ir_led_pin, HIGH);
      delay(50);
      int reflection = analogRead(phototransistor_pin);
      digitalWrite(ir_led_pin, LOW);
      
      Serial.printf("%4d ADC | Signal: ", reflection);
      
      // Print bar graph
      int bar_length = reflection / 100;
      for (int i = 0; i < bar_length; i++) Serial.print("█");
      Serial.println();
      
      delay(1000);
    }
    
    Serial.println("=============================\n");
  }
  
  // Diagnostic: continuous monitoring mode
  void diagnosticMonitorMode(int duration_sec) {
    Serial.println("\n=== OPTICAL MONITORING MODE ===");
    Serial.printf("Monitoring for %d seconds...\n\n", duration_sec);
    
    unsigned long start_time = millis();
    
    while (millis() - start_time < duration_sec * 1000) {
      // LED on
      digitalWrite(ir_led_pin, HIGH);
      int with_led = analogRead(phototransistor_pin);
      digitalWrite(ir_led_pin, LOW);
      
      // LED off
      delayMicroseconds(100);
      int without_led = analogRead(phototransistor_pin);
      
      int difference = abs(with_led - without_led);
      
      Serial.printf("LED ON: %4d | LED OFF: %4d | Δ: %4d | ",
                    with_led, without_led, difference);
      
      // Bar graph
      int bar_length = difference / 50;
      for (int i = 0; i < bar_length; i++) Serial.print("▓");
      if (difference > PEAK_THRESHOLD) Serial.print(" ⚠️ PEAK");
      Serial.println();
      
      delay(100);
    }
    
    Serial.println("=============================\n");
  }
  
  // Get baseline value
  float getBaseline() {
    return baseline_reflection;
  }
};

#endif
