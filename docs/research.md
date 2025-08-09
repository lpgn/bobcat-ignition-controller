# Engineering Report: Integration of an ESP32-Based Control System with a Bobcat 743 Skid Steer Loader

## Section 1: System Component Analysis and Foundational Assessment

This report provides a comprehensive technical guide for interfacing a LILYGO T-Relay ESP32 development board with a Bobcat 743 skid steer loader. The objective is to create a robust monitoring and control system for key engine and electrical functions. This requires a detailed analysis of both the modern microcontroller platform and the legacy analog systems of the heavy machinery.

### 1.1 The Control Platform: LILYGO T-Relay ESP32 Module

The core of the proposed system is the LILYGO T-Relay, a development board featuring an Espressif ESP32 microcontroller. This platform is well-suited for this application due to its integrated relays, wireless capabilities, and flexible power requirements. Several versions of this board exist, offering four, six, or eight relay channels.[1, 2, 3] For the scope of this project, which involves controlling starting and lighting circuits while monitoring multiple sensors, the 4-channel or 8-channel models provide sufficient I/O.

**Core Component Specifications:**
*   **Microcontroller (MCU):** The board is equipped with an ESP32-WROVER-E or similar ESP32 module, which includes a powerful dual-core processor, Wi-Fi (802.11 b/g/n), and Bluetooth v4.2 BLE capabilities. This allows for potential remote monitoring and control via a wireless interface.[3, 4]
*   **Power Input:** A key feature is the wide DC power input range of 12-24V, making it directly compatible with the Bobcat's native 12V electrical system without requiring an additional voltage regulator.[2, 3]
*   **Relay GPIO Mapping:** The onboard relays are pre-wired to specific General-Purpose Input/Output (GPIO) pins of the ESP32. For the common 4-channel model (H516), the mapping is as follows [4]:
    *   K1 Relay: GPIO21
    *   K2 Relay: GPIO19
    *   K3 Relay: GPIO18
    *   K4 Relay: GPIO05
    For the 8-channel model, the mapping extends to additional pins.[3] This fixed mapping is a critical parameter for software development.
*   **Programming Interface:** The T-Relay board does not include an onboard USB-to-serial converter chip. Therefore, programming the ESP32 requires an external LILYGO T-U2T USB to TTL adapter, which connects to the board's programming header.[2, 3]

### 1.2 The Target Platform: Bobcat 743 Electrical and Mechanical Architecture

The Bobcat 743 is a legacy skid steer loader with a purely analog control system. Successful integration hinges on understanding its fundamental electrical and mechanical configuration. The loader operates on a standard **12-volt, negative-ground electrical system**, with power supplied by a battery and an engine-driven alternator.[5]

**Engine Identification:**
The Bobcat 743 has been produced over a long period, resulting in several possible engine configurations due to original manufacturing runs, replacements, and upgrades. The most commonly cited engine for this model is the **Kubota V1702**, a 4-cylinder indirect injection diesel engine.[6, 7, 8, 9] However, documentation and aftermarket suppliers also list compatibility with or upgrade paths to other Kubota engines, such as the V2003-T and V2203.[10, 11]

While the fundamental operating principles of the electrical sensors (e.g., oil pressure switches, temperature senders) are consistent across these Kubota engine series, specific part numbers and their precise electrical characteristics may vary. Therefore, it is imperative for the implementer to physically verify the engine model installed in their specific machine. This report will proceed with the Kubota V1702 as the baseline, as it is the most common configuration, but the methodologies described herein are broadly applicable.

**Key OEM Part Numbers:**
A primary reference for this integration is the engine electrical circuitry parts list for the Bobcat 743, specifically for serial numbers 501914085 and above.[12] This document provides OEM part numbers for critical components, which are essential for sourcing data and confirming compatibility.

### 1.3 Relay Power Handling Capacity: Analysis of the HRS4H-S-DC5V

The LILYGO T-Relay boards are equipped with HRS4H-S-DC5V relays, which are responsible for all switching functions.[2, 4] A thorough analysis of these relays' capabilities is necessary to determine their suitability for controlling the Bobcat's electrical loads.

**Datasheet Analysis:**
A synthesis of data from multiple datasheets for the HRS4 series relay reveals its electrical limits.[13, 14, 15, 16]
*   **Coil Specifications:** The relay coil is designed to be actuated by a 5V DC signal and has a nominal power consumption of approximately 360mW.[15, 17] This low-power coil is driven by the ESP32 via an onboard transistor circuit, not directly from a GPIO pin.
*   **Contact Rating (Load Switching):** The specifications for the load-bearing contacts vary slightly between sources but can be consolidated to a safe operating limit. While some datasheets list ratings as high as 15A at 28VDC, a more common and conservative rating is **10A at 250VAC / 28VDC**.[15, 16, 18] For reliable, long-term operation in a demanding DC environment, this report will adopt a maximum continuous DC current rating of **10A**.

This 10A current limit is the single most important constraint for the control portion of this project. Any electrical load on the Bobcat that is to be controlled by the T-Relay must be analyzed to ensure its current draw does not exceed this value. As will be detailed in Section 3, this immediately indicates that high-current systems like the glow plugs or the full starter motor current cannot be switched directly and will require an alternative control strategy.

## Section 2: Sensor Interfacing and Signal Conditioning

The ESP32 microcontroller operates at a 3.3V logic level. The sensors on the Bobcat 743, however, operate within a 12V automotive electrical system and produce signals that are not directly compatible with the ESP32's inputs. This section details the design of specific signal conditioning circuits required to safely and accurately interface each sensor with the microcontroller.

### 2.1 Engine Run Status: Deriving a Signal from the Alternator

A reliable method for determining if the engine is running is to monitor the state of the alternator's charge indicator circuit.

**Principle of Operation:**
Most automotive alternators have a terminal (often labeled "L" for Lamp or "D+" for Dynamo) that is used to drive the charge warning light on the dashboard. The behavior of this terminal provides a clear binary signal:
*   **Engine Off (Ignition On):** The alternator is not generating voltage. The "L" terminal is internally connected to ground through the alternator's field windings.
*   **Engine Running (Charging):** The alternator spins up and begins generating its own voltage. The "L" terminal is pulled up to the system's charging voltage, typically between 13.5V and 14.5V.

This voltage swing from near 0V to ~14V is an ideal signal for detecting engine state, but it must be scaled down to be read by the ESP32.

**Interface Circuit Design:**
A simple voltage divider is required to reduce the ~14V signal to a level below the ESP32's 3.3V maximum input voltage. Furthermore, to protect the microcontroller from voltage spikes common in vehicle electrical systems, a Zener diode should be included for over-voltage clamping.[19, 20]

A robust circuit consists of:
1.  A 10 kΩ resistor ($R_1$) connected between the alternator's "L" terminal and a GPIO input pin on the ESP32.
2.  A 3.3 kΩ resistor ($R_2$) connected between the same ESP32 GPIO pin and the system ground.
3.  A 3.6V Zener diode connected in parallel with the 3.3 kΩ resistor (cathode to the GPIO pin, anode to ground).

The output voltage ($V_{out}$) at the ESP32 pin can be calculated as:
$V_{out} = V_{in} \times \frac{R_2}{R_1 + R_2}$

With a charging voltage of 14V, this circuit yields:
$V_{out} = 14V \times \frac{3300\Omega}{10000\Omega + 3300\Omega} \approx 3.48V$

The 3.6V Zener diode will clamp this voltage to a safe level for the ESP32, ensuring that even if the charging voltage rises slightly, the microcontroller's input is protected. The software can then read this GPIO pin as a digital input: a LOW signal indicates the engine is off, and a HIGH signal indicates the engine is running.

### 2.2 Engine Coolant Temperature Sender (P/N 6658818)

The engine temperature is monitored by a sender unit that changes its electrical resistance with temperature.

**Component Identification and Characteristics:**
The parts catalog for the Bobcat 743 (S/N 501914085 & Above) identifies the engine temperature sender as Bobcat P/N 6658818 (which supersedes P/N 6599492).[12] This part is confirmed to be compatible with the 743 model.[21, 22] This type of sensor is a Negative Temperature Coefficient (NTC) thermistor, meaning its resistance decreases as the engine coolant temperature increases.

A significant challenge is that no public datasheet with a resistance-vs-temperature curve is available for this specific part number. Without this data, it is impossible to convert a resistance reading into an accurate temperature value. Therefore, a manual characterization procedure is required.

**Actionable Guidance: The Characterization Procedure:**
1.  Safely remove the temperature sender from the engine block.
2.  Prepare a container of water that can be heated, and place a calibrated thermometer (e.g., a digital multimeter with a thermocouple probe) in the water to monitor its temperature.
3.  Submerge the sensing tip of the Bobcat sender in the water.
4.  Connect a multimeter set to measure resistance (Ohms) to the sender's electrical terminal and its metal body (ground).
5.  Record the resistance at several distinct temperature points as the water is slowly heated (e.g., at 20°C, 40°C, 60°C, 80°C, and 100°C). This will create a lookup table or a curve that can be used in software.

**Interface Circuit Design:**
To read the changing resistance, a voltage divider circuit is used. This circuit converts the resistance value into a voltage that can be measured by the ESP32's Analog-to-Digital Converter (ADC).
*   Connect the temperature sender between the ESP32's 3.3V power pin and an ADC-capable GPIO pin.
*   Connect a fixed, known-value resistor (a 10 kΩ precision resistor is a good starting point) from the same ADC pin to ground.

The voltage read at the ADC pin ($V_{ADC}$) will be:
$V_{ADC} = 3.3V \times \frac{R_{fixed}}{R_{sender} + R_{fixed}}$

**Software Implementation:**
The software must perform the reverse calculation to determine the sender's resistance and then convert that to a temperature.
1.  Read the raw value from the ADC pin.
2.  Convert the raw ADC value to a voltage ($V_{ADC}$).
3.  Rearrange the voltage divider formula to solve for the sender's resistance ($R_{sender}$):
    $R_{sender} = R_{fixed} \times (\frac{3.3V}{V_{ADC}} - 1)$
4.  Using the characterization data gathered previously, convert the calculated $R_{sender}$ value into a temperature. This can be done using a lookup table with linear interpolation or by fitting the data to the Steinhart-Hart thermistor equation for a more accurate continuous model.

### 2.3 Pressure Switches (Engine & Hydraulic)

The Bobcat 743 uses simple pressure switches for critical low-pressure warnings, rather than variable senders.

**Component Identification and Principle of Operation:**
*   **Engine Oil Pressure Switch:** Identified as P/N 6969775 (supersedes 6652576).[12, 23, 24] For the Kubota V1702 engine, this is a normally open switch that closes to ground when oil pressure drops below a threshold of approximately 0.25 bar.[25]
*   **Hydraulic Oil Pressure Switch:** Identified as P/N 6671062 (supersedes 6664577).[12, 26, 27] This switch also functions by closing a circuit to ground when hydraulic charge pressure is too low.

These components are designed to complete a circuit for a warning light on the instrument panel. When system pressure is normal, the switch is open. When pressure falls to a critical level, the switch closes, connecting its signal wire to the chassis ground.

**Interface Circuit Design:**
The nature of these switches allows for an elegant, zero-component hardware interface by leveraging the ESP32's internal pull-up resistors.
*   Connect the signal wire from the pressure switch directly to an ESP32 GPIO pin.
*   In the software, configure this pin as an input with the internal pull-up resistor enabled (e.g., in the Arduino framework: `pinMode(pressurePin, INPUT_PULLUP);`).

This configuration causes the ESP32 to internally connect a resistor between the GPIO pin and its 3.3V supply.
*   **Normal Pressure:** The switch is open, so the pull-up resistor pulls the GPIO pin to a HIGH state (3.3V).
*   **Low Pressure:** The switch closes, connecting the GPIO pin directly to the chassis ground, which pulls the pin to a LOW state (0V).

The software can then monitor this pin for a LOW state to trigger a warning. It is also advisable to implement a software "debouncing" routine to prevent false triggers from momentary signal flicker caused by vibration or contact bounce.

### 2.4 Fuel Level Sender

Interfacing with the fuel level sender presents a challenge similar to the temperature sender, compounded by a lack of definitive part numbers in the available documentation for the 743 model.[28, 29, 30]

**Identifying Common Standards:**
Automotive fuel senders are variable resistors, but their resistance ranges are not universal. Common standards include [31]:
*   **240Ω (Empty) - 33.5Ω (Full):** A common industry standard.
*   **73Ω (Empty) - 10Ω (Full):** Used in many older Ford and Chrysler vehicles.
*   **0Ω (Empty) - 90Ω (Full):** Used in many GM vehicles from 1965 onward.

The Bobcat 743 most likely uses one of these standards.

**Actionable Guidance: Measurement Procedure:**
The implementer must determine the resistance range of their specific sender. This can be done by disconnecting the sender wires and measuring the resistance across its terminals with a multimeter under two conditions: when the fuel tank is nearly full and when it is nearly empty.[31] This will provide the $R_{full}$ and $R_{empty}$ values.

**Adaptable Interface Circuit:**
A voltage divider is again the correct approach. The key is to select the fixed resistor value ($R_{fixed}$) to maximize the voltage swing across the ADC's range, thereby maximizing measurement resolution. The optimal value for the fixed resistor is the geometric mean of the sender's minimum and maximum resistance:
$R_{fixed} = \sqrt{R_{empty} \times R_{full}}$

For example, if the sender is found to use the 240-33.5Ω standard, the optimal fixed resistor would be:
$R_{fixed} = \sqrt{240\Omega \times 33.5\Omega} \approx \sqrt{8040} \approx 89.7\Omega$
A standard 100Ω resistor would be a suitable and readily available choice.

The circuit and software logic would then be identical to that of the temperature sender, using the measured resistance range and the chosen fixed resistor value to calculate the fuel level.

### Table 1: Sensor Interface Design Summary

| Parameter | Bobcat Component | OEM P/N | Electrical Type | Interface Circuit | Key Components | ESP32 Pin Type | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Engine Run Status** | Alternator | N/A | 12V Switched Signal | Voltage Divider | $R_1=10k\Omega$, $R_2=3.3k\Omega$, 3.6V Zener | Digital Input | Signal from "L" or "D+" terminal. |
| **Engine Temp** | Temp Sender | 6658818 | NTC Thermistor | Voltage Divider | $R_{fixed}=10k\Omega$ (recommended) | ADC Input | Requires manual characterization of R vs. T curve. |
| **Engine Oil Pressure** | Pressure Switch | 6969775 | Switch to Ground | Internal Pull-up | None | Digital Input | Configure pin as `INPUT_PULLUP`. |
| **Hydraulic Pressure** | Pressure Switch | 6671062 | Switch to Ground | Internal Pull-up | None | Digital Input | Configure pin as `INPUT_PULLUP`. |
| **Fuel Level** | Fuel Sender | Varies | Variable Resistor | Voltage Divider | $R_{fixed} = \sqrt{R_{empty} \times R_{full}}$ | ADC Input | Requires manual measurement of sender's resistance range. |

## Section 3: High-Current Load Control and Relay Integration

This section addresses the control of the Bobcat's high-current electrical systems using the LILYGO T-Relay. The analysis is guided by the conservative 10A continuous DC current rating of the onboard HRS4H-S-DC5V relays, as established in Section 1.3.

### 3.1 Starter Motor Solenoid Integration

The engine starting system involves two distinct components: the high-current starter motor and a lower-current starter solenoid that actuates it.

**Load Analysis:**
The starter motor itself draws an immense amount of current during cranking, often in the range of 150-300A for a diesel engine of this size.[32] This current **must not** be switched directly by the T-Relay. The appropriate target for control is the starter solenoid (P/N 6659319).[12] The solenoid is an electromagnet that performs two functions: it engages the starter gear with the engine's flywheel and closes a high-power contact to energize the starter motor. The current required to energize the solenoid's coil is significantly lower, typically in the range of 5-15A.

**Verification and Circuit Design:**
The estimated 5-15A draw of the solenoid falls within the capabilities of the T-Relay's 10A rating, although it is at the upper end. For this reason, using an external, higher-rated relay is the most conservative approach, but direct control is feasible.

If direct control is chosen, the circuit must include protection against the inductive voltage spike generated when the solenoid is de-energized. This is accomplished with a flyback diode.
*   **Wiring:** The T-Relay's normally open (NO) and common (COM) contacts should be wired in series with the control wire that runs from the ignition switch's "START" position to the solenoid's "S" terminal.
*   **Protection:** A flyback diode (e.g., a 1N4007 or similar) must be connected in parallel with the solenoid's coil terminals. The cathode (banded end) of the diode connects to the positive side of the coil (the "S" terminal), and the anode connects to the ground side. This diode provides a safe path for the current to circulate and dissipate when the relay opens, preventing a high-voltage spike that could arc across and damage the relay contacts.

### 3.2 Lighting Circuit Integration

The Bobcat 743 is equipped with front and rear work lights, which represent a significant resistive load.[5, 33]

**Load Analysis:**
Assuming the machine is fitted with two standard 55W halogen headlight bulbs, the total current draw ($I$) on the 12V system can be calculated as:
$I = \frac{P}{V} = \frac{2 \times 55W}{12V} = \frac{110W}{12V} \approx 9.17A$

**Verification and Engineering Recommendation:**
This calculated load of 9.17A is extremely close to the T-Relay's conservative 10A continuous rating. While technically within the limit, it leaves no safety margin for factors such as lower system voltage during engine idle (which increases current for the same power), the installation of higher-wattage bulbs, or long-term contact degradation.

For a robust and reliable system, it is strongly recommended **not** to switch the lighting load directly with the T-Relay. The best practice is to use the T-Relay as a pilot for a standard 30/40A automotive-style (Bosch) relay.
*   **Circuit Design:** The T-Relay will switch the low-current coil of the external automotive relay (typically drawing only 150-200mA). The high-power contacts of the automotive relay will then switch the main 9.17A lighting load. This isolates the T-Relay from the high current, ensuring its longevity and preventing overheating. A flyback diode across the external relay's coil is also recommended.

### 3.3 Glow Plug Circuit Integration

The glow plug system is essential for starting the diesel engine in cold conditions and represents the largest electrical load in the control system.

**Load Analysis:**
The Kubota V1702 engine is equipped with four glow plugs.[9] The relevant Bobcat part number for these plugs is 6655233.[34, 35, 36] A single diesel engine glow plug can draw between 10A and 20A during its initial heating phase. Therefore, the total current draw for all four plugs operating simultaneously is in the range of **40A to 80A**.

**Verification and Mandatory Instruction:**
This load **grossly exceeds** the 10A capacity of the T-Relay's onboard relays. Attempting to switch the glow plugs directly with the T-Relay will result in immediate and catastrophic failure of the relay, potentially damaging the board itself.

The Bobcat 743 is equipped from the factory with a high-current glow plug relay or solenoid designed to handle this load. The T-Relay **must not** be used to replace this component. Instead, it should be wired to energize the low-current *coil* of the factory glow plug relay. This integration method is identical in principle to using an external relay for the lighting circuit and is the only safe and functional approach.

### Table 2: Load Control Integration Strategy

| Electrical Load | Estimated Current Draw | T-Relay Capacity | Engineering Recommendation | Verdict |
| :--- | :--- | :--- | :--- | :--- |
| **Starter Solenoid** | 5-15A (Inductive) | 10A DC Continuous | Direct control is feasible. **Flyback diode is mandatory** for contact protection. | ✅ Safe with Protection |
| **Lighting Circuit** | ~9.2A (Resistive) | 10A DC Continuous | Use T-Relay to pilot an external 30/40A automotive relay. Direct control is not recommended due to lack of safety margin. | ⚠️ Use External Relay |
| **Glow Plugs** | 40-80A (Resistive) | 10A DC Continuous | Use T-Relay to pilot the existing factory high-current glow plug relay. **Direct control is strictly forbidden.** | ❌ Direct Control Unsafe |

## Section 4: System Integration, Power, and Safety Protocols

Proper physical installation and adherence to safety protocols are paramount for the long-term reliability and safe operation of the custom control system. This section provides guidelines for wiring, power, and the integration of critical safety interlocks.

### 4.1 Physical Installation and Wiring Best Practices

The harsh operating environment of a skid steer—replete with vibration, moisture, and temperature extremes—demands professional-grade installation practices.
*   **Wire Gauge:** Use appropriately sized automotive-grade (e.g., TXL/GXL) wire. 18-22 AWG is suitable for low-current sensor signals and relay coil triggers. Heavier loads switched by external relays (e.g., lighting) should use 14-16 AWG wire.
*   **Protection:** All wiring should be routed away from sharp edges, exhaust components, and moving parts. Protect wiring runs with convoluted tubing (loom) and use rubber grommets where wires must pass through sheet metal bulkheads.
*   **Grounding:** Establish a single, clean, and robust grounding point on the vehicle's chassis for the entire control system. All ground connections for the ESP32, sensors, and external relays should terminate at this point. This practice prevents ground loops, which can introduce noise and cause unpredictable behavior in sensor readings.
*   **Enclosure:** The LILYGO T-Relay board must be housed in a weather-resistant enclosure (e.g., an IP65-rated project box) to protect it from moisture and dust. The enclosure should be securely mounted in a location that minimizes exposure to direct engine heat and excessive vibration, such as inside the operator cab or within a protected section of the engine bay.

### 4.2 Powering the Control Module

To prevent the control system from draining the Bobcat's battery when the machine is not in use, it must be powered from a switched ignition source.
*   **Connection Point:** The 12-24V power input terminals on the T-Relay should be connected to a circuit that is only active when the ignition key is in the "RUN" or "ON" position. A suitable tap point is the "RUN" terminal of the ignition switch itself.
*   **Circuit Protection:** An inline blade fuse holder with a 1A or 2A fuse must be installed on the positive power wire leading to the T-Relay. This protects both the control module and the machine's wiring in the event of a short circuit.

### 4.3 Critical Safety Interlocks and Failsafe Logic

A custom control system must respect, not bypass, the machine's original safety features. The Bobcat 743 is equipped with safety interlocks, such as a seat bar and neutral safety switches, to prevent accidental movement or operation.[37, 38, 39] Integrating a remote start or alternative control system without incorporating these interlocks creates an extremely dangerous condition.

**Implementation Strategy:**
*   **Hardware Integration:** The seat bar switch and the transmission neutral switch (if applicable) should be wired as digital inputs to the ESP32. These switches typically function like the pressure switches (closing a circuit to ground when in the "safe" state), so the `INPUT_PULLUP` method described in Section 2.3 is the appropriate interface.
*   **Software Logic:** The microcontroller's software must be written to treat these safety signals as non-negotiable preconditions. The logic for enabling the starter relay must verify that these inputs are in their correct state (e.g., seat bar is down, transmission is in neutral) before allowing the starter to engage. An example of this logic would be: `if (seatBarEngaged && transmissionInNeutral && startButtonPressed) { activateStarterRelay(); }`.
*   **Failsafe Mechanisms:** Implement a software watchdog timer (a standard feature of the ESP32). The watchdog is a hardware timer that will automatically reset the microcontroller if the main program loop fails to "check in" periodically. This ensures that if the software hangs or freezes, the system will reset to a known-safe state rather than leaving a relay energized unintentionally.
*   **System Redundancy:** It is strongly recommended that the original factory ignition switch and its associated wiring be left fully intact. The T-Relay control system should be wired in parallel, allowing the factory switch to function as a manual override and a reliable backup in case of failure in the custom electronics.

## Conclusion and Final Verification Checklist

The integration of a LILYGO T-Relay ESP32 module with a Bobcat 743 skid steer is a feasible project that can add significant modern functionality to the legacy machine. Success is contingent on a methodical approach that addresses the unique challenges of interfacing a digital microcontroller with analog automotive systems.

The analysis confirms that while the ESP32 platform is highly capable, careful signal conditioning is required for all sensor inputs to protect the microcontroller and ensure accurate readings. The primary challenge lies in the lack of public data for the temperature and fuel senders, necessitating manual characterization by the implementer. For control outputs, the 10A limit of the onboard relays is a critical constraint. This limit is sufficient for piloting low-current loads like the starter solenoid coil but mandates the use of external, higher-power automotive relays for switching the lighting and glow plug circuits. Finally, the integration of factory safety interlocks into the control logic is not optional; it is a fundamental requirement for a safe and responsible implementation.

Before powering on the integrated system, the following verification checklist should be completed:

*   [ ] **Engine Model:** Confirm the specific Kubota engine model installed (e.g., V1702).
*   [ ] **Temperature Sender:** Complete the resistance-vs-temperature characterization procedure.
*   [ ] **Fuel Sender:** Measure and record the full and empty resistance range.
*   [ ] **Circuit Construction:** Verify all sensor voltage divider and protection circuits are built correctly per the schematics.
*   [ ] **Starter Solenoid:** Confirm the flyback diode is installed correctly across the solenoid coil with the proper polarity.
*   [ ] **High-Current Loads:** Verify that external relays are used for the lighting and glow plug circuits and that the T-Relay only switches their coils.
*   [ ] **Safety Interlocks:** Confirm the Bobcat's seat bar and neutral safety switches are correctly wired as inputs to the ESP32.
*   [ ] **Software Verification:** Review the code to ensure it correctly checks for safety interlock conditions before activating the starter and that a watchdog timer is implemented.
*   [ ] **Power Source:** Confirm the T-Relay is powered from a switched ignition source and that an inline fuse is installed.
*   [ ] **Grounding:** Verify that all system components share a single, clean chassis ground point.