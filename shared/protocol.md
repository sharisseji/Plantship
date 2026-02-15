# Communication Protocol

This document defines the message formats used between components.

## Overview

```
ESP32 ──(HTTP POST)──► Flask Server ◄──(PC Mic PTT)
                            │
                            ▼
                      [Process/Shrink]
                            │
                            ▼
                  Arduino Uno (USB Serial)
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
  "temp": 23.7,
  "humidity": 41,
  "moisture": 78
}
```

All fields are optional - send only what changed or all at once.

**Response**:
```json
{
  "status": "ok",
  "sent": {
    "temp": true,
    "humidity": true,
    "moisture": true
  }
}
```

---

## Server → Arduino (Serial)

Line-based protocol over USB serial at **115200 baud**.

### Message Format

```
<CMD> <PARAM> <VALUE>\n
```

### Commands

| Command | Format | Example | Description |
|---------|--------|---------|-------------|
| Temperature | `S T <float>` | `S T 23.7` | Update temperature box |
| Humidity | `S H <int>` | `S H 41` | Update humidity box |
| Moisture | `S M <int>` | `S M 78` | Update moisture box |
| Voice | `V <text>` | `V LIGHTS ON` | Update voice box (max 20 chars) |

### Parsing Rules

1. Read until newline (`\n`)
2. Split on first space to get command
3. Parse remaining based on command type

**Arduino pseudo-code**:
```cpp
if (Serial.available()) {
  String line = Serial.readStringUntil('\n');
  line.trim();
  
  if (line.startsWith("S T ")) {
    float temp = line.substring(4).toFloat();
    updateTempBox(temp);
  }
  else if (line.startsWith("S H ")) {
    int hum = line.substring(4).toInt();
    updateHumidBox(hum);
  }
  else if (line.startsWith("S M ")) {
    int moist = line.substring(4).toInt();
    updateMoistBox(moist);
  }
  else if (line.startsWith("V ")) {
    String text = line.substring(2);
    updateVoiceBox(text);
  }
}
```

---

## LCD Layout (480x320, 4 boxes)

```
┌──────────────┬──────────────┐
│     TEMP     │    HUMID     │
│    23.7°C    │     41%      │
├──────────────┼──────────────┤
│    MOIST     │    VOICE     │
│      78      │  LIGHTS ON   │
└──────────────┴──────────────┘
```

### Box Coordinates (landscape mode)

| Box | X | Y | Width | Height |
|-----|---|---|-------|--------|
| Temp (top-left) | 0 | 0 | 240 | 160 |
| Humid (top-right) | 240 | 0 | 240 | 160 |
| Moist (bottom-left) | 0 | 160 | 240 | 160 |
| Voice (bottom-right) | 240 | 160 | 240 | 160 |

---

## Text Shrinking (Server-side)

Voice transcripts are processed before sending to Arduino:

1. **Intent mapping**: Match known phrases to short codes
   - "turn on the bedroom lights" → `BED LIGHT ON`
   - "what's the temperature" → `CHECK TEMP`

2. **Keyword extraction**: Remove filler words, keep 2-3 keywords
   - "could you please check the humidity level" → `CHECK HUMIDITY`

3. **Hard truncate**: Limit to 20 characters max

See `server/shrink.py` for implementation.

---

## Error Handling

### ESP32
- Retry HTTP POST up to 3 times on failure
- Continue sensor reads even if WiFi disconnected

### Server
- Return 400 for malformed requests
- Log serial errors but don't crash

### Arduino
- Ignore malformed serial messages
- Keep displaying last valid data
