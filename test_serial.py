import serial
try:
    ser = serial.Serial('COM13', 9600, timeout=1)
    print("Connected to COM13")
    ser.close()
except serial.SerialException as e:
    print(f"Error: {e}")