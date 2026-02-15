"""Serial bridge to Arduino Uno LCD."""

import serial
import time
from typing import Optional


class SerialBridge:
    """Manages serial connection to Arduino Uno."""

    def __init__(self, port: str, baudrate: int = 115200, timeout: float = 1.0):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial: Optional[serial.Serial] = None

    def connect(self) -> bool:
        """Open serial connection. Returns True if successful."""
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=self.timeout
            )
            # Wait for Arduino to reset after connection
            time.sleep(2)
            print(f"[SerialBridge] Connected to {self.port}")
            return True
        except serial.SerialException as e:
            print(f"[SerialBridge] Failed to connect: {e}")
            return False

    def send(self, msg: str) -> bool:
        """Send a message to Arduino. Appends newline if not present."""
        if not self.serial or not self.serial.is_open:
            print("[SerialBridge] Not connected")
            return False

        if not msg.endswith('\n'):
            msg += '\n'

        try:
            self.serial.write(msg.encode('utf-8'))
            self.serial.flush()
            print(f"[SerialBridge] Sent: {msg.strip()}")
            
            # Wait briefly and check for response
            time.sleep(0.1)
            if self.serial.in_waiting > 0:
                response = self.serial.readline().decode('utf-8').strip()
                print(f"[SerialBridge] Arduino response: {response}")
            
            return True
        except serial.SerialException as e:
            print(f"[SerialBridge] Send failed: {e}")
            return False

    def send_temp(self, value: float) -> bool:
        """Send temperature update."""
        return self.send(f"S T {value:.1f}")

    def send_humidity(self, value: int) -> bool:
        """Send humidity update."""
        return self.send(f"S H {value}")

    def send_moisture(self, value: int) -> bool:
        """Send moisture update."""
        return self.send(f"S M {value}")

    def send_voice(self, text: str) -> bool:
        """Send voice text (max 20 chars)."""
        truncated = text[:20]
        return self.send(f"V {truncated}")

    def close(self):
        """Close serial connection."""
        if self.serial and self.serial.is_open:
            self.serial.close()
            print("[SerialBridge] Connection closed")

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


# Quick test
if __name__ == "__main__":
    import os
    from dotenv import load_dotenv

    load_dotenv()
    port = os.getenv("ARDUINO_COM_PORT", "COM3")

    with SerialBridge(port) as bridge:
        bridge.send_temp(23.7)
        time.sleep(0.5)
        bridge.send_humidity(41)
        time.sleep(0.5)
        bridge.send_moisture(78)
        time.sleep(0.5)
        bridge.send_voice("TEST MSG")
