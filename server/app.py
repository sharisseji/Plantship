"""
Flask server hub for ESP32 -> PC -> Arduino communication.

Endpoints:
- POST /sensor  - Receive sensor data from ESP32
- POST /voice   - Receive voice text (manual or from PTT)
- GET /health   - Health check
"""

import os
import threading
from flask import Flask, request, jsonify
from dotenv import load_dotenv

from serial_bridge import SerialBridge
from shrink import shrink

# Load environment variables
load_dotenv()

app = Flask(__name__)

# Global serial bridge instance
bridge: SerialBridge = None


def get_bridge() -> SerialBridge:
    """Get or create serial bridge connection."""
    global bridge
    if bridge is None:
        port = os.getenv("ARDUINO_COM_PORT", "COM3")
        bridge = SerialBridge(port)
        bridge.connect()
    return bridge


@app.route("/health", methods=["GET"])
def health():
    """Health check endpoint."""
    b = get_bridge()
    return jsonify({
        "status": "ok",
        "serial_connected": b.serial is not None and b.serial.is_open
    })


@app.route("/sensor", methods=["POST"])
def sensor():
    """
    Receive sensor data from ESP32.
    
    Expected JSON:
    {
        "temp": 23.7,      // optional
        "humidity": 41,    // optional  
        "moisture": 78     // optional
    }
    """
    data = request.get_json()
    
    if not data:
        return jsonify({"error": "No JSON data"}), 400
    
    print(f"[ESP32] Received: {data}")
    
    b = get_bridge()
    results = {}
    
    # Send each sensor value if present
    if "temp" in data:
        results["temp"] = b.send_temp(float(data["temp"]))
    
    if "humidity" in data:
        results["humidity"] = b.send_humidity(int(data["humidity"]))
    
    if "moisture" in data:
        results["moisture"] = b.send_moisture(int(data["moisture"]))
    
    return jsonify({
        "status": "ok",
        "sent": results
    })


@app.route("/voice", methods=["POST"])
def voice():
    """
    Receive voice text and send to LCD.
    
    Expected JSON:
    {
        "text": "turn on the lights"  // will be shrunk automatically
    }
    
    Or for pre-shrunk text:
    {
        "text": "LIGHTS ON",
        "shrink": false
    }
    """
    data = request.get_json()
    
    if not data or "text" not in data:
        return jsonify({"error": "Missing 'text' field"}), 400
    
    text = data["text"]
    
    # Optionally skip shrinking if already processed
    if data.get("shrink", True):
        text = shrink(text)
    
    b = get_bridge()
    success = b.send_voice(text)
    
    return jsonify({
        "status": "ok" if success else "error",
        "display_text": text
    })



def run_ptt_background():
    """Run PTT controller in background thread."""
    try:
        from ptt_mic import PTTController
        
        def on_voice(text):
            """Called when voice is transcribed."""
            print(f"[Voice] {text}")
            b = get_bridge()
            b.send_voice(text)
        
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
    
    # Initialize serial bridge
    print("=" * 50)
    print("MakeUofT2026 Hub Server")
    print("=" * 50)
    
    b = get_bridge()
    if not b.serial or not b.serial.is_open:
        print("[WARNING] Serial connection failed - LCD updates will not work")
    
    # Start PTT in background (optional)
    ptt_enabled = os.getenv("PTT_ENABLED", "true").lower() == "true"
    if ptt_enabled:
        ptt_thread = threading.Thread(target=run_ptt_background, daemon=True)
        ptt_thread.start()
        print(f"[PTT] Press {os.getenv('PTT_HOTKEY', 'ctrl+space')} to record")
    
    print(f"[Server] Starting on http://{host}:{port}")
    print("=" * 50)
    
    app.run(host=host, port=port, debug=False, threaded=True)


if __name__ == "__main__":
    main()
