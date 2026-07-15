import serial
import time

# Set up the serial connection to the ESP
try:
    esp = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    print("Serial port connected successfully.")
except Exception as e:
    print(f"Error opening serial port: {e}")
    exit()

# Give the ESP time to initialize
time.sleep(2)

print("\n=============================================")
print("--- TERMINAL MOTOR CONTROL READY ---")
print("Type 'start' and press Enter to run motors.")
print("Type 'stop' and press Enter to stop motors.")
print("Type 'exit' and press Enter to quit the script.")
print("=============================================\n")

while True:
    # Read the text typed into the VS Code terminal
    user_command = input("Enter command: ").strip().lower()
    
    if user_command == "start":
        print("Sending START command to ESP...")
        esp.write(b"start\n")
        
    elif user_command == "stop":
        print("Sending STOP command to ESP...")
        esp.write(b"stop\n")
        
    elif user_command == "exit":
        print("Closing connection and exiting program.")
        break
        
    else:
        print("⚠️ Invalid command. Please type 'start', 'stop', or 'exit'.")

# Clean up and close the port when exiting the loop
esp.close()