# ESP32 Board Development - Pin-by-Pin Analysis

## Project: Bobcat Ignition Controller PCB Design

### **Bobcat 743 Engine Specifications**

**Target Engine:** Kubota V1702-BA 4-cylinder diesel

- **Power:** 36 HP (26.8 kW) @ 2800 RPM
- **Displacement:** 1.73L (105.7 cu.in)
- **Fuel Tank:** 49L (13 gal)
- **Oil Capacity:** 8.5L (9 qts)
- **Cooling:** 12.3L (13 qts) liquid-cooled
- **Electrical:** 12V system, 55A alternator
- **Operating Temperature:** Typical diesel range -40°C to 120°C
- **Oil Pressure:** Full pressure lubrication system
- **Engine Type:** Naturally aspirated, mechanical injection

### **Pin Assignment Summary**

| GPIO | Function | Type | Range/Logic | ADC Channel | Notes |
|------|----------|------|-------------|-------------|-------|
| 21 | Glow Plugs | Output | Active HIGH | - | 40A Relay Control |
| 22 | Starter | Output | Active HIGH | - | 100A Relay Control |
| 23 | Ignition | Output | Active HIGH | - | 30A Relay Control |
| 27 | Alternator Charge | Input | Active LOW | - | Digital Status |
| 14 | Engine Run Feedback | Input | Active HIGH | - | Digital Status |
| 36 | Engine Temperature | Input | -40°C to 150°C | ADC1_CH0 | NTC Thermistor |
| 39 | Oil Pressure | Input | 0-689 kPa | ADC1_CH3 | Resistive Sender |
| 34 | Battery Voltage | Input | 0-30V | ADC1_CH6 | Voltage Divider |
| 35 | Fuel Level | Input | 0-100% | ADC1_CH7 | Float Sender |

---

## **GPIO21 - Glow Plugs Relay** ⚡ **CRITICAL**

### **Function & Requirements**

- **Purpose:** Control glow plug preheating for diesel engine cold start
- **Load:** 40A automotive relay coil (12V/24V compatible)
- **Timing:** 20-second preheat cycle before starter engagement
- **Safety:** Critical for cold weather starting (-40°C capability)
- **Logic:** Active HIGH output

### **Kubota V1702-BA Glow Plug Specifications**

- **Type:** Fast-heat ceramic glow plugs
- **Voltage:** 12V system (compatible with 24V via relay)
- **Current:** ~15A per glow plug (4 cylinders = 60A total)
- **Preheat Time:** 15-20 seconds @ -40°C, 5-10 seconds @ 0°C
- **Control:** Normally controlled by engine ECU or manual timer

### **Circuit Design**

```
GPIO21 → [1kΩ] → MOSFET Gate (IRLZ44N)
                      ↓
                 Relay Coil (12V, 200mA)
                      ↓
                 Flyback Diode (1N4007)
                      ↓
                    Ground
```

### **Component Specifications**

- **MOSFET:** IRLZ44N (Logic Level, 55V, 47A, 22mΩ Rds(on))
- **Gate Resistor:** 1kΩ (1%, 1206 package)
- **Flyback Diode:** 1N4007 (1000V, 1A, fast recovery)
- **Relay:** Bosch 40A automotive relay (12V coil, SPST-NO)
- **Status LED:** Red LED + 470Ω resistor
- **Protection:** 3.3V Zener diode on GPIO

### **PCB Layout Requirements**

- **Trace Width:** 50mil minimum for relay coil current
- **Via Stitching:** Multiple vias for heat dissipation
- **Component Placement:** MOSFET close to relay socket
- **Heat Sinking:** Thermal pad for MOSFET
- **Test Points:** Gate, drain, source accessible

---

## **GPIO22 - Starter Relay** ⚡ **CRITICAL**

### **Function & Requirements**

- **Purpose:** Control starter solenoid for engine cranking
- **Load:** 100A automotive relay coil (highest current in system)
- **Duration:** Short duty cycle (10 seconds maximum)
- **Safety:** Engine must be stopped before starter engagement
- **Logic:** Active HIGH output with safety interlocks

### **Kubota V1702-BA Starter Specifications**

- **Type:** 12V electric starter motor
- **Current:** 200-300A cranking current (through relay contacts)
- **Coil Current:** 300-500mA for relay activation
- **Safety:** Oil pressure and temperature interlocks required
- **Timing:** Maximum 10-second engagement cycles

### **Circuit Design**

```
GPIO22 → [1kΩ] → High-Current MOSFET (IRLB8721)
                      ↓
                 100A Relay Coil (12V, 500mA)
                      ↓
                 Heavy Flyback Diode (1N5819)
                      ↓
                    Ground
```

### **Component Specifications**

- **MOSFET:** IRLB8721 (Logic Level, 30V, 62A, 6.2mΩ Rds(on))
- **Gate Resistor:** 1kΩ (1%, 1206 package)
- **Flyback Diode:** 1N5819 Schottky (40V, 1A, fast switching)
- **Relay:** Heavy-duty 100A automotive relay (12V coil, SPST-NO)
- **Status LED:** Red LED + 470Ω resistor
- **Heat Sink:** TO-220 heat sink for MOSFET

### **PCB Layout Requirements**

- **Trace Width:** 100mil minimum for relay coil current
- **Copper Weight:** 2oz copper minimum
- **Via Stitching:** Extensive thermal vias
- **Component Placement:** Isolated from sensitive circuits
- **Heat Management:** Thermal relief on ground plane

---

## **GPIO23 - Ignition Relay** ⚡ **CRITICAL**

### **Function & Requirements**

- **Purpose:** Control main ignition/run relay for engine operation
- **Load:** 30A automotive relay coil
- **Duration:** Continuous operation while engine running
- **Safety:** Automatic shutoff on low oil pressure or high temperature
- **Logic:** Active HIGH output

### **Kubota V1702-BA Ignition System**

- **Type:** Mechanical injection pump (no electronic ignition)
- **Control:** Fuel solenoid shutoff valve
- **Current:** 2-5A continuous for fuel solenoid
- **Safety:** Engine stops immediately when relay opens
- **Monitoring:** Engine run feedback required

### **Circuit Design**

```
GPIO23 → [1kΩ] → MOSFET Gate (IRLZ44N)
                      ↓
                 Relay Coil (12V, 200mA)
                      ↓
                 Flyback Diode (1N4007)
                      ↓
                    Ground
```

### **Component Specifications**

- **MOSFET:** IRLZ44N (Logic Level, 55V, 47A, 22mΩ Rds(on))
- **Gate Resistor:** 1kΩ (1%, 1206 package)
- **Flyback Diode:** 1N4007 (1000V, 1A)
- **Relay:** 30A automotive relay (12V coil, SPST-NO)
- **Status LED:** Green LED + 470Ω resistor
- **Protection:** 3.3V Zener diode on GPIO

### **PCB Layout Requirements**

- **Trace Width:** 30mil minimum for relay coil current
- **Component Placement:** Central location for easy access
- **Status Indicators:** Visible LED placement
- **Test Points:** All critical signals accessible

---

## **GPIO27 - Alternator Charge Status** ⚡

### **Function & Requirements**

- **Purpose:** Monitor alternator charge indicator light
- **Signal:** 12V charge light circuit (Active LOW when charging)
- **Logic:** LOW = charging, HIGH = not charging
- **Monitoring:** Engine run detection and battery charging status

### **Kubota V1702-BA Alternator Specifications**

- **Type:** 12V, 55A automotive alternator
- **Charge Light:** Standard automotive circuit
- **W Terminal:** AC output proportional to RPM (future RPM sensing)
- **Regulation:** Internal voltage regulator (14.4V typical)

### **Circuit Design**

```
Charge Light Signal → [47kΩ] → [10kΩ] → GPIO27
    (12V)                 ↓        ↓
                     Voltage   [100nF] → GND
                     Divider   Filter
```

### **Component Specifications**

- **R1:** 47kΩ (1%, 1206 package) - Voltage divider high side
- **R2:** 10kΩ (1%, 1206 package) - Voltage divider low side
- **C1:** 100nF ceramic (X7R, 50V) - Noise filtering
- **D1:** 3.3V Zener diode - Overvoltage protection
- **Pull-up:** Internal ESP32 pull-up enabled

### **Signal Characteristics**

- **Input Voltage:** 0-12V (automotive charge light circuit)
- **Output Voltage:** 0-2.1V (safe for ESP32 3.3V logic)
- **Logic Levels:** HIGH = 2.1V (not charging), LOW = 0V (charging)
- **Filtering:** 100nF for automotive noise immunity

---

## **GPIO14 - Engine Run Feedback** 🏃

### **Function & Requirements**

- **Purpose:** Detect engine running state for safety interlocks
- **Signal:** Digital feedback from ignition system or oil pressure switch
- **Logic:** HIGH = engine running, LOW = engine stopped
- **Safety:** Used for starter interlock and safety shutdowns

### **Signal Sources (Options)**

1. **Oil Pressure Switch:** Opens when engine running (oil pressure > 0.7 bar)
2. **Alternator W Terminal:** AC signal presence indicates engine running
3. **Fuel Pump Current:** Current sensor on fuel pump circuit
4. **Vibration Sensor:** Accelerometer detecting engine vibration

### **Circuit Design**

```
Engine Run Signal → [10kΩ] → GPIO14
    (12V)              ↓
                   [100nF] → GND
                   Filter
```

### **Component Specifications**

- **R1:** 10kΩ (1%, 1206 package) - Current limiting
- **C1:** 100nF ceramic (X7R, 50V) - Debouncing filter
- **Pull-up:** Internal ESP32 pull-up enabled
- **Protection:** 3.3V Zener diode for overvoltage

### **Signal Characteristics**

- **Input Voltage:** 0-12V (automotive signal)
- **Logic Levels:** HIGH = engine running, LOW = engine stopped
- **Debouncing:** Software debouncing + hardware filtering
- **Response Time:** <100ms for safety-critical functions

---

## **GPIO36 - Engine Temperature** 🌡️

### **Function & Requirements**

- **Purpose:** Monitor engine coolant temperature
- **Sensor:** NTC thermistor (Negative Temperature Coefficient)
- **Range:** -40°C to 150°C operating range
- **Resolution:** 0.1°C for precise temperature monitoring
- **Safety:** Overheat protection and warning thresholds

### **Kubota V1702-BA Temperature Sensor**

- **Type:** 2-wire NTC thermistor sender
- **Resistance:** 2.2kΩ @ 25°C (typical automotive)
- **Beta Value:** 3950K (temperature coefficient)
- **Location:** Engine block or thermostat housing
- **Thread:** M14 x 1.5 or M12 x 1.5 (verify on actual engine)

### **Resistance vs Temperature Curve**

| Temperature (°C) | Resistance (Ω) | ADC Reading (12-bit) |
|------------------|----------------|----------------------|
| -40 | 28,680 | 3892 |
| 0 | 6,530 | 3204 |
| 25 | 2,200 | 2275 |
| 80 | 326 | 819 |
| 100 | 177 | 506 |
| 120 | 105 | 324 |
| 150 | 52 | 174 |

### **Circuit Design**

```
                    +5V
                     |
                   [4.7kΩ]     ← Pull-up/Reference
                     |
    To ESP32 -------|-------[100Ω]---- GPIO36
    (ADC1_CH0)       |                  ↑
                     |              Protection
                   [NTC]         & Current Limit
               [2.2kΩ @ 25°C]
                     |
                   [1kΩ]         ← Linearization
                     |
                    GND
```

### **Component Specifications**

- **R1:** 4.7kΩ precision (1%, 1206) - Reference resistor
- **R2:** 100Ω (5%, 1206) - Protection resistor
- **R3:** 1kΩ (5%, 1206) - Linearization resistor
- **C1:** 100nF ceramic (X7R, 50V) - Noise filtering
- **C2:** 1µF ceramic (X7R, 25V) - Low frequency filtering
- **Trimmer:** 10kΩ multi-turn for calibration

### **Calibration & Software**

- **Lookup Table:** Store resistance-to-temperature conversion
- **Steinhart-Hart Equation:** For precise temperature calculation
- **Averaging:** 10-sample rolling average for stability
- **Thresholds:** Warning @ 100°C, critical @ 110°C, shutdown @ 115°C

---

## **GPIO39 - Oil Pressure** 💧

### **Function & Requirements**

- **Purpose:** Monitor engine oil pressure for lubrication safety
- **Sensor:** Resistive oil pressure sender
- **Range:** 0-689 kPa (0-100 psi) typical diesel engine range
- **Resolution:** 1 kPa (0.15 psi) for accurate monitoring
- **Safety:** Low pressure warning and engine shutdown protection

### **Kubota V1702-BA Oil Pressure System**

- **Type:** Full pressure lubrication system
- **Normal Pressure:** 275-415 kPa (40-60 psi) @ 2000 RPM
- **Low Pressure Warning:** < 140 kPa (20 psi)
- **Critical Shutdown:** < 70 kPa (10 psi)
- **Sender Type:** Variable resistance, 0-10 bar range

### **Pressure Sender Characteristics**

| Pressure (kPa) | Pressure (psi) | Resistance (Ω) | ADC Reading (12-bit) |
|----------------|----------------|----------------|----------------------|
| 0 | 0 | 180 | 434 |
| 140 | 20 | 142 | 560 |
| 275 | 40 | 90 | 842 |
| 415 | 60 | 55 | 1318 |
| 550 | 80 | 32 | 1876 |
| 689 | 100 | 10 | 2925 |

### **Circuit Design**

```
                    +5V
                     |
                   [2.2kΩ]     ← Reference resistor
                     |
    To ESP32 -------|-------[220Ω]---- GPIO39
    (ADC1_CH3)       |                  ↑
                     |              Protection
                 [Oil Sender]     & Current Limit
                 (10-180Ω)
                     |
                    GND
```

### **Component Specifications**

- **R1:** 2.2kΩ precision (1%, 1206) - Reference resistor
- **R2:** 220Ω (5%, 1206) - Protection resistor
- **C1:** 10µF tantalum (25V) - Low frequency filtering
- **C2:** 100nF ceramic (X7R, 50V) - High frequency filtering
- **D1:** 3.3V Zener diode - Overvoltage protection
- **Trimmer:** 10kΩ multi-turn for full-scale adjustment

### **Signal Processing**

- **Conversion:** Resistance to pressure using lookup table
- **Filtering:** Heavy filtering due to engine vibration and pulsation
- **Averaging:** 20-sample rolling average for stability
- **Thresholds:** Warning @ 140 kPa, critical @ 70 kPa

---

## **GPIO34 - Battery Voltage** 🔋

### **Function & Requirements**

- **Purpose:** Monitor battery voltage for charging system health
- **Range:** 8-30V (covers 12V and 24V systems)
- **Resolution:** 0.1V for accurate voltage monitoring
- **Protection:** Overvoltage protection for automotive transients
- **Compatibility:** Works with both 12V and 24V Bobcat variants

### **Kubota V1702-BA Electrical System**

- **Battery:** 12V system standard
- **Charging Voltage:** 13.8-14.4V (alternator regulated)
- **Low Voltage:** < 11.5V (battery discharge warning)
- **High Voltage:** > 15.5V (overcharge protection)
- **Transients:** Automotive transients up to 40V possible

### **Voltage Ranges & Thresholds**

| Condition | 12V System | 24V System | Action |
|-----------|------------|------------|--------|
| Critical Low | < 10.5V | < 21.0V | Engine shutdown |
| Low Warning | < 11.5V | < 23.0V | Warning indicator |
| Normal | 11.5-14.5V | 23.0-29.0V | Normal operation |
| High Warning | > 14.5V | > 29.0V | Charging system warning |
| Critical High | > 16.0V | > 32.0V | System protection |

### **Circuit Design**

```
    +12V/24V ----[100kΩ]----+----[10kΩ]---- GPIO34
                             |
                        [100nF] + [3.3V Zener]
                             |      |
                            GND    GND
```

### **Voltage Divider Calculation**

- **Divider Ratio:** 10kΩ/(100kΩ + 10kΩ) = 1/11 = 0.0909
- **12V Input:** 12V × 0.0909 = 1.09V
- **24V Input:** 24V × 0.0909 = 2.18V
- **30V Input:** 30V × 0.0909 = 2.73V (safe for ESP32)
- **40V Transient:** 40V × 0.0909 = 3.64V (protected by Zener)

### **Component Specifications**

- **R1:** 100kΩ (1%, 1206) - High voltage divider resistor
- **R2:** 10kΩ (1%, 1206) - Low voltage divider resistor
- **C1:** 100nF ceramic (X7R, 50V) - Noise filtering
- **D1:** 3.3V Zener diode (500mW) - Overvoltage protection
- **D2:** Schottky diode (40V, 1A) - Reverse polarity protection

### **Software Calibration**

- **ADC Resolution:** 12-bit (4096 counts)
- **Voltage per Count:** 3.3V / 4096 = 0.806mV per count
- **Battery Voltage:** ADC_Reading × 0.806mV × 11 = actual voltage
- **Averaging:** 50-sample rolling average for stability

---

## **GPIO35 - Fuel Level** ⛽

### **Function & Requirements**

- **Purpose:** Monitor fuel tank level for range estimation
- **Sensor:** Variable resistance float sender
- **Range:** 0-100% fuel level (49L tank capacity)
- **Resolution:** 1% fuel level for accurate monitoring
- **Calibration:** Non-linear tank shape requires lookup table

### **Bobcat 743 Fuel System**

- **Tank Capacity:** 49L (13 gallons)
- **Tank Shape:** Irregular shape affects sensor linearity
- **Sender Type:** Float-operated variable resistor
- **Common Ranges:** 0-90Ω, 33-240Ω, or 240-33Ω (depends on OEM)
- **Location:** Tank-mounted sender unit

### **Fuel Sender Characteristics**

| Fuel Level (%) | Tank Volume (L) | Resistance (Ω) | ADC Reading (12-bit) |
|----------------|-----------------|----------------|----------------------|
| 0 (Empty) | 0 | 240 | 341 |
| 12.5 (1/8) | 6.1 | 180 | 434 |
| 25 (1/4) | 12.25 | 120 | 650 |
| 50 (1/2) | 24.5 | 90 | 842 |
| 75 (3/4) | 36.75 | 60 | 1241 |
| 100 (Full) | 49 | 33 | 1876 |

### **Circuit Design**

```
                    +5V
                     |
                   [240Ω]      ← Match sender range
                     |
    To ESP32 -------|-------[100Ω]---- GPIO35
    (ADC1_CH7)       |                  ↑
                     |              Protection
                [Fuel Sender]
                 (33-240Ω)
                     |
                    GND
```

### **Component Specifications**

- **R1:** 240Ω precision (1%, 1206) - Matches full-scale sender
- **R2:** 100Ω (5%, 1206) - Protection resistor
- **C1:** 1µF ceramic (X7R, 25V) - Float movement filtering
- **C2:** 100nF ceramic (X7R, 50V) - High frequency filtering
- **Trimmer:** 1kΩ multi-turn for sender range matching

### **Signal Processing & Calibration**

- **Lookup Table:** Resistance-to-fuel-level conversion
- **Tank Compensation:** Non-linear tank shape correction
- **Filtering:** Heavy filtering due to vehicle movement and float oscillation
- **Averaging:** 100-sample rolling average for stability
- **Low Fuel Warning:** Alert at 10% (4.9L remaining)

---

## **Power Supply Design**

### **Input Power Requirements**

- **Input Voltage:** 12V/24V automotive (8-30V operating range)
- **Input Current:** 500mA maximum (normal operation)
- **Transient Protection:** 40V automotive transients
- **Reverse Polarity:** Full protection required
- **EMI Filtering:** Automotive-grade input filtering

### **Power Rail Design**

```text
12V/24V Input → Fuse → Reverse Protection → Buck Converter → 5V Rail
                                                         ↓
                                           LDO Regulator → 3.3V Rail
```

### **Required Power Rails**

- **12V/24V Raw:** Relay coils, automotive sensors (200mA)
- **5V Regulated:** Sensor excitation, signal conditioning (100mA)
- **3.3V Regulated:** ESP32, logic levels (150mA)

### **Component Specifications**

- **Input Fuse:** 2A automotive blade fuse
- **TVS Diode:** 36V bidirectional TVS (automotive transient protection)
- **Reverse Polarity:** P-channel MOSFET (automatic)
- **Buck Converter:** MP2307 (24V input, 5V/1A output)
- **LDO Regulator:** AMS1117-3.3 (5V to 3.3V, 1A)

---

## **PCB Layout Guidelines**

### **Layer Stack-up (4-layer)**

1. **Top Layer:** Components, fine signal traces
2. **Ground Plane:** Solid ground pour with analog/digital separation
3. **Power Plane:** 3.3V and 5V distribution
4. **Bottom Layer:** Power traces, high-current connections

### **Trace Width Guidelines**

| Current | Minimum Width | Recommended | Copper Weight |
|---------|---------------|-------------|---------------|
| 100mA | 4mil | 8mil | 1oz |
| 500mA | 12mil | 20mil | 1oz |
| 1A | 20mil | 30mil | 1oz |
| 5A | 50mil | 80mil | 2oz |
| 10A+ | 100mil | 150mil | 2oz |

### **Critical Design Rules**

1. **Analog/Digital Separation:** Separate ground planes connected at single point
2. **Power Supply Isolation:** Switching regulator away from analog circuits
3. **Crystal Placement:** Short traces with ground guard rings
4. **High Current Routing:** Heavy copper traces with thermal vias
5. **Test Points:** All critical signals accessible for debugging
6. **Component Orientation:** All polarized components clearly marked

---

## **Environmental & Regulatory Requirements**

### **Automotive Operating Conditions**

- **Temperature Range:** -40°C to +85°C (extended automotive)
- **Humidity:** 0-95% non-condensing with salt spray resistance
- **Vibration:** 20G @ 10-2000Hz (engine compartment mounting)
- **Shock:** 50G, 11ms duration (road shock and impact)
- **EMI/EMC:** CISPR-25 automotive EMC compliance
- **Ingress Protection:** IP67 sealing (dust and water protection)

### **Component Selection Criteria**

- **Automotive Grade:** AEC-Q100 qualified components where available
- **Extended Temperature:** -40°C to +125°C for critical components
- **Vibration Resistant:** Components rated for automotive vibration
- **Conformal Coating:** PCB protection against moisture and contaminants
- **Connector Sealing:** IP67 rated automotive connectors

---

## **Cost Analysis & BOM**

### **Component Costs (Quantity 10)**

| Category | Cost per Board | Components | Notes |
|----------|---------------|------------|-------|
| ESP32 Module | $8.00 | ESP32-WROOM-32D | Main controller |
| Passive Components | $5.00 | Resistors, capacitors | 1% precision |
| Protection Circuits | $12.00 | Zeners, TVS, fuses | Automotive grade |
| Power MOSFETs | $6.00 | Relay drivers | Logic level |
| Automotive Relays | $25.00 | 3× relays + sockets | 40A/30A/100A |
| Connectors | $15.00 | IP67 automotive | Sealed connectors |
| PCB Fabrication | $20.00 | 4-layer, 2oz copper | Professional fab |
| Assembly & Test | $15.00 | Hand assembly | Initial prototypes |
| **Total** | **$106.00** | **Target Achieved** | <$150 goal |

### **Open-Source Project Resources**

Based on research of similar projects:

- **ieb/EngineMonitor:** Marine diesel monitoring with NMEA2000
- **MohamedElalawy/gsm-esp32-diesel-engine-monitor:** GSM telemetry system
- **Common Best Practices:** External ADC, heavy filtering, automotive protection

---

**STATUS: Complete pin-by-pin analysis ready for schematic design**

**NEXT STEPS: See TODO.md for detailed action plan**
