import serial

# Can just do proper timestamping here instead of on the ESP just for convenience.
# In future when this isn't connected over serial, the ESP32 should transmit proper time.

if __name__ == "__main__":
    ser = serial.Serial('COM4', 115200)
    while True:
        data = ser.readline().decode().strip()
        if data:
            print(data)
