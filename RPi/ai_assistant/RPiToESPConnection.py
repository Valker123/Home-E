import serial
import time

# Set up the serial connection using your verified USB port
try:
    esp = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    print("Serial port connected successfully.")
except Exception as e:
    print(f"Error opening serial port: {e}")
    exit()

# Give the ESP time to initialize
time.sleep(2)

print("\n=============================================")
print("--- TERMINAL STATE & DIRECTION CONTROL ---")
print("Modes:")
print("  'manual'  Switch to User Controlled Mode")
print("  'auto'   Switch to Autonomous Mode")
print("\nDirections (Manual Mode Only):")
print("  'w'  Drive Forward")
print("  'a'  Turn Left")
print("  'd'  Turn Right")
print("  's'  Drive Backward")
print("  'v'  Stop Motors")
print("\nType 'exit' and press Enter to quit the script.")
print("=============================================\n")

while True:
    user_command = input("Enter command: ").strip().lower()
    
    if user_command == "exit":
        print("Stopping robot and exiting program.")
        esp.write(b'V') # Send stop command before closing
        break
        
    # State switches
    elif user_command == "manual":
        print("Shifting state: User Controlled (M)")
        esp.write(b'M')
    elif user_command == "auto":
        print("Shifting state: Fully Autonomous (T)")
        esp.write(b'T')
        
    # Manual directional steering overrides
    elif user_command == "w":
        print("Sending: Forward (W)")
        esp.write(b'W')
    elif user_command == "s":
        print("Sending: Backward (S)")
        esp.write(b'S')
    elif user_command == "a":
        print("Sending: Turn Left (A)")
        esp.write(b'A')
    elif user_command == "d":
        print("Sending: Turn Right (D)")
        esp.write(b'D')
    elif user_command == "v":
        print("Sending: Stop (V)")
        esp.write(b'V')
        
    else:
        print("Invalid command. Type a direction (w/a/d/s) or a state mode (manual/auto).")

esp.close()