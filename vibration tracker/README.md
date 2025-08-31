# ESP32 IMU Vibration Tracker

This project provides ESP32 code for reading IMU sensor data and tracking vibrations. It includes two options:
 - Generic MPU6050/MPU9250 reader: `esp32_imu_reader.ino`
 - CodeCell board reader using the CodeCell library: `codecell_imu_reader.ino`

## Features

- **Real-time IMU Data Reading**: Accelerometer, gyroscope, and magnetometer data
- **Configurable Sampling Rate**: Adjustable from 10Hz to 1000Hz
- **Multiple Output Formats**: CSV for data logging, JSON for API transmission
- **WiFi Connectivity**: Optional data transmission to remote server
- **Vibration Analysis**: Built-in magnitude and RMS calculations
- **Serial Commands**: Interactive control via serial interface
- **Status Indicators**: LED feedback for system status

## Hardware Requirements

- ESP32 development board
- IMU sensor (MPU6050, MPU9250, or compatible)
- Jumper wires for connections
- Optional: WiFi network for data transmission

## Wiring

### ESP32 to IMU Sensor (MPU6050/MPU9250)

| ESP32 Pin | IMU Pin | Description |
|-----------|---------|-------------|
| 3.3V      | VCC     | Power supply |
| GND       | GND     | Ground      |
| GPIO 21   | SDA     | I2C Data    |
| GPIO 22   | SCL     | I2C Clock   |

### Optional LED Indicator
- Connect LED to GPIO 2 with appropriate resistor

## Installation

1. **Install Arduino IDE** with ESP32 board support
2. **Install Required Libraries**:
   - Wire (usually included with Arduino IDE)
   - ArduinoJson (for JSON data formatting; optional)
   - CodeCell library (for CodeCell board): install from source `https://github.com/microbotsio/CodeCell.git` or add as Arduino library

3. **Upload the Code**:
   - For generic IMU boards: open `esp32_imu_reader.ino`
   - For CodeCell board (ESP32-C3 + BNO085): open `codecell_imu_reader.ino`
   - Select your ESP32 board
   - Upload the code

## Configuration

Edit `config.h` to customize settings:

```cpp
// Sampling rate (Hz)
#define SAMPLE_RATE 100

// WiFi credentials (if using WiFi)
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"

// Sensor sensitivity ranges
#define ACCEL_RANGE 2    // ±2g
#define GYRO_RANGE 250   // ±250°/s
```

## Usage

### Basic Operation

1. **Power on** the ESP32
2. **Open Serial Monitor** at 115200 baud
3. **View real-time data** in CSV format
4. **Use serial commands** for additional features

### Serial Commands (generic sketch)

- `status` - Show system status
- `detailed` - Show detailed sensor data
- `magnitude` - Show acceleration magnitude
- `rms` - Show RMS acceleration
- `help` - Show available commands

### Data Output

The system outputs data in CSV format by default:
For CodeCell sketch, the CSV header is:

```
ts_ms,ax,ay,az,gx,gy,gz,mx,my,mz,linAx,linAy,linAz,gravX,gravY,gravZ,roll,pitch,yaw
```


```
Timestamp,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,MagX,MagY,MagZ,Temperature
12345,0.123,0.456,9.789,0.001,0.002,0.003,0.0,0.0,0.0,25.3
```

## Vibration Analysis

### Acceleration Magnitude
Calculates the total acceleration magnitude:
```
magnitude = √(accelX² + accelY² + accelZ²)
```

### RMS (Root Mean Square)
Provides a measure of vibration intensity over a window of samples.

## Troubleshooting

### Common Issues

1. **IMU Not Detected**:
   - Check wiring connections
   - Verify I2C address (default: 0x68) for generic sketch
   - For CodeCell: ensure the CodeCell library is installed and sensor is enabled via `myCodeCell.Init()` macros
   - Ensure proper power supply (3.3V)

2. **No Data Output**:
   - Check serial monitor baud rate (115200)
   - Verify IMU initialization in serial output

3. **WiFi Connection Issues** (generic sketch):
   - Check SSID and password in config.h
   - Ensure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)

## CodeCell Library

This project supports the CodeCell board via the official library. See the upstream docs for features, init macros, and read functions: [CodeCell GitHub repository](https://github.com/microbotsio/CodeCell.git).

Key APIs used (from the CodeCell README):
- `myCodeCell.Init(LIGHT + MOTION_ACCELEROMETER + MOTION_GYRO + MOTION_MAGNETOMETER + MOTION_LINEAR_ACC + MOTION_GRAVITY + MOTION_ROTATION)`
- `myCodeCell.Run(rateHz)` returns true at 10–100 Hz and manages power/LED
- `Motion_AccelerometerRead`, `Motion_GyroRead`, `Motion_MagnetometerRead`, `Motion_LinearAccRead`, `Motion_GravityRead`, `Motion_RotationRead`

Reference: [CodeCell](https://github.com/microbotsio/CodeCell.git)

### Debug Mode

Enable debug mode in `config.h`:
```cpp
#define DEBUG_MODE true
#define VERBOSE_OUTPUT true
```

## Data Analysis

### Python Analysis Script

Use the provided `analyze_vibration_data.py` script to analyze collected data:

```bash
python analyze_vibration_data.py vibration_data.csv
```

### Features:
- Time series plotting
- Frequency domain analysis (FFT)
- Vibration intensity metrics
- Statistical analysis

## API Integration

### Sending Data to Server

1. Configure WiFi settings in `config.h`
2. Set `ENABLE_WIFI_UPLOAD true`
3. Update `SERVER_URL` with your endpoint

### JSON Format

Data is sent in JSON format:
```json
{
  "timestamp": 12345,
  "accel": {"x": 0.123, "y": 0.456, "z": 9.789},
  "gyro": {"x": 0.001, "y": 0.002, "z": 0.003},
  "mag": {"x": 0.0, "y": 0.0, "z": 0.0},
  "temperature": 25.3
}
```

## Performance Considerations

- **Sampling Rate**: Higher rates provide more detail but consume more power
- **WiFi Usage**: Disable if not needed to save power
- **Serial Output**: Can be disabled for production use

## Customization

### Adding New Sensors

1. Modify the `IMUData` structure
2. Update the `readIMUData()` function
3. Add new sensor initialization code

### Custom Analysis

Add new analysis functions:
```cpp
float customVibrationMetric() {
    // Your custom calculation
    return result;
}
```

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Contributing

Contributions are welcome! Please feel free to submit issues and enhancement requests.
