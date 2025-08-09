# Sensor Calibration

All calibration logic runs in C++. The frontend only collects an “Actual” value and sends it to an API endpoint. The device computes and persists runtime calibration values in NVS.

## Supported sensors (examples)

- Battery voltage (divider scale)
- Engine temperature (NTC mapping / scale)
- Oil pressure (resistive sender)
- Fuel level (empty/full ADC)
- Hydraulic pressure (scale)

## How to calibrate

1. Go to Settings → Sensor Configuration.
2. Enter the Actual value (e.g., multimeter battery voltage) and press Enter.
3. Confirm the prompt. The backend will:
   - Read current raw ADC
   - Compute new calibration constants
   - Persist to NVS
   - Apply immediately

## Rules

- No business logic in JavaScript. JS must not compute calibration constants.
- Runtime calibration variables are used for all reads (not static config).
- Persist in NVS and reload on boot.

## Troubleshooting

- If values don’t change, hard-refresh the UI (Ctrl+F5).
- Verify the raw ADC is non-zero.
- Check /status for current runtime scales and last calibration time (if exposed).
