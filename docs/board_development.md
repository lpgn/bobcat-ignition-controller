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

### Current Pin Assignments Summary

#### **Digital Output Pins (Relay Control)**
| GPIO | Function | Load Type | Current | Logic | Notes |
|------|----------|-----------|---------|-------|-------|
| 21 | Glow Plugs | 40A Relay | High | Active HIGH | Critical timing - 20s preheat |
| 23 | Ignition | 30A Relay | Medium | Active HIGH | Main run control |
| 22 | Starter | 100A Relay | Very High | Active HIGH | Short duration, high current |

#### **Analog Input Pins (ADC1 - WiFi Compatible)**
| GPIO | ADC Ch | Function | Range | Signal | Impedance |
|------|--------|----------|-------|--------|-----------|
| 36 | ADC1_CH0 | Engine Temp | -40°C to 150°C | 0-5V | High Z |
| 39 | ADC1_CH3 | Oil Pressure | 0-689 kPa | 0-5V | High Z |
| 34 | ADC1_CH6 | Battery Voltage | 0-30V | 0-3.3V | Voltage divider |
| 35 | ADC1_CH7 | Fuel Level | 0-100% | 0-5V | High Z |

#### **Digital Input Pins**
| GPIO | Function | Type | Logic | Pull | Notes |
|------|----------|------|-------|------|-------|
| 27 | Alternator Charge | Status | Active LOW | External | Charge indicator |
| 14 | Engine Run Feedback | Status | Active HIGH | Internal PULLUP | Engine running detect |

---

## Pin-by-Pin Board Design Requirements

### **Power Supply Design**
```
12V/24V Input → Fuse → Reverse Protection → Buck Converter → 5V Rail
                                                         ↓
                                           LDO Regulator → 3.3V Rail
```

**Required Rails:**
- **12V/24V**: Relay coils, automotive sensors
- **5V**: Sensor excitation, signal conditioning
- **3.3V**: ESP32, logic levels

### **GPIO21 - Glow Plugs Relay** ⚡ **CRITICAL**
**Requirements:**
- Drive 40A automotive relay
- Flyback diode protection
- LED status indicator
- Overcurrent protection

**Circuit Design:**
```
GPIO21 → 1kΩ → MOSFET Gate (IRLZ44N)
              ↓
         Relay Coil (12V/24V)
              ↓
         Flyback Diode (1N4007)
              ↓
            Ground
```

**PCB Considerations:**
- Use heavy copper traces (50mil minimum)
- Place relay close to MOSFET
- Add test point for diagnostics
- Include manual override jumper

### **GPIO23 - Ignition Relay** ⚡ **CRITICAL**
**Requirements:**
- Drive 30A automotive relay
- Similar protection as glow plugs
- Status LED

**Circuit Design:** Same as GPIO21 but 30A relay

### **GPIO22 - Starter Relay** ⚡ **CRITICAL**
**Requirements:**
- Drive 100A automotive relay
- Heavy duty protection
- Short duty cycle (10s max)
- Emergency cutoff capability

**Circuit Design:**
```
GPIO22 → 1kΩ → High-current MOSFET (IRLB8721)
              ↓
         100A Relay Coil
              ↓
         Heavy Flyback Diode
              ↓
            Ground
```

**PCB Considerations:**
- Use maximum copper thickness
- Wide traces (100mil minimum)
- Heat sinking for MOSFET
- Separate ground plane section

### **GPIO36 - Engine Temperature** 🌡️
**Requirements:**
- NTC thermistor interface
- Signal conditioning circuit
- Temperature range: -40°C to 150°C
- 0.1°C resolution

**Signal Conditioning Circuit:**
```
NTC Thermistor → Wheatstone Bridge → Op-amp Buffer → Voltage Divider → GPIO36
     ↑                   ↑                ↑               ↑
5V Reference      Reference Resistor    LM358        5V→3.3V
```

**PCB Considerations:**
- Use precision reference resistor (0.1%)
- Shield analog traces
- Place conditioning circuit close to connector
- Add ESD protection

### **GPIO39 - Oil Pressure** 💧
**Requirements:**
- 0-689 kPa pressure transducer
- 0-5V signal input
- High input impedance
- Noise filtering

**Circuit Design:**
```
Pressure Sensor → RC Filter → Voltage Divider → GPIO39
                     ↑             ↑
               Low-pass filter   5V→3.3V
```

**Components:**
- RC filter: 1kΩ + 100nF
- Voltage divider: 47kΩ + 47kΩ
- ESD protection diodes

### **GPIO34 - Battery Voltage** 🔋
**Requirements:**
- Monitor 12V/24V battery
- Voltage divider for 0-30V range
- High accuracy (±1%)
- Overvoltage protection

**Circuit Design:**
```
Battery+ → Fuse → 47kΩ → GPIO34 → 4.7kΩ → GND
                    ↑               ↑
              Protection      3.3V Zener
```

**PCB Considerations:**
- Use precision resistors (0.1%)
- Add TVS diode for protection
- Separate analog ground

### **GPIO35 - Fuel Level** ⛽
**Requirements:**
- Resistive fuel sender (0-90Ω typical)
- Ratiometric measurement
- Noise immunity
- Water resistance

**Circuit Design:**
```
Fuel Sender → Pull-up Resistor → GPIO35
      ↑              ↑
   Variable R    Fixed Reference
```

### **GPIO27 - Alternator Charge** ⚡
**Requirements:**
- Monitor charge light signal
- 12V/24V logic level
- Noise filtering
- Isolation

**Circuit Design:**
```
Charge Signal → Voltage Divider → RC Filter → GPIO27
                      ↑              ↑
                  12V→3.3V      Noise filter
```

### **GPIO14 - Engine Run Feedback** 🏃
**Requirements:**
- Detect engine running state
- Digital input with pullup
- Debouncing circuit
- ESD protection

---

## PCB Layout Guidelines

### **Layer Stack-up (4-layer recommended)**
1. **Top Layer**: Components, fine traces
2. **Ground Plane**: Solid ground pour
3. **Power Plane**: 3.3V and 5V distribution
4. **Bottom Layer**: Power traces, larger components

### **Trace Width Guidelines**
| Current | Minimum Width | Recommended |
|---------|---------------|-------------|
| 100mA | 4mil | 8mil |
| 1A | 20mil | 30mil |
| 5A | 50mil | 80mil |
| 10A+ | 100mil | 150mil |

### **Component Placement**
```
[Power Input] → [Protection] → [ESP32] → [Connectors]
      ↓              ↓           ↓           ↓
[Relay Drivers] [Signal Cond.] [Status LEDs] [Sensors]
```

### **Critical Design Rules**
1. **Separate analog and digital grounds**
2. **Use star grounding for sensitive signals**
3. **Keep switching circuits away from analog**
4. **Add test points for all critical signals**
5. **Include programming header (UART)**
6. **Add reset and boot buttons**
7. **Include status LEDs for each function**

---

## Connector Requirements

### **Main Engine Harness (16-pin)**
| Pin | Signal | Wire Gauge | Color |
|-----|--------|------------|-------|
| 1 | +12V/24V Battery | 12 AWG | Red |
| 2 | Ground | 12 AWG | Black |
| 3 | Glow Plugs Out | 10 AWG | Yellow |
| 4 | Ignition Out | 12 AWG | Blue |
| 5 | Starter Out | 8 AWG | Purple |
| 6 | Engine Temp In | 22 AWG | Green |
| 7 | Oil Pressure In | 22 AWG | White |
| 8 | Fuel Level In | 22 AWG | Brown |
| 9 | Alternator Charge | 20 AWG | Orange |
| 10 | Engine Run FB | 20 AWG | Pink |
| 11-16 | Future Expansion | 20 AWG | Gray |

### **Programming/Debug (6-pin)**
| Pin | Signal | Notes |
|-----|--------|-------|
| 1 | 3.3V | Power |
| 2 | GND | Ground |
| 3 | TX | ESP32 TX |
| 4 | RX | ESP32 RX |
| 5 | EN | Reset |
| 6 | BOOT | Boot mode |

---

## Next Steps for Board Development

### **Phase 1: Schematic Design** 📋
- [ ] Complete schematic capture in KiCad/Altium
- [ ] Add all protection circuits
- [ ] Include test points and debug features
- [ ] Design rule check (DRC)

### **Phase 2: PCB Layout** 🎯
- [ ] Component placement optimization
- [ ] Route critical signals first
- [ ] Power plane design
- [ ] EMI/EMC considerations

### **Phase 3: Manufacturing Prep** 🏭
- [ ] Generate Gerber files
- [ ] Create assembly drawings
- [ ] Bill of Materials (BOM)
- [ ] Pick and place files

### **Phase 4: Testing Plan** 🧪
- [ ] Bring-up test procedures
- [ ] Functional test suite
- [ ] Environmental testing
- [ ] Certification requirements

Would you like me to elaborate on any specific pin or start with the schematic design for a particular section?

---

### **Kubota V1702-BA Sensor Specifications (Research-Based)**

#### **Temperature Sensors - Thermistor Type**
- **Type:** NTC Thermistor (Negative Temperature Coefficient)
- **Common Values:** 2.2kΩ @ 25°C (typical automotive)
- **Operating Range:** -40°C to 150°C
- **Beta Value:** ~3950K (typical)
- **Resistance at Key Temps:**
  - 80°C (normal): ~500Ω
  - 100°C (warm): ~250Ω
  - 120°C (hot): ~150Ω

#### **Oil Pressure Sensors - Resistive Type**
- **Common Type:** 0-10 bar (0-145 psi) resistive sender
- **Output:** Variable resistance (typically 10-180Ω)
- **Alternative:** 0.5-4.5V analog output (modern retrofit)
- **Pressure Range:** 0-689 kPa (0-100 psi) for diesel engines
- **Typical Values:**
  - 0 kPa: 180Ω
  - 345 kPa (50 psi): 90Ω
  - 689 kPa (100 psi): 10Ω

#### **Fuel Level Sensors - Float Type**
- **Type:** Variable resistor float sender
- **Range:** 0-90Ω or 240-33Ω (depends on manufacturer)
- **Tank Capacity:** 49L (13 gallons)
- **Common Issue:** Non-linear response curve
- **Calibration:** Requires lookup table

#### **Alternator Monitoring**
- **W Terminal:** AC signal proportional to RPM
- **Frequency:** ~47Hz per 1000 RPM (typical)
- **Voltage:** 2-8V AC (battery voltage dependent)
- **Circuit:** Needs AC coupling and pulse shaping

---

### **Sensor Interface Circuit Designs**

#### **Temperature Sensor Circuit (GPIO36)**

```
                    +5V
                     |
                   [4.7kΩ]     ← Pull-up resistor
                     |
    To ESP32 -------|-------[100Ω]---- GPIO36
    (ADC1_CH0)       |                  ↑
                     |              Protection
                   [NTC]         & Current Limit
                     |
                   [1kΩ]         ← Linearization
                     |
                    GND
```

**Component Values:**
- **R1:** 4.7kΩ precision (1%) - Pull-up
- **R2:** 100Ω - Protection resistor
- **R3:** 1kΩ - Linearization resistor
- **C1:** 100nF ceramic - Noise filtering
- **Provisions:** Trimmer option for calibration

#### **Oil Pressure Sensor Circuit (GPIO39)**

```
                    +5V
                     |
                   [2.2kΩ]     ← Reference resistor
                     |
    To ESP32 -------|-------[220Ω]---- GPIO39
    (ADC1_CH3)       |                  ↑
                     |              Protection
                 [Oil Sender]     & Current Limit
                     |
                    GND
```

**Component Values:**
- **R1:** 2.2kΩ precision (1%) - Reference
- **R2:** 220Ω - Protection resistor
- **C1:** 10µF tantalum + 100nF ceramic - Filtering
- **D1:** 3.3V Zener - Overvoltage protection
- **Provisions:** 10kΩ trimmer for full-scale adjust

#### **Battery Voltage Monitor Circuit (GPIO34)**

```
    +12V/24V ----[47kΩ]----+----[10kΩ]---- GPIO34
                            |
                          [100nF]
                            |
                           GND
```

**Voltage Divider Calculation:**
- **12V Input:** 12V × (10kΩ/(47kΩ+10kΩ)) = 2.1V
- **24V Input:** 24V × (10kΩ/(47kΩ+10kΩ)) = 4.2V ⚠️ **TOO HIGH!**

**CORRECTED Design for 12V/24V:**
- **R1:** 100kΩ (1%, metal film)
- **R2:** 10kΩ (1%, metal film)
- **Max Input:** 36V → 3.27V output (safe)
- **Resolution:** ~11mV per ADC count @ 12-bit

#### **Fuel Level Sensor Circuit (GPIO35)**

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

**Component Values:**
- **R1:** 240Ω precision - Matches full scale
- **R2:** 100Ω - Protection
- **C1:** 1µF + 100nF - Heavy filtering (float movement)
- **Provisions:** 1kΩ trimmer for sender matching

---

### **Open-Source Reference Projects Analysis**

#### **Project 1: ieb/EngineMonitor (Marine Diesel)**
**GitHub:** https://github.com/ieb/EngineMonitor
**Target:** Volvo Penta D2-40F marine diesel
**Key Insights:**
- Uses ADS1115 16-bit ADC instead of ESP32 internal ADC (non-linear)
- Signal conditioning with 74HC14 Schmitt trigger for pulses
- Temperature: 1-wire DS18B20 sensors for remote monitoring
- Oil pressure: Resistive sender with voltage divider
- RPM from W+ alternator terminal with AC coupling
- Power consumption: <50mA @ 5V (excellent for automotive)

**Circuit Highlights:**
- **Zener regulation:** 5V through zener powered by 12V via 470Ω
- **20-turn trimmers:** For precise calibration
- **2N2222 switching:** For alternator signal conditioning
- **BMP280:** Environmental pressure/temperature reference

#### **Project 2: MohamedElalawy/gsm-esp32-diesel-engine-monitor**
**GitHub:** https://github.com/MohamedElalawy/gsm-esp32-diesel-engine-monitor
**Target:** Generic diesel engines with GSM telemetry
**Key Insights:**
- 12V/24V compatibility with buck converter
- JSON data logging format
- Engine hours tracking via digital input
- Temperature and pressure analog inputs
- Remote monitoring capabilities

#### **Common Best Practices from Research:**
1. **External ADC preferred** over ESP32 internal (linearity issues)
2. **Heavy filtering** on analog inputs (100nF + larger caps)
3. **Protection resistors** on all inputs (100-220Ω typical)
4. **Trimmer provisions** for field calibration
5. **AC coupling** for alternator RPM sensing
6. **Schmitt triggers** for digital signal conditioning
7. **Zener protection** on all analog inputs
8. **Current limiting** on all outputs

---

### **Component Specifications & Sourcing**

#### **Precision Resistors (1% Metal Film)**
| Value | Quantity | Application | Package | Part Number (Example) |
|-------|----------|-------------|---------|----------------------|
| 100Ω | 6 | Protection | 1206 | ERJ-8ENF1000V |
| 220Ω | 4 | Protection | 1206 | ERJ-8ENF2200V |
| 1kΩ | 8 | General | 1206 | ERJ-8ENF1001V |
| 2.2kΩ | 2 | Reference | 1206 | ERJ-8ENF2201V |
| 4.7kΩ | 4 | Pull-up | 1206 | ERJ-8ENF4701V |
| 10kΩ | 6 | Dividers | 1206 | ERJ-8ENF1002V |
| 47kΩ | 2 | High voltage | 1206 | ERJ-8ENF4702V |
| 100kΩ | 2 | High voltage | 1206 | ERJ-8ENF1003V |

#### **Trimmer Potentiometers (Multi-turn)**
| Value | Quantity | Application | Type | Part Number (Example) |
|-------|----------|-------------|------|----------------------|
| 1kΩ | 2 | Temp calibration | 10-turn | 3296W-1-102LF |
| 10kΩ | 4 | General cal | 10-turn | 3296W-1-103LF |

#### **Capacitors (Filtering & Decoupling)**
| Value | Quantity | Application | Type | Voltage |
|-------|----------|-------------|------|---------|
| 100nF | 12 | HF filtering | Ceramic X7R | 50V |
| 1µF | 4 | LF filtering | Ceramic X7R | 25V |
| 10µF | 4 | Power filtering | Tantalum | 25V |
| 100µF | 2 | Bulk filtering | Electrolytic | 35V |

#### **Protection Components**
| Component | Quantity | Application | Rating | Part Number (Example) |
|-----------|----------|-------------|--------|----------------------|
| 3.3V Zener | 6 | Input protection | 500mW | BZX84C3V3 |
| Schottky Diode | 8 | Reverse protection | 40V/1A | 1N5819 |
| TVS Diode | 4 | ESD protection | 15V | SMAJ15A |

#### **Relays & Drivers**
| Component | Quantity | Application | Rating | Notes |
|-----------|----------|-------------|--------|-------|
| Automotive Relay | 3 | Glow/Ignition/Starter | 40A/12V | Bosch type |
| Relay Socket | 3 | Mounting | - | PCB mount |
| MOSFET Driver | 3 | Relay drive | Logic level | IRLZ44N or similar |

---

### **PCB Layout Guidelines**

#### **Trace Width Calculations**
| Current | Internal | External | Copper Weight | Notes |
|---------|----------|----------|---------------|-------|
| 100mA | 6 mil | 4 mil | 1oz | Signal traces |
| 500mA | 12 mil | 8 mil | 1oz | Power traces |
| 1A | 25 mil | 15 mil | 1oz | Relay drivers |
| 5A | 75 mil | 40 mil | 2oz | Power input |
| 40A | Via stitching | External wire | N/A | Relay contacts |

#### **Critical Layout Requirements**
1. **Analog Section Isolation:** Separate ground planes for analog/digital
2. **Power Supply Placement:** Input protection near connector
3. **Crystal Placement:** Short traces, ground guard rings
4. **High Current Traces:** Heavy copper, thermal vias
5. **Connector Placement:** Automotive-grade, sealed connectors
6. **Test Points:** All critical signals accessible
7. **Mounting:** Automotive vibration resistance

---

### **Environmental Requirements (Automotive)**

#### **Operating Conditions**
- **Temperature:** -40°C to +85°C (automotive grade)
- **Humidity:** 0-95% non-condensing
- **Vibration:** 20G @ 10-2000Hz (engine compartment)
- **Shock:** 50G, 11ms duration
- **EMI/EMC:** CISPR-25 compliance
- **Ingress:** IP67 (connector sealing)

#### **Component Selection Criteria**
- **Automotive Grade:** AEC-Q100 qualified where possible
- **Extended Temperature:** -40°C to +125°C for critical components
- **Low ESR Capacitors:** For power supply filtering
- **Metal Film Resistors:** For precision and stability
- **Conformal Coating:** PCB protection against moisture/contaminants

---

### **Immediate Next Steps - Action Plan**

#### **Phase 1: Schematic Design (Week 1-2)**
1. **Create detailed schematic** using KiCad or Altium Designer
2. **Component selection** - specific part numbers from major suppliers
3. **Circuit simulation** - SPICE models for critical analog circuits
4. **Design review** - peer review of schematic before PCB layout
5. **BOM cost analysis** - target <$100 per board in qty 10

#### **Phase 2: PCB Layout (Week 3-4)**
1. **Component placement** - optimize for EMI/thermal performance
2. **Routing strategy** - separate analog/digital domains
3. **Power plane design** - minimize noise coupling
4. **DRC compliance** - design rule check for manufacturing
5. **3D modeling** - mechanical fit verification

#### **Phase 3: Prototype & Testing (Week 5-8)**
1. **Board fabrication** - 4-layer PCB with controlled impedance
2. **Component assembly** - initial hand-soldering for prototype
3. **Bench testing** - verify all circuits without engine
4. **Engine integration** - real-world testing on Bobcat 743
5. **Calibration & tuning** - optimize sensor readings

#### **Phase 4: Documentation & Release (Week 9-12)**
1. **Assembly documentation** - detailed build instructions
2. **Calibration procedures** - step-by-step sensor setup
3. **Installation guide** - mechanical and electrical installation
4. **Troubleshooting guide** - common issues and solutions
5. **Open-source release** - GitHub repository with full design files

---

### **Critical Questions for Next Review**

#### **Design Decisions Needed:**
1. **External ADC vs ESP32 ADC?** - Recommendation: ADS1115 for precision
2. **Single PCB vs modular?** - Recommendation: Single PCB with removable sections
3. **Connector type?** - Automotive IP67 vs standard headers
4. **Power input protection?** - Fuse + TVS + reverse protection
5. **Enclosure strategy?** - IP67 aluminum enclosure vs plastic

#### **Sensor Compatibility Verification:**
1. **Actual Bobcat 743 sensor resistance values** - need field measurements
2. **Wiring harness compatibility** - connector pinouts and wire colors
3. **Alternator W terminal availability** - verify on target machine
4. **Glow plug controller integration** - existing vs replacement
5. **Safety interlocks** - oil pressure, coolant temperature limits

#### **Regulatory Compliance:**
1. **EMI/EMC requirements** - CISPR-25 automotive standard
2. **Safety certifications** - UL, CE marking requirements
3. **Environmental compliance** - RoHS, REACH directives
4. **Functional safety** - ISO 26262 considerations for critical functions

---

### **Cost Analysis (Preliminary)**

#### **Component Costs (Qty 10 estimate)**
| Category | Cost per Board | Notes |
|----------|---------------|-------|
| ESP32 Module | $8 | ESP32-WROOM-32D |
| Resistors/Caps | $5 | Passive components |
| Protection | $12 | Zeners, TVS, fuses |
| Relays | $25 | 3× automotive relays |
| Connectors | $15 | IP67 automotive grade |
| PCB Fabrication | $20 | 4-layer, controlled impedance |
| Assembly | $15 | Hand assembly/testing |
| **Total** | **$100** | Target achieved |

#### **Development Costs**
- **Design time:** 80 hours @ $50/hr = $4,000
- **Prototype PCBs:** 10 boards @ $25 = $250
- **Test equipment:** $500 (if needed)
- **Certification:** $2,000-5,000 (if pursuing)

---

### **Collaboration Opportunities**

#### **Skill Sets Needed:**
1. **PCB Layout** - Experienced with automotive electronics
2. **Mechanical Design** - Enclosure and mounting systems
3. **Firmware Development** - ESP32 programming expertise
4. **Testing & Validation** - Access to Bobcat 743 for real-world testing
5. **Documentation** - Technical writing and illustration

#### **Potential Partners:**
- **Open-source communities** - Arduino, ESP32, agricultural automation
- **Academic institutions** - Engineering capstone projects
- **Bobcat owners/operators** - Real-world testing and feedback
- **PCB fabricators** - Local/regional electronics manufacturers
- **Component suppliers** - Digikey, Mouser, Arrow for volume pricing

---

### **Success Metrics**

#### **Technical Performance:**
- **Sensor accuracy:** ±2% full scale for critical parameters
- **System reliability:** >99.9% uptime in field conditions
- **Response time:** <100ms for safety-critical functions
- **Power consumption:** <200mA average, <50mA standby
- **Operating temperature:** -40°C to +85°C verified

#### **Project Goals:**
- **Open-source release** - Complete design files and documentation
- **Community adoption** - >100 downloads in first year
- **Commercial viability** - Clear path to small-scale production
- **Safety compliance** - No safety incidents in field testing
- **Cost effectiveness** - <$150 total installed cost

---

**STATUS: Ready for detailed schematic design phase**
**NEXT ACTION: Create KiCad project and begin component placement**
