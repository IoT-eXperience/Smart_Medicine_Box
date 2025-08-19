#include <Wire.h>
#include "ADXL345.h"
#include <math.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
ADXL345 adxl;

int xOffset = 0, yOffset = 0, zOffset = 0;

float predict_box_status(float angle_x, float angle_y) {
    float logit = -10.21934052 + -0.13532308 * angle_x + 2.16820608 * angle_y;
    float prob = 1.0f / (1.0f + expf(-logit));
    return prob > 0.5f ? 1.0f : 0.0f; // 1.0 = Box Open, 0.0 = Box Closed
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  adxl.powerOn();
  adxl.setRangeSetting(16);

  // Calibrate offsets
  int raw[3];
  adxl.readAccel(raw);
  xOffset = raw[0];
  yOffset = raw[1];
  zOffset = raw[2] - 256;

  // Start Bluetooth
  SerialBT.begin("ESP32_MedicineBox"); // Bluetooth device name
  Serial.println("Bluetooth started! Pair and connect using your phone.");
}

void loop() {
  int raw[3];
  adxl.readAccel(raw);

  float x_g = (raw[0] - xOffset) * 0.004;
  float y_g = (raw[1] - yOffset) * 0.004;
  float z_g = (raw[2] - zOffset) * 0.004;

  float angleX = atan2(x_g, sqrt(y_g * y_g + z_g * z_g)) * 180.0 / PI;
  float angleY = atan2(y_g, sqrt(x_g * x_g + z_g * z_g)) * 180.0 / PI;

  float result = predict_box_status(angleX, angleY);
  String status = (result > 0.5) ? "Box Open" : "Box Closed";

  // Send data over Bluetooth
  String msg = "AngleX:" + String(angleX, 2) + ", AngleY:" + String(angleY, 2) + ", Status:" + status + "\n";
  SerialBT.print(msg);

  delay(1000);
}