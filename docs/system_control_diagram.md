# Bobcat Ignition Control System Diagram

## Control Flow Overview

```
Web Interface Buttons:
┌─────────────┐  ┌──────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│  Power ON   │  │ Ignition ON  │  │ Front Light │  │ Back Light  │  │ Override    │
└─────────────┘  └──────────────┘  └─────────────┘  └─────────────┘  └─────────────┘
       │                │                │               │               │
       ▼                ▼                ▼               ▼               ▼
┌─────────────┐  ┌──────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│Main Power   │  │Glow Plugs +  │  │Front Light  │  │Back Light   │  │Emergency    │
│Relay ON     │  │Starter Motor │  │Relay Toggle │  │Relay Toggle │  │Start Bypass │
└─────────────┘  └──────────────┘  └─────────────┘  └─────────────┘  └─────────────┘
```

## System States and Transitions

```
┌─────────┐   Power ON    ┌───────────┐   Ignition ON   ┌──────────────────┐
│  IDLE   │ ──────────── ▶│ POWER_ON  │ ──────────────▶ │ GLOW_PLUG_HEATING│
└─────────┘               └───────────┘                 └──────────────────┘
     ▲                           │                              │
     │ Power OFF                 │ Power OFF                    │ 20 seconds
     │                           ▼                              ▼
┌─────────┐               ┌─────────────┐              ┌─────────────────┐
│ System  │               │   System    │              │ READY_TO_START  │
│Shutdown │               │  Shutdown   │              └─────────────────┘
└─────────┘               └─────────────┘                       │
                                                                │ Ignition ON
                                                                ▼
┌─────────────────┐                                    ┌─────────────────┐
│    RUNNING      │ ◀────────────────────────────────  │    STARTING     │
│                 │              Engine Started        │                 │
│ - Monitor Temp  │                                    │ - Starter Motor │
│ - Monitor Oil   │                                    │ - Max 10 sec    │
│ - Show Alerts   │                                    └─────────────────┘
└─────────────────┘
         │
         │ Safety Alert Triggered
         ▼
┌─────────────────┐         ┌─────────────────┐
│ LOW_OIL_PRESSURE│   OR    │ HIGH_TEMPERATURE│
│                 │         │                 │
│ - Flash Warning │         │ - Flash Warning │
│ - Continue Run  │         │ - Continue Run  │
└─────────────────┘         └─────────────────┘
```

## Hardware Control Points

### What the System CAN Control:
- ✅ Main Power Relay (Powers all systems)
- ✅ Glow Plug Relay (Preheating)
- ✅ Starter Motor Relay (Engine cranking)
- ✅ Front Light Relay
- ✅ Back Light Relay
- ✅ Safety Alert Display

### What the System CANNOT Control:
- ❌ Engine Stop (Manual lever only)
- ❌ Fuel Shutoff (Manual lever only)

## Safety Alert System

```
Safety Monitoring Loop:
┌─────────────────┐
│ Read Sensors:   │
│ - Oil Pressure  │  ──┐
│ - Engine Temp   │    │
│ - Battery Volt  │    │
└─────────────────┘    │
                       │
                       ▼
                ┌─────────────────┐    Alert Triggered
                │ Safety Check    │ ──────────────────┐
                │ Parameters      │                   │
                └─────────────────┘                   │
                       │                              │
                       │ All OK                       ▼
                       ▼                    ┌─────────────────┐
                ┌─────────────────┐         │ Flash Red Alert │
                │ Continue Normal │         │ on Web Interface│
                │ Operation       │         │                 │
                └─────────────────┘         │ Operator must   │
                                           │ manually stop   │
                                           │ engine with     │
                                           │ lever if needed │
                                           └─────────────────┘
```

## Power Control Logic

```
Power ON Button:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│ IDLE State  │───▶│ Main Power  │───▶│ POWER_ON    │
│             │    │ Relay ON    │    │ State       │
└─────────────┘    └─────────────┘    └─────────────┘
                                             │
                                             │ Lights, Ignition
                                             │ now available
                                             ▼
                                    ┌─────────────────┐
                                    │ Ready for       │
                                    │ Ignition or     │
                                    │ Light Control   │
                                    └─────────────────┘

Power OFF Button:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│ Any State   │───▶│ All Relays  │───▶│ IDLE State  │
│             │    │ OFF         │    │             │
└─────────────┘    └─────────────┘    └─────────────┘
                                             │
                                             │ Note: Engine
                                             │ continues running
                                             │ if started
                                             ▼
                                    ┌─────────────────┐
                                    │ Manual engine   │
                                    │ stop required   │
                                    │ with lever      │
                                    └─────────────────┘
```

## Emergency Override

The Override button bypasses all safety checks and immediately attempts to start the engine. This should only be used in emergency situations.

```
Override Button:
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│ Any State   │───▶│ Bypass All  │───▶│ STARTING    │
│             │    │ Safety      │    │ State       │
└─────────────┘    └─────────────┘    └─────────────┘
                                             │
                                             │ Confirmation
                                             │ Dialog First
                                             ▼
                                    ┌─────────────────┐
                                    │ Direct Starter  │
                                    │ Motor Engage    │
                                    └─────────────────┘
```
