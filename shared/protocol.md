# Communication Protocol

This document defines the message formats used between components.

## Overview

```
ESP32-A ──┐                    ┌──► Arduino-A (8 boxes)
          ├──(WiFi/HTTP)──► Flask Server
ESP32-B ──┘                    └──► Arduino-B (8 boxes)
                                      
Both Arduinos show identical 8-box display with data from both ESP32s.
```

## ESP32 → Server (HTTP JSON)

### POST /sensor

Send sensor readings from ESP32 to PC server.

**URL**: `http://<PC_IP>:5000/sensor`  
**Method**: POST  
**Content-Type**: application/json

**Request Body**:
```json
{
  "device": "A",
  "temp": 23.7,
  "humidity": 41,
  "moisture": 78
}
```

- `device`: Required. Either `"A"` or `"B"` to identify the ESP32.
- All sensor fields are optional - send only what changed or all at once.

**Response**:
```json
{
  "status": "ok",
  "device": "A",
  "sent": {
    "temp": {"Arduino-A": true, "Arduino-B": true},
    "humidity": {"Arduino-A": true, "Arduino-B": true},
    "moisture": {"Arduino-A": true, "Arduino-B": true}
  }
}
```

### GET /status

View current sensor values from all devices.

**Response**:
```json
{
  "status": "ok",
  "devices": {
    "A": {"temp": 23.7, "humidity": 41, "moisture": 78},
    "B": {"temp": 25.1, "humidity": 55, "moisture": 62}
  }
}
```

---

## Server → Arduino (Serial)

Line-based protocol over USB serial at **115200 baud**.

### Message Format

```
<CMD> <DEVICE> <PARAM> <VALUE>\n
```

### Commands

| Command | Format | Example | Description |
|---------|--------|---------|-------------|
| Temperature | `S <D> T <float>` | `S A T 23.7` | Update device A/B temperature |
| Humidity | `S <D> H <int>` | `S B H 41` | Update device A/B humidity |
| Moisture | `S <D> M <int>` | `S A M 78` | Update device A/B moisture |
| Voice | `V <D> <text>` | `V A HELLO` | Update device A/B voice (max 12 chars) |

Where `<D>` is the device ID: `A` or `B`

### Parsing Rules

1. Read until newline (`\n`)
2. Split to get command type, device ID, and parameters
3. Update corresponding box on display

**Arduino pseudo-code**:
```cpp
if (line.startsWith("S ")) {
    char device = line.charAt(2);  // 'A' or 'B'
    char sensor = line.charAt(4);  // 'T', 'H', or 'M'
    String value = line.substring(6);
    
    if (device == 'A') {
        if (sensor == 'T') updateBox(BOX_A_TEMP, value);
        else if (sensor == 'H') updateBox(BOX_A_HUMID, value);
        else if (sensor == 'M') updateBox(BOX_A_MOIST, value);
    }
    else if (device == 'B') {
        if (sensor == 'T') updateBox(BOX_B_TEMP, value);
        // ... etc
    }
}
else if (line.startsWith("V ")) {
    char device = line.charAt(2);
    String text = line.substring(4);
    
    if (device == 'A') updateBox(BOX_A_VOICE, text);
    else if (device == 'B') updateBox(BOX_B_VOICE, text);
}
```

---

## LCD Layout (480x320, 8 boxes - 2x4 grid)

```
┌────────┬────────┬────────┬────────┐
│ A:TEMP │ A:HUM  │ A:MOIST│ A:VOICE│
│ 23.7°C │  41%   │   78   │ READY  │
├────────┼────────┼────────┼────────┤
│ B:TEMP │ B:HUM  │ B:MOIST│ B:VOICE│
│ 25.1°C │  55%   │   62   │ READY  │
└────────┴────────┴────────┴────────┘
```

### Box Coordinates (landscape mode)

| Box | Index | X | Y | Width | Height |
|-----|-------|---|---|-------|--------|
| A:TEMP | 0 | 0 | 0 | 120 | 160 |
| A:HUM | 1 | 120 | 0 | 120 | 160 |
| A:MOIST | 2 | 240 | 0 | 120 | 160 |
| A:VOICE | 3 | 360 | 0 | 120 | 160 |
| B:TEMP | 4 | 0 | 160 | 120 | 160 |
| B:HUM | 5 | 120 | 160 | 120 | 160 |
| B:MOIST | 6 | 240 | 160 | 120 | 160 |
| B:VOICE | 7 | 360 | 160 | 120 | 160 |

---

## Multi-Device Setup

### Hardware
- **PC-1**: Powers both ESP32s (via USB or power supply), provides WiFi
- **PC-2**: Runs Flask server, connects to both Arduinos via USB

### Configuration

**ESP32-A** (`temphumid.cpp`):
```cpp
const char* DEVICE_ID = "A";
const char* SERVER_URL = "http://<PC-2_IP>:5000/sensor";
```

**ESP32-B** (`temphumid.cpp`):
```cpp
const char* DEVICE_ID = "B";
const char* SERVER_URL = "http://<PC-2_IP>:5000/sensor";
```

**Flask** (`.env` on PC-2):
```env
ARDUINO_COM_PORT_A=COM9
ARDUINO_COM_PORT_B=COM3
```

---

## Text Shrinking (Server-side)

Voice transcripts are processed before sending to Arduino:

1. **Intent mapping**: Match known phrases to short codes
   - "turn on the bedroom lights" → `BED LIT`
   - "what's the temperature" → `CHK TMP`

2. **Keyword extraction**: Remove filler words, keep 1-2 keywords
   - "could you please check the humidity level" → `CHK HUM`

3. **Hard truncate**: Limit to 12 characters max (for 120px wide boxes)

See `server/shrink.py` for implementation.

---

## Error Handling

### ESP32
- Retry HTTP POST up to 3 times on failure
- Continue sensor reads even if WiFi disconnected
- Include device ID in all requests

### Server
- Return 400 for malformed requests
- Log serial errors but don't crash
- Broadcast to all connected Arduinos

### Arduino
- Ignore malformed serial messages
- Keep displaying last valid data
- Respond with "OK <D> <S>" or "ERR ..."
