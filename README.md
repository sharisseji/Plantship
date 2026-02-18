# Smart Plant Monitor — MakeUofT 2026

An IoT plant health monitoring system that displays real-time sensor data on an LCD display with an expressive plant "mood" interface. Features voice command support via push-to-talk.

## Features

- **Real-time environmental monitoring** — Temperature, humidity, and soil moisture
- **Expressive LCD interface** — Plant shows happy/sad faces based on moisture levels
- **Voice commands** — Push-to-talk with ElevenLabs speech-to-text
- **Wireless connectivity** — ESP32 sends sensor data over WiFi to a Flask server

## System Architecture

```
┌─────────────────┐     HTTP POST      ┌─────────────────┐     USB Serial    ┌─────────────────┐
│     ESP32       │ ─────────────────► │   Flask Server  │ ────────────────► │   Arduino Uno   │
│  (Sensor Hub)   │    WiFi / JSON     │     (PC)        │    115200 baud    │   (LCD Display) │
└─────────────────┘                    └─────────────────┘                    └─────────────────┘
        │                                      │
   DHT11 + Moisture                     Push-to-Talk Mic
      Sensors                          (ElevenLabs STT)
```

## Project Structure

```
MakeUofT2026/
├── ArduinoUno-Firmware/          # LCD display controller
│   ├── src/
│   │   └── lcd.cpp               # Main LCD display code with mood system
│   └── platformio.ini            # Arduino Uno build config
│
├── ESP32-Firmware/               # Sensor hub
│   ├── src/
│   │   ├── main.cpp              # Basic moisture sensor test
│   │   └── temphumid.cpp         # Full sensor hub with WiFi + HTTP
│   ├── include/
│   │   ├── credentials.h         # WiFi credentials (gitignored)
│   │   └── credentials.h.example # Template for credentials
│   └── platformio.ini            # ESP32-S3 build config
│
├── server/                       # Python Flask server
│   ├── app.py                    # Main server with REST endpoints
│   ├── serial_bridge.py          # Arduino serial communication
│   ├── shrink.py                 # Text shrinking for LCD display
│   ├── ptt_mic.py                # Push-to-talk microphone recording
│   ├── stt_elevenlabs.py         # ElevenLabs speech-to-text
│   └── requirements.txt          # Python dependencies
│
└── shared/
    └── protocol.md               # Communication protocol documentation
```

## Hardware Requirements

| Component | Description |
|-----------|-------------|
| **ESP32-S3 WROOM** | Freenove ESP32-S3 for sensor data collection |
| **Arduino Uno** | Display controller |
| **3.5" TFT LCD (480×320)** | ILI9481 driver, MCUFRIEND shield |
| **DHT11** | Temperature & humidity sensor (GPIO 15) |
| **Capacitive Soil Moisture Sensor** | Analog moisture reading (GPIO 16) |
| **USB Cables** | ESP32 (COM11) + Arduino (COM9) to PC |

## Dependencies

### Python Server

```
flask==3.0.0
pyserial==3.5
python-dotenv==1.0.0
requests==2.31.0
sounddevice==0.4.6
scipy==1.11.4
keyboard==0.13.5
```

### Arduino Uno (PlatformIO)

- `adafruit/Adafruit GFX Library@^1.12.4`
- `prenticedavid/MCUFRIEND_kbv@^3.1.0-Beta`
- `adafruit/Adafruit TouchScreen@^1.1.6`

### ESP32-S3 (PlatformIO)

- `beegee-tokyo/DHT sensor library for ESPx@^1.19`
- `madhephaestus/ESP32Servo`

## Setup Instructions

### 1. Clone & Install Python Dependencies

```bash
cd server
pip install -r requirements.txt
```

### 2. Configure Environment Variables

Create a `.env` file in the `server/` directory:

```env
ARDUINO_COM_PORT=COM9
FLASK_HOST=0.0.0.0
FLASK_PORT=5000
PTT_ENABLED=true
PTT_HOTKEY=ctrl+space
ELEVENLABS_API_KEY=your_api_key_here
```

### 3. Configure ESP32 WiFi

Copy and edit the credentials file:

```bash
cd ESP32-Firmware/include
cp credentials.h.example credentials.h
```

Edit `credentials.h` or directly in `temphumid.cpp`:

```cpp
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourPassword";
const char* SERVER_URL = "http://YOUR_PC_IP:5000/sensor";
```

### 4. Flash Firmware (PlatformIO)

**Arduino Uno:**
```bash
cd ArduinoUno-Firmware
pio run -t upload
```

**ESP32-S3:**
```bash
cd ESP32-Firmware
pio run -t upload
```

### 5. Run the Server

```bash
cd server
python app.py
```

Server starts at `http://0.0.0.0:5000`

## 📡 API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/sensor` | POST | Receive sensor data from ESP32 |
| `/voice` | POST | Send voice text to LCD |
| `/health` | GET | Health check |

### POST /sensor Example

```json
{
  "temp": 23.7,
  "humidity": 65,
  "moisture": 2100
}
```

## Moisture Thresholds

| Moisture Value | Status | Plant Mood |
|---------------|--------|------------|
| > 2000 | GOOD | 😊 Happy |
| 1000 - 2000 | AVERAGE | 😊 Happy |
| < 1000 | BAD | 😢 Sad |

## Voice Commands

Hold `Ctrl+Space` to record, release to send. The system uses ElevenLabs STT and automatically shrinks text for the LCD display.

Supported intents:
- "Turn on the lights" → `LIGHTS ON`
- "Check temperature" → `CHECK TEMP`
- "Water the plant" → `WATER PLANT`

## Serial Protocol

Commands sent from server to Arduino (115200 baud):

| Command | Format | Example |
|---------|--------|---------|
| Temperature | `S T <float>` | `S T 23.7` |
| Humidity | `S H <int>` | `S H 65` |
| Moisture | `S M <int>` | `S M 2100` |
| Healthy | `H` | `H` |
| Unhealthy | `U` | `U` |

## Team

Built at MakeUofT 2026

Check out our Devpost [HERE](https://devpost.com/software/plantship-r15mzo)