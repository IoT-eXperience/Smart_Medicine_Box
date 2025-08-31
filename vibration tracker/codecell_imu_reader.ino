#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <CodeCell.h>

// Wi-Fi credentials
const char* WIFI_SSID = "AZRAIL";
const char* WIFI_PSK  = "2962hana";

// API endpoint
const char* serverName = "https://iotexperience.com/medicinebox";

// CodeCell IMU setup
CodeCell myCodeCell;
const uint8_t SAMPLE_HZ = 100; // CodeCell Run frequency (10..100)
const uint32_t FEATURES =
  MOTION_ACCELEROMETER +
  MOTION_GYRO +
  MOTION_MAGNETOMETER +
  MOTION_LINEAR_ACC +
  MOTION_GRAVITY +
  MOTION_ROTATION; // roll/pitch/yaw

// Latest readings to post
float g_ax = 0, g_ay = 0, g_az = 0;
float g_gx = 0, g_gy = 0, g_gz = 0;
unsigned long g_lastPostMs = 0;
const unsigned long POST_INTERVAL_MS = 5000;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PSK);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Initialize CodeCell IMU
  myCodeCell.Init(FEATURES);
}

void loop() {
  // Update sensors at SAMPLE_HZ; when true, fresh data are available
  if (myCodeCell.Run(SAMPLE_HZ)) {
    myCodeCell.Motion_AccelerometerRead(g_ax, g_ay, g_az);
    myCodeCell.Motion_GyroRead(g_gx, g_gy, g_gz);

    Serial.print("IMU ax,ay,az: ");
    Serial.print(g_ax, 3); Serial.print(", ");
    Serial.print(g_ay, 3); Serial.print(", ");
    Serial.println(g_az, 3);

    Serial.print("IMU gx,gy,gz: ");
    Serial.print(g_gx, 3); Serial.print(", ");
    Serial.print(g_gy, 3); Serial.print(", ");
    Serial.println(g_gz, 3);
  }

  if (WiFi.status() == WL_CONNECTED && millis() - g_lastPostMs >= POST_INTERVAL_MS) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // Build JSON payload
    String jsonData = "{";
    jsonData += "\"device\":\"ESP32_Box_01\",";
    jsonData += "\"gyroX\":" + String(g_gx, 3) + ",";
    jsonData += "\"gyroY\":" + String(g_gy, 3) + ",";
    jsonData += "\"gyroZ\":" + String(g_gz, 3) + ",";
    jsonData += "\"accelX\":" + String(g_ax, 3) + ",";
    jsonData += "\"accelY\":" + String(g_ay, 3) + ",";
    jsonData += "\"accelZ\":" + String(g_az, 3);
    jsonData += "}";

    Serial.println("Sending JSON: ");
    Serial.println(jsonData);

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println("Response: " + payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
    g_lastPostMs = millis();
  } else {
    Serial.println("WiFi Disconnected");
  }
}