// Logistic Regression Model for Box Open/Closed
// Features: Angle_X_deg, Angle_Y_deg
#include <math.h>

float predict_box_status(float angle_x, float angle_y) {
    float logit = -10.21934052 + -0.13532308 * angle_x + 2.16820608 * angle_y;
    float prob = 1.0f / (1.0f + expf(-logit));
    return prob > 0.5f ? 1.0f : 0.0f; // 1.0 = Box Open, 0.0 = Box Closed
}

void setup() {
  Serial.begin(115200);
  // Example usage:
  float angle_x = 0.0; // Replace with your sensor value
  float angle_y = 5.0; // Replace with your sensor value
  float result = predict_box_status(angle_x, angle_y);
  if (result > 0.5) {
    Serial.println("Box Open");
  } else {
    Serial.println("Box Closed");
  }
}

void loop() {
  // In your real application, call predict_box_status with live sensor data
  // Example (replace with actual sensor readings):
  float angle_x = 0.0; // Replace with your sensor value
  float angle_y = 5.0; // Replace with your sensor value
  float result = predict_box_status(angle_x, angle_y);
  if (result > 0.5) {
    Serial.println("Box Open");
  } else {
    Serial.println("Box Closed");
  }
  delay(1000);
} 