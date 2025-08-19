import serial
import csv
import time
from datetime import datetime
import keyboard  # Make sure to install this

# Change this to your actual COM port
SERIAL_PORT = 'COM4'
BAUD_RATE = 115200
CSV_FILE = 'box_tilt_log.csv'

def main():
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for ESP32 to be ready

    with open(CSV_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Timestamp', 'Angle_X_deg', 'Angle_Y_deg', 'Box_Status'])

        print("Logging started... Press 'X' to stop.")
        try:
            while True:
                if keyboard.is_pressed('x'):
                    print("Stopping log by keypress.")
                    break

                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if "Angle X:" in line and "Angle Y:" in line:
                    try:
                        parts = line.split("°")
                        angle_x = float(parts[0].split(":")[1].strip())
                        angle_y = float(parts[1].split(":")[1].strip())
                        status = "Box Closed" if "📦" in line else "Box Open"

                        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
                        writer.writerow([timestamp, angle_x, angle_y, status])
                        print(f"{timestamp} -> X: {angle_x:.2f}, Y: {angle_y:.2f}, Status: {status}")
                    except (ValueError, IndexError):
                        continue

        finally:
            ser.close()

if __name__ == '__main__':
    main()
