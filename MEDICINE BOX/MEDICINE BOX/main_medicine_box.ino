#include <WiFi.h>
#include <Wire.h>
#include "ADXL345.h"
#include <math.h>
#include <BluetoothSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

const char* ssid = "AZRAIL";
const char* password = "2962hana";

WiFiServer server(80);
ADXL345 adxl;

// BLE Configuration
#define DEVICE_NAME "MedicineBox"
#define SERVICE_UUID_STATUS      "ad82a001-23b9-4e8e-8d92-6cb65f0c1a10"
#define CHAR_UUID_BOX_STATUS     "ad82a002-23b9-4e8e-8d92-6cb65f0c1a10"
#define CHAR_UUID_PREDICTION     "ad82a003-23b9-4e8e-8d92-6cb65f0c1a10"
#define CHAR_UUID_MISSED_DOSES   "ad82a004-23b9-4e8e-8d92-6cb65f0c1a10"
#define SERVICE_UUID_INFO        "ad82a010-23b9-4e8e-8d92-6cb65f0c1a10"
#define CHAR_UUID_COMPANY        "ad82a011-23b9-4e8e-8d92-6cb65f0c1a10"
#define CHAR_UUID_FIRMWARE       "ad82a012-23b9-4e8e-8d92-6cb65f0c1a10"
#define SERVICE_UUID_OTA         "66443771-d481-49b0-be32-8ce24ac0f09c"
#define CHAR_UUID_OTA_DATA       "66443772-d481-49b0-be32-8ce24ac0f09c"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharBoxStatus = nullptr;
BLECharacteristic* pCharPrediction = nullptr;
BLECharacteristic* pCharMissedDoses = nullptr;
BLECharacteristic* pCharOtaData = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;

int xOffset = 0, yOffset = 0, zOffset = 0;
unsigned long lastOpenTime = 0;
bool wasOpenPrev = false;

// BLE Server Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE: Client connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE: Client disconnected");
  }
};

// OTA Callbacks
class OtaCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("OTA Data received: ");
    Serial.println(value.c_str());
    // TODO: Implement actual OTA logic
  }
};

void setupBLE() {
  // Initialize BLE
  BLEDevice::init(DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Status Service
  BLEService* statusSvc = pServer->createService(SERVICE_UUID_STATUS);
  pCharBoxStatus = statusSvc->createCharacteristic(
    CHAR_UUID_BOX_STATUS,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharBoxStatus->addDescriptor(new BLE2902());

  pCharPrediction = statusSvc->createCharacteristic(
    CHAR_UUID_PREDICTION,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharPrediction->addDescriptor(new BLE2902());

  pCharMissedDoses = statusSvc->createCharacteristic(
    CHAR_UUID_MISSED_DOSES,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharMissedDoses->addDescriptor(new BLE2902());
  statusSvc->start();

  // Device Info Service
  BLEService* infoSvc = pServer->createService(SERVICE_UUID_INFO);
  BLECharacteristic* charCompany = infoSvc->createCharacteristic(
    CHAR_UUID_COMPANY, BLECharacteristic::PROPERTY_READ
  );
  charCompany->setValue("Khep Labs");
  
  BLECharacteristic* charFirmware = infoSvc->createCharacteristic(
    CHAR_UUID_FIRMWARE, BLECharacteristic::PROPERTY_READ
  );
  charFirmware->setValue("v1.0.0");
  infoSvc->start();

  // OTA Service
  BLEService* otaSvc = pServer->createService(SERVICE_UUID_OTA);
  pCharOtaData = otaSvc->createCharacteristic(
    CHAR_UUID_OTA_DATA,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharOtaData->setCallbacks(new OtaCallbacks());
  otaSvc->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_STATUS);
  pAdvertising->addServiceUUID(SERVICE_UUID_OTA);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE advertising started");
  Serial.print("Device name: ");
  Serial.println(DEVICE_NAME);
}

void updateBLEData(String boxStatus, float angleX, float angleY, float prediction) {
  if (!deviceConnected) return;

  // Update Box Status
  if (pCharBoxStatus) {
    pCharBoxStatus->setValue(boxStatus.c_str());
    pCharBoxStatus->notify();
  }

  // Update Prediction (probability based on angles)
  if (pCharPrediction) {
    char predStr[10];
    snprintf(predStr, sizeof(predStr), "%.2f", prediction);
    pCharPrediction->setValue(predStr);
    pCharPrediction->notify();
  }

  // Update Missed Doses (time since last open)
  if (pCharMissedDoses) {
    unsigned long currentTime = millis();
    unsigned long timeSinceOpen = (lastOpenTime == 0) ? 0 : (currentTime - lastOpenTime) / 1000;
    String timeStr = String(timeSinceOpen);
    pCharMissedDoses->setValue(timeStr.c_str());
    pCharMissedDoses->notify();
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  adxl.powerOn();
  adxl.setRangeSetting(16);

  // Setup BLE first
  setupBLE();

  // Setup WiFi
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
  // Handle BLE connections
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  int raw[3];
  adxl.readAccel(raw);

  int x = raw[0] - xOffset;
  int y = raw[1] - yOffset;
  int z = raw[2] - zOffset;

  float x_g = x * 0.004;
  float y_g = y * 0.004;
  float z_g = z * 0.004;

  // Calculate tilt angles in degrees for X and Y axes
  float angleX = atan2(x_g, sqrt(y_g * y_g + z_g * z_g)) * 180.0 / PI;
  float angleY = atan2(y_g, sqrt(x_g * x_g + z_g * z_g)) * 180.0 / PI;

  // Determine if box is closed (angles near 0 ±5 degrees)
  bool isNearZero = (abs(angleX) < 5.0) && (abs(angleY) < 5.0);
  bool isOpen = !isNearZero;

  // Track box opening events
  if (isOpen && !wasOpenPrev) {
    lastOpenTime = millis();
    Serial.println("Box opened - medication taken!");
  }
  wasOpenPrev = isOpen;

  String boxStatus = isOpen ? "📭 Box Open" : "📦 Box Closed";
  
  // Calculate prediction probability (simplified ML-like approach)
  float prediction = 0.0;
  if (isOpen) {
    prediction = 1.0; // Currently open
  } else {
    // Predict based on time since last open and angle patterns
    unsigned long timeSinceOpen = (lastOpenTime == 0) ? 0 : (millis() - lastOpenTime) / 1000;
    if (timeSinceOpen > 3600) { // More than 1 hour
      prediction = 0.8; // High probability of opening soon
    } else if (timeSinceOpen > 1800) { // More than 30 minutes
      prediction = 0.5; // Medium probability
    } else {
      prediction = 0.1; // Low probability
    }
  }

  // Update BLE characteristics
  updateBLEData(boxStatus, angleX, angleY, prediction);

  // Print to Serial Monitor
  Serial.print("Angle X: ");
  Serial.print(angleX, 2);
  Serial.print("°, Angle Y: ");
  Serial.print(angleY, 2);
  Serial.print("° --> ");
  Serial.print(boxStatus);
  Serial.print(" | Prediction: ");
  Serial.print(prediction, 2);
  Serial.print(" | Time since open: ");
  Serial.print((lastOpenTime == 0) ? 0 : (millis() - lastOpenTime) / 1000);
  Serial.println("s");

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
    client.println("<h1>Smart Medicine Box Dashboard</h1>");
    client.printf("<p>Angle X: %.2f &deg;</p>", angleX);
    client.printf("<p>Angle Y: %.2f &deg;</p>", angleY);
    client.printf("<p>Z Accel: %.3f g</p>", z_g);
    client.print("<h2 style='font-size:4em;'>");
    client.print(boxStatus);
    client.println("</h2>");
    client.printf("<p>Prediction: %.2f</p>", prediction);
    client.printf("<p>Time since last open: %ds</p>", (lastOpenTime == 0) ? 0 : (millis() - lastOpenTime) / 1000);
    client.println("<p><small>BLE Device: MedicineBox</small></p>");
    client.println("</body></html>");

    delay(10);
    client.stop();
  }

  delay(200);
}
