import platform
import serial
import time

if platform.system() == "Windows":
    PORT = "COM4"          # Change to your ESP32's COM port
else:
    PORT = "/dev/ttyUSB0"  # Raspberry Pi

esp = serial.Serial(PORT, 115200)

time.sleep(2)

print("Sending start")
esp.write(b"start\n")

time.sleep(5)

print("Sending stop")
esp.write(b"stop\n")