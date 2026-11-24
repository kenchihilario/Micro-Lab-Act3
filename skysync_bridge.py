"""
Sky-Sync serial bridge (Lab Activity 3) GROUP KENCHI

Install:  python -m pip install pyserial requests
Run:      python skysync_bridge.py
"""
import socket
import sys
import time

import requests
import serial
import serial.tools.list_ports

API_KEY = "3AA8BWW19Q7ENZ1S"
BAUD_RATE = 9600
UPLOAD_INTERVAL = 16 
THINGSPEAK_URL = "https://api.thingspeak.com/update"


def find_port():
    """Find the Arduino's serial port on any OS. Ask the user if it is not obvious."""
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        sys.exit("No serial ports found. Plug in the Arduino and try again.")

    keywords = ("arduino", "ch340", "wch", "usb serial", "usbmodem", "ttyacm", "ttyusb")
    likely = [
        p for p in ports
        if any(k in f"{p.description} {p.device} {p.manufacturer or ''}".lower() for k in keywords)
    ]
    if len(likely) == 1:
        return likely[0].device

    choices = likely or ports
    print("Select the Arduino port:")
    for i, p in enumerate(choices, start=1):
        print(f"  {i}. {p.device} - {p.description}")
    try:
        return choices[int(input("Number: ")) - 1].device
    except (ValueError, IndexError):
        sys.exit("Invalid choice.")


def get_ip():
    """Return this laptop's local IP address, or None if there is no network."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except OSError:
        return None


def parse_line(line):
    """Turn '23.4,61,627' into (23.4, 61, 627). Return None for bad lines."""
    parts = line.split(",")
    if len(parts) != 3:
        return None
    try:
        return float(parts[0]), int(parts[1]), int(parts[2])
    except ValueError:
        return None


def upload(api_key, distance, light):
    """Send one reading to ThingSpeak. Returns True if it was accepted."""
    params = {"api_key": api_key, "field2": light}
    if distance >= 0:                    
        params["field1"] = distance
    try:
        r = requests.get(THINGSPEAK_URL, params=params, timeout=10)
        return r.status_code == 200 and r.text.strip() != "0"
    except requests.RequestException:
        return False


def main():
    api_key = API_KEY
    if api_key == "YOUR_WRITE_API_KEY":
        api_key = input("Enter your ThingSpeak Write API Key: ").strip()

    port = find_port()
    print(f"Opening {port} at {BAUD_RATE} baud ...")
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=3)
    except serial.SerialException as e:
        sys.exit(f"Cannot open {port}. Close the Arduino Serial Monitor and try again.\n{e}")

    time.sleep(2) 
    ser.reset_input_buffer()

    last_upload = 0.0
    cloud_ok = False
    status = ""

    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            reading = parse_line(line)
            if reading is None:
                continue
            distance, light, raw = reading

            now = time.time()
            if now - last_upload >= UPLOAD_INTERVAL:
                cloud_ok = upload(api_key, distance, light)
                last_upload = now
                status = ("Data Packet Sent Successfully!" if cloud_ok
                          else "Upload FAILED (check API key / internet)")
            else:
                wait = int(UPLOAD_INTERVAL - (now - last_upload))
                status = f"{'Last upload OK' if cloud_ok else 'Last upload failed'} - next in {wait} s"

            ip = get_ip()
            dist_text = f"{distance:.1f} cm" if distance >= 0 else "No echo"

            print("*" * 60)
            print(f"Laptop Network: {'Connected (IP: ' + ip + ')' if ip else 'NOT connected'}")
            print(f"Cloud Platform: ThingSpeak [{'ONLINE' if cloud_ok else 'OFFLINE'}]")
            print()
            print(f"Current Distance Reading: {dist_text}")
            print(f"Current Light Intensity:  {light} % (Raw ADC: {raw})")
            print()
            print(f"Cloud Sync Status: {status}")
            print("*" * 60)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
