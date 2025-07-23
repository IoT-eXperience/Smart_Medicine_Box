#include <WiFi.h>
#include <Wire.h>
#include "ADXL345.h"
#include <math.h>

const char* ssid = "AZRAIL";
const char* password = "2962hana";

WiFiServer server(80);
ADXL345 adxl;

int xOffset = 0, yOffset = 0, zOffset = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  adxl.powerOn();
  adxl.setRangeSetting(16);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  Serial.println("Calibrating... keep sensor still and flat");
  delay(3000);

  int raw[3];
  adxl.readAccel(raw);
  xOffset = raw[0];
  yOffset = raw[1];
  zOffset = raw[2] - 256; // Adjust for gravity on Z axis

  Serial.print("Offsets set: X=");
  Serial.print(xOffset);
  Serial.print(" Y=");
  Serial.print(yOffset);
  Serial.print(" Z=");
  Serial.println(zOffset);
}

void loop() {
  int raw[3];
  adxl.readAccel(raw);

  int x = raw[0] - xOffset;
  int y = raw[1] - yOffset;
  int z = raw[2] - zOffset;

  float x_g = x * 0.004;
  float y_g = y * 0.004;
  float z_g = z * 0.004;

  // Calculate tilt angles in degrees for X and Y axes
  // angleX = rotation around Y axis (tilt forward/back)
  float angleX = atan2(x_g, sqrt(y_g * y_g + z_g * z_g)) * 180.0 / PI;

  // angleY = rotation around X axis (tilt left/right)
  float angleY = atan2(y_g, sqrt(x_g * x_g + z_g * z_g)) * 180.0 / PI;

  // Determine if box is closed (angles near 0 ±5 degrees)
  bool isNearZero = (abs(angleX) < 5.0) && (abs(angleY) < 5.0);

  String boxStatus = isNearZero ? "📦 Box Closed" : "📭 Box Open";

  // Print to Serial Monitor
  Serial.print("Angle X: ");
  Serial.print(angleX, 2);
  Serial.print("°, Angle Y: ");
  Serial.print(angleY, 2);
  Serial.print("° --> ");
  Serial.println(boxStatus);

  // Serve web page if client connected
  WiFiClient client = server.available();
  if (client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.println("<!DOCTYPE html><html><head>");
    client.println("<meta http-equiv='refresh' content='1'/>");
    client.println("<style>body{font-family:sans-serif;text-align:center;padding-top:50px;font-size:1.5em;}</style>");
    client.println("</head><body>");
    client.println("<h1>Tilt Angles from ADXL345</h1>");
    client.printf("<p>Angle X: %.2f &deg;</p>", angleX);
    client.printf("<p>Angle Y: %.2f &deg;</p>", angleY);
    client.printf("<p>Z Accel: %.3f g</p>", z_g);
    client.print("<h2 style='font-size:4em;'>");
    client.print(boxStatus);
    client.println("</h2>");
    client.println("</body></html>");

    delay(10);
    client.stop();
  }

  delay(200);
}
