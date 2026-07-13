import serial
import time

esp = serial.Serial('/dev/ttyUSB0', 115200)

time.sleep(2)

print("Sending start")
esp.write(b"start\n")

time.sleep(5)

print("Sending stop")
esp.write(b"stop\n")