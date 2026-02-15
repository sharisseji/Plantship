"""
Flask server hub for ESP32 -> PC -> Arduino communication.

Supports multiple ESP32 devices (A, B) and multiple Arduino displays.

Endpoints:
- POST /sensor  - Receive sensor data from ESP32 (with device ID)
- POST /voice   - Receive voice text (manual or from PTT)
- GET /health   - Health check
- GET /status   - View current sensor values from all devices
"""

import os
import threading
from flask import Flask, request, jsonify
from dotenv import load_dotenv

from serial_bridge import MultiBridge
from shrink import shrink

# Load environment variables
load_dotenv()

app = Flask(__name__)

# Global multi-bridge instance (manages multiple Arduinos)
multi_bridge: MultiBridge = None

# Store latest sensor data from each device
sensor_data = {
    "A": {"temp": None, "humidity": None, "moisture": None},
    "B": {"temp": None, "humidity": None, "moisture": None},
}


def get_multi_bridge() -> MultiBridge:
    """Get or create multi-bridge connection."""
    global multi_bridge
    if multi_bridge is None:
        multi_bridge = MultiBridge()
        
        # Connect to Arduino A
        port_a = os.getenv("ARDUINO_COM_PORT_A")
        if port_a:
            if multi_bridge.add_bridge(port_a, "Arduino-A"):
                print(f"[Bridge] Arduino-A connected on {port_a}")
            else:
                print(f"[Bridge] Arduino-A failed on {port_a}")
        
        # Connect to Arduino B
        port_b = os.getenv("ARDUINO_COM_PORT_B")
        if port_b:
            if multi_bridge.add_bridge(port_b, "Arduino-B"):
                print(f"[Bridge] Arduino-B connected on {port_b}")
            else:
                print(f"[Bridge] Arduino-B failed on {port_b}")
        
        print(f"[Bridge] Total connected: {multi_bridge.connected_count}")
    
    return multi_bridge


@app.route("/health", methods=["GET"])
def health():
    """Health check endpoint."""
    mb = get_multi_bridge()
    return jsonify({
        "status": "ok",
        "arduinos_connected": mb.connected_count
    })


@app.route("/status", methods=["GET"])
def status():
    """View current sensor values from all devices."""
    return jsonify({
        "status": "ok",
        "devices": sensor_data
    })


@app.route("/sensor", methods=["POST"])
def sensor():
    """
    Receive sensor data from ESP32.
    
    Expected JSON:
    {
        "device": "A",     // required: "A" or "B"
        "temp": 23.7,      // optional
        "humidity": 41,    // optional  
        "moisture": 78     // optional
    }
    """
    data = request.get_json()
    
    if not data:
        return jsonify({"error": "No JSON data"}), 400
    
    # Get device ID (default to "A" for backwards compatibility)
    device = data.get("device", "A").upper()
    if device not in ["A", "B"]:
        return jsonify({"error": "Invalid device ID, must be A or B"}), 400
    
    print(f"[ESP32-{device}] Received: {data}")
    
    mb = get_multi_bridge()
    results = {}
    
    # Send each sensor value if present (broadcast to all Arduinos)
    if "temp" in data:
        temp = float(data["temp"])
        sensor_data[device]["temp"] = temp
        results["temp"] = mb.send_temp(device, temp)
    
    if "humidity" in data:
        humidity = int(data["humidity"])
        sensor_data[device]["humidity"] = humidity
        results["humidity"] = mb.send_humidity(device, humidity)
    
    if "moisture" in data:
        moisture = int(data["moisture"])
        sensor_data[device]["moisture"] = moisture
        results["moisture"] = mb.send_moisture(device, moisture)
    
    return jsonify({
        "status": "ok",
        "device": device,
        "sent": results
    })


@app.route("/voice", methods=["POST"])
def voice():
    """
    Receive voice text and send to LCD.
    
    Expected JSON:
    {
        "device": "A",                  // optional, default "A"
        "text": "turn on the lights"    // will be shrunk automatically
    }
    
    Or for pre-shrunk text:
    {
        "device": "A",
        "text": "LIGHTS ON",
        "shrink": false
    }
    """
    data = request.get_json()
    
    if not data or "text" not in data:
        return jsonify({"error": "Missing 'text' field"}), 400
    
    device = data.get("device", "A").upper()
    if device not in ["A", "B"]:
        return jsonify({"error": "Invalid device ID, must be A or B"}), 400
    
    text = data["text"]
    
    # Optionally skip shrinking if already processed
    if data.get("shrink", True):
        text = shrink(text)
    
    mb = get_multi_bridge()
    results = mb.send_voice(device, text)
    
    return jsonify({
        "status": "ok",
        "device": device,
        "display_text": text,
        "sent": results
    })


def run_ptt_background():
    """Run PTT controller in background thread."""
    try:
        from ptt_mic import PTTController
        
        def on_voice(text):
            """Called when voice is transcribed."""
            print(f"[Voice] {text}")
            mb = get_multi_bridge()
            # Default to device A for PTT
            mb.send_voice("A", text)
        
        controller = PTTController(
            hotkey=os.getenv("PTT_HOTKEY", "ctrl+space"),
            on_transcript=on_voice
        )
        controller.start()
        
        # Keep thread alive
        import keyboard
        keyboard.wait()
        
    except ImportError as e:
        print(f"[PTT] Keyboard library not available: {e}")
    except Exception as e:
        print(f"[PTT] Error: {e}")


def main():
    """Run the server."""
    host = os.getenv("FLASK_HOST", "0.0.0.0")
    port = int(os.getenv("FLASK_PORT", 5000))
    
    # Initialize serial bridges
    print("=" * 50)
    print("MakeUofT2026 Hub Server (Multi-Device)")
    print("=" * 50)
    
    mb = get_multi_bridge()
    if mb.connected_count == 0:
        print("[WARNING] No Arduinos connected - LCD updates will not work")
    
    # Start PTT in background (optional)
    ptt_enabled = os.getenv("PTT_ENABLED", "false").lower() == "true"
    if ptt_enabled:
        ptt_thread = threading.Thread(target=run_ptt_background, daemon=True)
        ptt_thread.start()
        print(f"[PTT] Press {os.getenv('PTT_HOTKEY', 'ctrl+space')} to record")
    
    print(f"[Server] Starting on http://{host}:{port}")
    print(f"[Server] Listening for ESP32-A and ESP32-B")
    print("=" * 50)
    
    app.run(host=host, port=port, debug=False, threaded=True)


if __name__ == "__main__":
    main()
