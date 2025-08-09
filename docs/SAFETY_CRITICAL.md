# 🚨 SAFETY CRITICAL REQUIREMENTS 🚨

**⚠️ MANDATORY SAFETY REQUIREMENTS BASED ON ENGINEERING RESEARCH ⚠️**

This document outlines **MANDATORY** safety requirements that MUST be implemented before using this system. Failure to implement these requirements can result in **equipment damage, injury, or death**.

## 1. RELAY CURRENT LIMITS - IMMEDIATE DANGER ⚠️

### Research Finding: HRS4H-S-DC5V Relay Rating = 10A Maximum DC Current

**CRITICAL SAFETY VIOLATIONS IN CURRENT DESIGN:**

### ❌ GLOW PLUG RELAY - IMMEDIATE FIRE/EXPLOSION HAZARD
- **Current Design**: Direct control via T-Relay GPIO19
- **Actual Load**: 40-80A total current (4 × 10-20A glow plugs)
- **Relay Rating**: 10A maximum
- **VIOLATION**: **8x OVERCURRENT** - Will cause immediate relay failure, arcing, potential fire
- **MANDATORY FIX**: Use T-Relay to pilot existing factory high-current glow plug relay coil ONLY

### ⚠️ LIGHTING RELAY - SAFETY MARGIN VIOLATION  
- **Current Design**: Direct control via T-Relay GPIO5
- **Actual Load**: ~9.2A (2 × 55W halogen bulbs at 12V)
- **Relay Rating**: 10A maximum
- **VIOLATION**: Only 0.8A safety margin - insufficient for voltage variations
- **MANDATORY FIX**: Use T-Relay to pilot external 30/40A automotive relay

### ✅ STARTER RELAY - ACCEPTABLE WITH PROTECTION
- **Current Design**: Direct control via T-Relay GPIO18
- **Actual Load**: 5-15A starter solenoid coil
- **Relay Rating**: 10A maximum
- **STATUS**: Acceptable but requires flyback diode protection

## 2. MANDATORY SAFETY INTERLOCKS ⚠️

### Research Finding: Safety Interlocks Are Non-Negotiable

**MISSING CRITICAL SAFETY SYSTEMS:**

### ❌ SEAT BAR SAFETY SWITCH (GPIO25)
- **Purpose**: Prevents operation when operator not seated
- **Type**: Normally open switch, closes to ground when seated
- **Implementation**: INPUT_PULLUP configuration
- **MANDATORY**: Must be checked before allowing starter engagement

### ❌ TRANSMISSION NEUTRAL SAFETY SWITCH (GPIO27)  
- **Purpose**: Prevents starting when transmission in gear
- **Type**: Normally open switch, closes to ground in neutral
- **Implementation**: INPUT_PULLUP configuration
- **MANDATORY**: Must be checked before allowing starter engagement

## 3. REQUIRED SOFTWARE SAFETY LOGIC ⚠️

### MANDATORY Safety Check Function:
```cpp
bool safetyInterlocksPassed() {
    // Read safety switches (INPUT_PULLUP = LOW when switch closed)
    bool seatBarEngaged = (digitalRead(SEAT_BAR_PIN) == LOW);
    bool transmissionInNeutral = (digitalRead(NEUTRAL_SAFETY_PIN) == LOW);
    
    return seatBarEngaged && transmissionInNeutral;
}
```

### MANDATORY Starter Logic:
```cpp
void handleStarterRequest() {
    if (!safetyInterlocksPassed()) {
        Serial.println("SAFETY VIOLATION: Cannot start - safety interlocks not satisfied");
        return; // ABORT - DO NOT START
    }
    
    if (batteryVoltage < MIN_BATTERY_VOLTAGE) {
        Serial.println("SAFETY VIOLATION: Battery voltage too low for starting");
        return; // ABORT - DO NOT START  
    }
    
    // Only proceed if ALL safety checks pass
    activateStarter();
}
```

## 4. HARDWARE PROTECTION REQUIREMENTS ⚠️

### MANDATORY: Flyback Diodes for Inductive Loads
- **Starter Solenoid**: 1N4007 diode across coil terminals (cathode to +, anode to ground)
- **External Relay Coils**: 1N4007 diode across each automotive relay coil
- **Purpose**: Prevents voltage spikes that can destroy relay contacts

### MANDATORY: Fusing and Circuit Protection  
- **T-Relay Power**: 1-2A inline fuse on positive power wire
- **Each Control Circuit**: Appropriately sized fuses for external relays
- **Grounding**: Single-point chassis ground for entire system

## 5. SENSOR CALIBRATION REQUIREMENTS 📊

### Temperature Sender (P/N 6658818) - REQUIRES CHARACTERIZATION
- **Issue**: No public resistance-vs-temperature curve available
- **MANDATORY**: Manual characterization procedure required
- **Method**: Measure resistance at known temperatures (20°C, 40°C, 60°C, 80°C, 100°C)

### Fuel Level Sender - REQUIRES MEASUREMENT
- **Issue**: Unknown resistance range (240-33Ω vs 73-10Ω vs 0-90Ω standards)
- **MANDATORY**: Measure resistance at full and empty tank conditions
- **Current Settings**: May be completely incorrect without proper measurement

## 6. VERIFICATION CHECKLIST - MANDATORY BEFORE USE ✅

**DO NOT OPERATE SYSTEM UNTIL ALL ITEMS CHECKED:**

### Hardware Safety:
- [ ] **Glow plugs controlled via external factory relay coil ONLY**
- [ ] **Lights controlled via external 30/40A automotive relay**  
- [ ] **Flyback diodes installed on all inductive loads**
- [ ] **All circuits properly fused**
- [ ] **Single-point grounding implemented**
- [ ] **T-Relay in weather-resistant enclosure**

### Safety Interlocks:
- [ ] **Seat bar safety switch wired and tested (GPIO25)**
- [ ] **Neutral safety switch wired and tested (GPIO27)**
- [ ] **Safety interlock logic implemented in software**
- [ ] **Emergency stop/abort function working**

### Sensor Calibration:
- [ ] **Temperature sender resistance characterized**
- [ ] **Fuel sender resistance range measured**
- [ ] **Battery voltage divider calibrated**
- [ ] **All sensor readings validated against known values**

### Software Safety:
- [ ] **Watchdog timer implemented**
- [ ] **Safety checks enforced before starter engagement**
- [ ] **Factory ignition switch remains as backup/override**

## 7. LEGAL AND LIABILITY NOTICE ⚠️

**MODIFICATION OF SAFETY SYSTEMS:**
This system modifies or bypasses original safety systems. Improper implementation can result in:
- Equipment damage
- Personal injury  
- Death
- Legal liability
- Insurance voiding

**IMPLEMENTER RESPONSIBILITY:**
By implementing this system, you accept full responsibility for:
- Proper safety interlock implementation
- Compliance with local safety regulations
- Regular safety system testing and maintenance
- All consequences of system failure or misuse

**BACKUP SYSTEMS:**
Original factory ignition switch and safety systems MUST remain intact and functional as backup/override systems.

---

**🚨 THIS IS A SAFETY-CRITICAL SYSTEM 🚨**
**LIVES DEPEND ON PROPER IMPLEMENTATION**
**WHEN IN DOUBT, DON'T OPERATE**
