 # Bobcat Ignition Controller - Detailed Wiring Schematics

## 1. Oil Pressure Sensor (GPIO34) - 5V to 3.3V Conversion

```
    12V Vehicle Power
         │
         ├─────┐
         │     │
    ┌────▼────┐│
    │ Oil     ││
    │Pressure ││
    │ Sensor  ││ 0-5V Output
    │ (0-5V)  ││
    └────┬────┘│
         │     │
      Vehicle  │
       GND ────┘
         │
    ┌────▼────────────────┐
    │                     │
    │     2.2kΩ           │
    ├─────[R1]─────┬──────┤ GPIO34 (ESP32)
    │              │      │
    │              │      │
    │           3.3kΩ     │
    │              │      │
    │             [R2]    │
    │              │      │
    │              │      │
    └──────────────┴──────┤ GND (ESP32)
                          │
                          │
    Optional: 0.1µF ──────┤ (Noise Filter)
    Capacitor             │
                          │
                     ESP32 GND
```

**Component Values:**
- R1: 2.2kΩ (Series resistor)
- R2: 3.3kΩ (Pull-down resistor)
- C1: 0.1µF ceramic capacitor (optional noise filter)

**Voltage Calculation:**
- Vout = Vin × (R2 ÷ (R1 + R2))
- Vout = 5V × (3.3kΩ ÷ 5.5kΩ) = 3.0V (safe for ESP32)

---

## 2. Battery Voltage Monitor (GPIO36) - 12V to 3.3V Conversion

```
    12V Battery Monitor
         │
         │
    ┌────▼────┐
    │ Battery │
    │ Monitor │
    │ 12V +   │
    └────┬────┘
         │
         │
         │ 22kΩ
    ┌────[R1]────┬─────┐ GPIO36 (ESP32)
    │            │     │
    │            │     │
    │         10kΩ     │
    │            │     │
    │           [R2]   │
    │            │     │
    │            │     │
    └────────────┴─────┤ GND (ESP32)
                       │
                       │
    Optional: 0.1µF ───┤ (Noise Filter)
    Capacitor          │
                       │
                  ESP32 GND
```

**Component Values:**
- R1: 22kΩ (High-side series resistor)
- R2: 10kΩ (Pull-down resistor)
- C1: 0.1µF ceramic capacitor (optional)

**Voltage Calculation:**
- Vout = Vin × (R2 ÷ (R1 + R2))
- Vout = 12V × (10kΩ ÷ 32kΩ) = 3.75V
- **Note**: For safer operation, use 33kΩ + 10kΩ for 2.8V output

---

## 3. Coolant Temperature Sensor (GPIO39) - NTC Thermistor

```
    3.3V ESP32 Power
         │
         │
    ┌────▼────┐
    │   NTC   │
    │Thermistor│ (Temperature varies resistance)
    │ Sensor  │ (Typical: 2.5kΩ @ 25°C)
    └────┬────┘
         │
         │
         ├─────────────┐ GPIO39 (ESP32)
         │             │
         │             │
         │ 10kΩ        │
         │             │
        [R1]           │
         │             │
         │             │
         └─────────────┤ GND (ESP32)
                       │
                       │
    Optional: 0.1µF ───┤ (Noise Filter)
    Capacitor          │
                       │
                  ESP32 GND
```

**Component Values:**
- R1: 10kΩ (Pull-down resistor)
- NTC: Automotive coolant temperature sensor (typically 2.5kΩ @ 25°C)
- C1: 0.1µF ceramic capacitor (optional)

**Operation:**
- Hot engine → Low resistance → Higher voltage on GPIO39
- Cold engine → High resistance → Lower voltage on GPIO39

---

## 4. Fuel Level Sensor (GPIO35) - Variable Resistance Sender

```
    12V Vehicle Power
         │
         │ 1kΩ
    ┌────[R1]────┬─────┐ GPIO35 (ESP32)
    │            │     │
    │            │     │
    │        ┌───▼───┐ │
    │        │ Fuel  │ │
    │        │ Level │ │
    │        │Sender │ │ (10Ω Empty to 180Ω Full)
    │        │(10-   │ │
    │        │180Ω)  │ │
    │        └───┬───┘ │
    │            │     │
    │            │     │
    └────────────┴─────┤ GND (ESP32)
                       │
                       │
    Optional: 0.1µF ───┤ (Noise Filter)
    Capacitor          │
                       │
                  ESP32 GND
```

**Component Values:**
- R1: 1kΩ (Pull-up resistor)
- Fuel Sender: Variable resistor (10Ω empty, 180Ω full - typical)
- C1: 0.1µF ceramic capacitor (optional)

**Operation:**
- Empty tank (10Ω) → Low voltage (~0.1V)
- Full tank (180Ω) → Higher voltage (~1.8V)

---

## 5. Engine Running Feedback (GPIO26) - 12V Digital Signal

```
    12V Engine Run Signal
    (from alternator/ECU)
         │
         │
         │ 10kΩ
    ┌────[R1]────┬─────┐ GPIO26 (ESP32)
    │            │     │
    │            │     │
    │         10kΩ     │
    │            │     │
    │           [R2]   │
    │            │     │
    │            │     │
    └────────────┴─────┤ GND (ESP32)
                       │
                       │
    Optional: 0.1µF ───┤ (Noise Filter)
    Capacitor          │
                       │
                  ESP32 GND
```

**Component Values:**
- R1: 10kΩ (Series resistor)
- R2: 10kΩ (Pull-down resistor)
- C1: 0.1µF ceramic capacitor (optional)

**Logic:**
- 12V input → ~1.65V output → HIGH (Engine Running)
- 0V input → 0V output → LOW (Engine Stopped)

---

## 6. Alternator Charge Indicator (GPIO22) - Active LOW Signal

```
    Alternator "L" Terminal
    (12V when NOT charging)
         │
         │
         │ 10kΩ
    ┌────[R1]────┬─────┐ GPIO22 (ESP32)
    │            │     │
    │            ├─────┤ 3.3V ESP32
    │         10kΩ     │
    │            │     │
    │           [R2]   │
    │            │     │
    │            │     │
    └────────────┴─────┤ GND (ESP32)
                       │
                       │
    Optional: 0.1µF ───┤ (Noise Filter)
    Capacitor          │
                       │
                  ESP32 GND
```

**Component Values:**
- R1: 10kΩ (Series resistor)
- R2: 10kΩ (Pull-up to 3.3V)
- C1: 0.1µF ceramic capacitor (optional)

**Logic:**
- 0V input (charging) → HIGH output (R2 pulls up)
- 12V input (not charging) → LOW output (voltage divider)

---

## 7. Wake-Up Button (GPIO0) - Simple Digital Input

```
                    3.3V ESP32
                         │
                         │ Internal
                         │ Pull-up
                    ┌────[R]────┐
                    │           │
    ┌───────────────┤           ├─────┐ GPIO0 (ESP32)
    │               │           │     │ (BOOT Button)
    │          ┌────▼────┐      │     │
    │          │ BOOT    │      │     │
    │          │ Button  │      │     │
    │          │         │      │     │
    │          └────┬────┘      │     │
    │               │           │     │
    └───────────────┼───────────┴─────┤ GND (ESP32)
                    │                 │
                    │                 │
              ESP32 Internal      ESP32 GND
               Pull-up
```

**Component Values:**
- Uses ESP32 internal pull-up resistor (no external components needed)
- Button connects GPIO0 to GND when pressed

**Operation:**
- Button released → HIGH (pull-up active)
- Button pressed → LOW (connects to GND)
- LOW signal wakes ESP32 from deep sleep

---

## Complete System Schematic Overview

```
                    ┌─────────────────────────────────┐
                    │            ESP32                │
                    │                                 │
    Oil Pressure ───┤ GPIO34  (ADC1_CH6)            │
    (0-5V)          │  via voltage divider            │
                    │                                 │
    Battery Voltage ┤ GPIO36  (ADC1_CH0)            │
    (12V)           │  via voltage divider            │
                    │                                 │
    Coolant Temp ───┤ GPIO39  (ADC1_CH3)            │
    (NTC)           │  via pull-down resistor         │
                    │                                 │
    Fuel Level ─────┤ GPIO35  (ADC1_CH7)            │
    (10-180Ω)       │  via pull-up resistor           │
                    │                                 │
    Engine Run ─────┤ GPIO26                          │
    (12V Digital)   │  via voltage divider            │
                    │                                 │
    Alternator ─────┤ GPIO22                          │
    Charge (12V)    │  via voltage divider + pull-up  │
                    │                                 │
    Wake Button ────┤ GPIO0   (BOOT)                 │
    (Active LOW)    │  internal pull-up               │
                    │                                 │
                    │ GPIO21 ──── Main Power Relay   │
                    │ GPIO19 ──── Glow Plug Relay    │
                    │ GPIO18 ──── Starter Relay      │
                    │ GPIO5  ──── Work Lights Relay  │
                    │                                 │
                    │ 3.3V, 5V, GND ──── Power       │
                    └─────────────────────────────────┘
```

---

## Safety and Installation Notes

### ⚠️ Critical Safety Points:

1. **Never exceed 3.6V on any GPIO pin**
2. **Always use fuses on 12V connections**
3. **Ensure proper grounding between vehicle and ESP32**
4. **Test voltage levels with multimeter before connecting**
5. **Use heat shrink tubing on all connections**
6. **Mount ESP32 in weatherproof enclosure**

### 🔧 Installation Tips:

1. **Start with one sensor at a time**
2. **Test each connection with multimeter**
3. **Use breadboard for initial testing**
4. **Solder connections for permanent installation**
5. **Route wires away from high-current cables**
6. **Use twisted pair for analog signals**

---

*Schematic Reference: Bobcat Ignition Controller v1.0*  
*Last Updated: August 6, 2025*
