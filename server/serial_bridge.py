"""Serial bridge to Arduino Uno LCD."""

import serial
import time
from typing import Optional, List


class SerialBridge:
    """Manages serial connection to Arduino Uno."""

    def __init__(self, port: str, name: str = "", baudrate: int = 115200, timeout: float = 1.0):
        self.port = port
        self.name = name or port
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
            print(f"[SerialBridge:{self.name}] Connected to {self.port}")
            return True
        except serial.SerialException as e:
            print(f"[SerialBridge:{self.name}] Failed to connect: {e}")
            return False

    def send(self, msg: str) -> bool:
        """Send a message to Arduino. Appends newline if not present."""
        if not self.serial or not self.serial.is_open:
            print(f"[SerialBridge:{self.name}] Not connected")
            return False

        if not msg.endswith('\n'):
            msg += '\n'

        try:
            self.serial.write(msg.encode('utf-8'))
            self.serial.flush()
            print(f"[SerialBridge:{self.name}] Sent: {msg.strip()}")
            
            # Wait briefly and check for response
            time.sleep(0.1)
            if self.serial.in_waiting > 0:
                response = self.serial.readline().decode('utf-8').strip()
                print(f"[SerialBridge:{self.name}] Arduino response: {response}")
            
            return True
        except serial.SerialException as e:
            print(f"[SerialBridge:{self.name}] Send failed: {e}")
            return False

    def send_temp(self, device: str, value: float) -> bool:
        """Send temperature update for a device."""
        return self.send(f"S {device} T {value:.1f}")

    def send_humidity(self, device: str, value: int) -> bool:
        """Send humidity update for a device."""
        return self.send(f"S {device} H {value}")

    def send_moisture(self, device: str, value: int) -> bool:
        """Send moisture update for a device."""
        return self.send(f"S {device} M {value}")

    def send_voice(self, device: str, text: str) -> bool:
        """Send voice text for a device (max 12 chars for 8-box layout)."""
        truncated = text[:12]
        return self.send(f"V {device} {truncated}")

    def close(self):
        """Close serial connection."""
        if self.serial and self.serial.is_open:
            self.serial.close()
            print(f"[SerialBridge:{self.name}] Connection closed")

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()


class MultiBridge:
    """Manages multiple serial connections and broadcasts to all."""

    def __init__(self):
        self.bridges: List[SerialBridge] = []

    def add_bridge(self, port: str, name: str = "") -> bool:
        """Add and connect a new serial bridge."""
        bridge = SerialBridge(port, name)
        if bridge.connect():
            self.bridges.append(bridge)
            return True
        return False

    def broadcast(self, msg: str) -> dict:
        """Send message to all connected bridges."""
        results = {}
        for bridge in self.bridges:
            results[bridge.name] = bridge.send(msg)
        return results

    def send_temp(self, device: str, value: float) -> dict:
        """Broadcast temperature to all Arduinos."""
        results = {}
        for bridge in self.bridges:
            results[bridge.name] = bridge.send_temp(device, value)
        return results

    def send_humidity(self, device: str, value: int) -> dict:
        """Broadcast humidity to all Arduinos."""
        results = {}
        for bridge in self.bridges:
            results[bridge.name] = bridge.send_humidity(device, value)
        return results

    def send_moisture(self, device: str, value: int) -> dict:
        """Broadcast moisture to all Arduinos."""
        results = {}
        for bridge in self.bridges:
            results[bridge.name] = bridge.send_moisture(device, value)
        return results

    def send_voice(self, device: str, text: str) -> dict:
        """Broadcast voice to all Arduinos."""
        results = {}
        for bridge in self.bridges:
            results[bridge.name] = bridge.send_voice(device, text)
        return results

    def close_all(self):
        """Close all connections."""
        for bridge in self.bridges:
            bridge.close()
        self.bridges = []

    @property
    def connected_count(self) -> int:
        return len([b for b in self.bridges if b.serial and b.serial.is_open])


# Quick test
if __name__ == "__main__":
    import os
    from dotenv import load_dotenv

    load_dotenv()
    port_a = os.getenv("ARDUINO_COM_PORT_A", "COM9")
    port_b = os.getenv("ARDUINO_COM_PORT_B", "COM3")

    multi = MultiBridge()
    multi.add_bridge(port_a, "Arduino-A")
    multi.add_bridge(port_b, "Arduino-B")

    print(f"Connected: {multi.connected_count} Arduinos")
    
    # Test broadcast
    multi.send_temp("A", 23.7)
    time.sleep(0.3)
    multi.send_temp("B", 25.1)
    time.sleep(0.3)
    multi.send_humidity("A", 41)
    time.sleep(0.3)
    multi.send_voice("A", "TEST")
    
    multi.close_all()
