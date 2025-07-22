# 📦 ESP32 Box Tilt Detection and Web Dashboard

This project uses an **ESP32** microcontroller and an **ADXL345 accelerometer** to detect the tilt of a box in real-time. The system determines whether the box is "closed" (flat) or "open" (tilted) and displays the results both over **serial** and a **refreshing web interface**. Additionally, a **Python script** is provided to log the tilt data and box status into a CSV file using serial communication.

---

## 🧰 Hardware Used

- ESP32 Dev Board
- ADXL345 Accelerometer (I2C)
- Wi-Fi access point for ESP32 connection

---

## 🚀 Features

- Detects tilt angles (X and Y) in real-time
- Classifies box state: 📦 **Box Closed** or 📭 **Box Open**
- Built-in Wi-Fi server for displaying data via browser
- Logs tilt data with timestamps to CSV using Python
- Auto-refreshing live dashboard (1-second refresh)

---

## 🌐 Web Interface

Access the ESP32 IP (shown in serial monitor) via a browser:
