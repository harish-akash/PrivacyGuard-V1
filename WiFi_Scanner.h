/*
  WiFi_Scanner.h - Passive WiFi Anomaly Detection
  
  Scans for suspicious SSIDs, known camera patterns, and unencrypted networks
  Uses ESP32 built-in WiFi for passive scanning
*/

#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFi_Scanner {
private:
  // Known camera SSID patterns
  const char* camera_keywords[] = {
    "Cam", "Cloud", "SkynetCam", "CamCloud", "DVR", "NVR", 
    "IPCam", "NetCam", "WebCam", "AxisCam", "HiCam"
  };
  const int num_keywords = 11;
  
  // Known camera OUI (MAC address vendor prefixes)
  // Format: first 3 bytes of MAC address
  const char* camera_ouis[] = {
    "00:13:10",  // Hikvision
    "60:E8:67",  // Xiaomi (some cameras)
    "BC:F5:AC",  // TP-Link (cameras)
    "00:0D:4F",  // Acme
  };
  const int num_ouis = 4;
  
  // Whitelist of known safe SSIDs/devices
  const char* whitelist_ssids[] = {
    "AndroidHotspot", "iPhone", "Guest", "Public", "Starbucks_WiFi"
  };
  const int num_whitelist = 5;
  
  String detected_ssid = "";
  int detected_rssi = -100;
  float wifi_score = 0;
  
public:
  void init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();  // Disable WiFi connection (passive mode only)
    Serial.println("[WiFi] Passive scanning mode enabled");
  }
  
  // Scan all WiFi networks and score for anomalies
  float scanAndScore() {
    Serial.println("[WiFi] Starting network scan...");
    
    int num_networks = WiFi.scanNetworks();
    float max_score = 0;
    
    if (num_networks == 0) {
      Serial.println("[WiFi] No networks found");
      detected_ssid = "";
      return 0;
    }
    
    Serial.printf("[WiFi] Found %d networks\n", num_networks);
    
    for (int i = 0; i < num_networks; i++) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      uint8_t* bssid = WiFi.BSSID(i);
      int channel = WiFi.channel(i);
      wifi_auth_mode_t encryption = WiFi.encryptionType(i);
      
      float network_score = scoreNetwork(ssid, rssi, bssid, encryption);
      
      if (network_score > max_score) {
        max_score = network_score;
        detected_ssid = ssid;
        detected_rssi = rssi;
      }
      
      // Log suspicious networks
      if (network_score > 30) {
        Serial.printf("[WiFi] SUSPICIOUS: %s | Score: %.1f | RSSI: %d dBm | CH: %d\n",
                      ssid.c_str(), network_score, rssi, channel);
      }
    }
    
    wifi_score = max_score;
    WiFi.scanDelete();  // Clear scan results
    
    return max_score;
  }
  
  // Score individual network based on multiple factors
  float scoreNetwork(String ssid, int rssi, uint8_t* bssid, wifi_auth_mode_t encryption) {
    float score = 0;
    
    // 1. Check if on whitelist (safe networks)
    if (isWhitelisted(ssid)) {
      return 0;
    }
    
    // 2. Check SSID against known camera patterns
    if (matchesCameraSSID(ssid)) {
      score += 50;
      Serial.printf("  → Camera SSID pattern match: %s\n", ssid.c_str());
    }
    
    // 3. Check MAC vendor (known camera manufacturers)
    if (matchesCameraOUI(bssid)) {
      score += 40;
      Serial.printf("  → Camera OUI match: %02X:%02X:%02X\n", bssid[0], bssid[1], bssid[2]);
    }
    
    // 4. Unencrypted network with strong signal = suspicious
    if (encryption == WIFI_AUTH_OPEN) {
      score += 25;
      Serial.printf("  → Unencrypted network detected\n");
      
      // High RSSI (close proximity) unencrypted = more suspicious
      if (rssi > -55) {
        score += 15;
        Serial.printf("  → Strong signal from open network (%d dBm)\n", rssi);
      }
    }
    
    // 5. Odd SSID patterns (very short, all lowercase, numbers, etc.)
    if (isOddSSID(ssid)) {
      score += 10;
      Serial.printf("  → Unusual SSID pattern\n");
    }
    
    // 6. Strong signal in unusual location
    if (rssi > -40) {  // Very strong = possibly hidden/concealed
      score += 5;
    }
    
    return constrain(score, 0, 100);
  }
  
  // Check if SSID matches known camera patterns
  bool matchesCameraSSID(String ssid) {
    for (int i = 0; i < num_keywords; i++) {
      if (ssid.indexOf(camera_keywords[i]) >= 0) {
        return true;
      }
    }
    return false;
  }
  
  // Check if MAC OUI matches known camera manufacturers
  bool matchesCameraOUI(uint8_t* bssid) {
    for (int i = 0; i < num_ouis; i++) {
      // Compare first 3 bytes
      if (bssid[0] == strtol(String(camera_ouis[i]).substring(0, 2).c_str(), NULL, 16) &&
          bssid[1] == strtol(String(camera_ouis[i]).substring(3, 5).c_str(), NULL, 16) &&
          bssid[2] == strtol(String(camera_ouis[i]).substring(6, 8).c_str(), NULL, 16)) {
        return true;
      }
    }
    return false;
  }
  
  // Check if SSID is on whitelist (safe)
  bool isWhitelisted(String ssid) {
    for (int i = 0; i < num_whitelist; i++) {
      if (ssid.equals(whitelist_ssids[i]) || ssid.indexOf(whitelist_ssids[i]) >= 0) {
        return true;
      }
    }
    return false;
  }
  
  // Detect unusual SSID patterns (likely auto-generated)
  bool isOddSSID(String ssid) {
    // Too short
    if (ssid.length() < 3) return true;
    
    // All numbers
    bool all_numbers = true;
    for (char c : ssid) {
      if (!isDigit(c) && c != '-') {
        all_numbers = false;
        break;
      }
    }
    if (all_numbers) return true;
    
    // Random hex-like pattern (common in auto-generated SSIDs)
    if (ssid.length() == 4 && isHexString(ssid)) {
      return true;
    }
    
    return false;
  }
  
  // Helper: check if string is valid hex
  bool isHexString(String str) {
    for (char c : str) {
      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
        return false;
      }
    }
    return true;
  }
  
  // Getters
  String getDetectedSSID() {
    return detected_ssid;
  }
  
  int getDetectedRSSI() {
    return detected_rssi;
  }
  
  float getScore() {
    return wifi_score;
  }
  
  // Diagnostic: print all networks
  void diagnosticScan() {
    Serial.println("\n=== WiFi DIAGNOSTIC SCAN ===");
    int num_networks = WiFi.scanNetworks();
    
    for (int i = 0; i < num_networks; i++) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      uint8_t* bssid = WiFi.BSSID(i);
      int channel = WiFi.channel(i);
      wifi_auth_mode_t encryption = WiFi.encryptionType(i);
      
      Serial.printf("%2d. SSID: %-20s | RSSI: %3d dBm | CH: %d | ",
                    i+1, ssid.c_str(), rssi, channel);
      Serial.print("Security: ");
      printEncryption(encryption);
      Serial.println();
    }
    
    WiFi.scanDelete();
    Serial.println("===========================\n");
  }
  
  void printEncryption(wifi_auth_mode_t auth) {
    switch(auth) {
      case WIFI_AUTH_OPEN: Serial.print("OPEN"); break;
      case WIFI_AUTH_WEP: Serial.print("WEP"); break;
      case WIFI_AUTH_WPA_PSK: Serial.print("WPA"); break;
      case WIFI_AUTH_WPA2_PSK: Serial.print("WPA2"); break;
      case WIFI_AUTH_WPA_WPA2_PSK: Serial.print("WPA/WPA2"); break;
      case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-Ent"); break;
      case WIFI_AUTH_WPA3_PSK: Serial.print("WPA3"); break;
      default: Serial.print("UNKNOWN"); break;
    }
  }
};

#endif
