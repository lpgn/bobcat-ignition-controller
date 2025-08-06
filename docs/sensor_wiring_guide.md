# Bobcat Ignition Controller - Sensor Wiring Guide

## ESP32 Pin Assignments

### Digital Output Pins (Relay Control) - Active HIGH
| Pin | GPIO | Function | Description |
|-----|------|----------|-------------|
| 21 | GPIO21 | Main Power Relay | Controls main engine power (Relay 1) |
| 19 | GPIO19 | Glow Plug Relay | Controls glow plug heating (Relay 2) |
| 18 | GPIO18 | Starter Relay | Controls starter solenoid (Relay 3) |
| 5 | GPIO5 | Work Lights Relay | Controls front/back lights (Relay 4) |

### Analog Input Pins (Engine Sensors)
| Pin | GPIO | ADC Channel | Function | Voltage Range |
|-----|------|-------------|----------|---------------|
| 34 | GPIO34 | ADC1_CH6 | Coolant Temperature | 0-3.3V |
| 35 | GPIO35 | ADC1_CH7 | Oil Pressure Sensor | 0-3.3V |
| 36 | GPIO36 | ADC1_CH0 | Battery Voltage Monitor | 0-3.3V (via divider) |
| 39 | GPIO39 | ADC1_CH3 | Fuel Level Sensor | 0-3.3V |

### Digital Input Pins (Status Feedback)
| Pin | GPIO | Function | Input Type |
|-----|------|----------|------------|
| 22 | GPIO22 | Alternator Charge Indicator | Digital (Active LOW) |
| 26 | GPIO26 | Engine Running Feedback | Digital (Active HIGH) |

### Power Management Pins
| Pin | GPIO | Function | Input Type |
|-----|------|----------|------------|
| 0 | GPIO0 | Wake-Up Button (BOOT) | Digital (Active LOW, Pull-up) |
| 12 | GPIO12 | Sleep Enable Control | Digital (Active LOW, Pull-up) |

---

## Sensor Wiring Details

### 1. Coolant Temperature Sensor (GPIO34)
**Sensor Type**: NTC Thermistor (typical automotive type)

```
Wiring:
Sensor Pin 1 ──── 3.3V (ESP32)
Sensor Pin 2 ──┬─ GPIO34 (ESP32)
                └─ 10kΩ Resistor ──── GND

Circuit Type: Voltage divider with pull-down resistor
Voltage Range: 0-3.3V (varies with temperature)
```

**Required Components:**
- 10kΩ resistor (pull-down)
- Optional: 0.1µF capacitor across sensor for noise filtering

### 2. Oil Pressure Sensor (GPIO35)
**Sensor Type**: 0-5V automotive pressure transducer

```
Wiring:
Sensor Red ────── 12V+ (vehicle power)
Sensor Black ──── GND (vehicle ground)
Sensor Signal ─┬─ 2.2kΩ Resistor ──── GPIO35 (ESP32)
               └─ 3.3kΩ Resistor ──── GND

Circuit Type: Voltage divider (5V to 3.3V conversion)
Input Range: 0-5V → Output Range: 0-3.0V (safe for ESP32)
```

**Required Components:**
- 2.2kΩ resistor (series)
- 3.3kΩ resistor (pull-down)
- Optional: 0.1µF capacitor for filtering

**Voltage Divider Calculation:**
- Vout = Vin × (3.3kΩ ÷ (2.2kΩ + 3.3kΩ)) = Vin × 0.6
- 5V input → 3.0V output (safe for ESP32)

### 3. Battery Voltage Monitor (GPIO36)
**Purpose**: Monitor 12V vehicle battery voltage

```
Wiring:
12V Battery+ ─┬─ 22kΩ Resistor ──── GPIO36 (ESP32)
              └─ 10kΩ Resistor ──── GND

Circuit Type: Voltage divider (12V to 3.3V conversion)
Input Range: 9-15V → Output Range: 0.9-1.5V
```

**Required Components:**
- 22kΩ resistor (series, high voltage side)
- 10kΩ resistor (pull-down, low voltage side)
- Optional: 0.1µF capacitor for filtering

**Voltage Divider Calculation:**
- Vout = Vin × (10kΩ ÷ (22kΩ + 10kΩ)) = Vin × 0.3125
- 12V input → 3.75V output ⚠️ **EXCEEDS ESP32 3.3V LIMIT!**
- **ISSUE**: Current 22kΩ+10kΩ divider saturates ADC (reads 4095 constantly)
- **RECOMMENDED**: Use 33kΩ + 10kΩ for 2.8V output or 47kΩ + 12kΩ for 2.4V output

### 4. Fuel Level Sensor (GPIO39)
**Sensor Type**: Variable resistance fuel sender (10-180Ω typical)

```
Wiring:
12V+ ──── 1kΩ Resistor ──┬─ GPIO39 (ESP32)
                         └─ Fuel Sender ──── GND

Circuit Type: Pull-up voltage divider
Resistance Range: 10Ω (empty) to 180Ω (full)
Voltage Range: 0.1V (empty) to 1.8V (full)
```

**Required Components:**
- 1kΩ resistor (pull-up)
- Optional: 0.1µF capacitor for filtering

### 5. Alternator Charge Indicator (GPIO22)
**Signal Type**: 12V "charge" light signal (Active LOW)

```
Wiring:
Alternator "L" Terminal ─┬─ 10kΩ Resistor ──── GPIO22 (ESP32)
                         └─ 10kΩ Resistor ──── 3.3V

Circuit Type: Voltage divider with pull-up
Logic: 0V = Charging, 12V = Not Charging
Output: HIGH = Charging, LOW = Not Charging
```

**Required Components:**
- Two 10kΩ resistors (voltage divider + pull-up)

### 6. Engine Running Feedback (GPIO26)
**Signal Type**: 12V signal when engine is running

```
Wiring:
Engine Run Signal (12V) ─┬─ 10kΩ Resistor ──── GPIO26 (ESP32)
                         └─ 10kΩ Resistor ──── GND

Circuit Type: Voltage divider
Logic: 12V = Engine Running, 0V = Engine Off
Output: HIGH = Running, LOW = Stopped
```

**Required Components:**
- Two 10kΩ resistors (voltage divider)

---

## Power Supply Requirements

### ESP32 Power
- **Input**: 5V via USB or VIN pin
- **Current**: ~250mA (normal operation), <20µA (deep sleep)
- **Source**: Vehicle 12V → 5V DC-DC converter recommended

### Relay Power (if using external relay board)
- **Voltage**: 12V or 5V (depending on relay board)
- **Current**: 20-50mA per relay when active
- **Isolation**: Optocoupled relay boards recommended

---

## Connection Summary Table

| Component | ESP32 Pin | Required Resistors | Notes |
|-----------|-----------|-------------------|-------|
| Coolant Temp | GPIO34 | 10kΩ pull-down | NTC thermistor |
| Oil Pressure | GPIO35 | 2.2kΩ series + 3.3kΩ pull-down | 5V→3.3V conversion |
| Battery Voltage | GPIO36 | 22kΩ series + 10kΩ pull-down | 12V→3.3V conversion |
| Fuel Level | GPIO39 | 1kΩ pull-up | Variable resistance sender |
| Alternator Charge | GPIO22 | 10kΩ divider + 10kΩ pull-up | 12V→3.3V, Active LOW |
| Engine Running | GPIO26 | 10kΩ divider (2x 10kΩ) | 12V→3.3V, Active HIGH |
| Wake Button | GPIO0 | Internal pull-up | BOOT button or external |
| Sleep Enable | GPIO12 | Internal pull-up | Optional external control |

---

## Important Safety Notes

⚠️ **CRITICAL SAFETY WARNINGS:**

1. **Voltage Protection**: Never apply more than 3.6V to ESP32 GPIO pins
2. **Current Limiting**: Use appropriate resistors to limit current
3. **Isolation**: Consider optocouplers for high-voltage signals
4. **Grounding**: Ensure proper common ground between ESP32 and vehicle
5. **Fusing**: Use appropriate fuses on all 12V connections
6. **ESD Protection**: Handle ESP32 with anti-static precautions

## Testing Without Sensors

The current firmware includes development overrides:
- `isEngineRunning()` returns `false` (no sensors needed for testing)
- All sensor readings will show default/safe values
- Sleep mode and web interface work without any sensors connected

## Enabling Real Sensors

To enable real sensor readings, edit `src/hardware.cpp`:
```cpp
// Comment out this line:
// return false; // Override: assume engine OFF when no sensors connected

// Uncomment this line:
return digitalRead(ENGINE_RUN_FEEDBACK_PIN) == HIGH;
```

---

*Last Updated: August 6, 2025*  
*Reference: Bobcat Ignition Controller Project*
