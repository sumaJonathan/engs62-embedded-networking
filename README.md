# ENGS 62: Embedded Networking (Railway Crossing Controller)
## Overview

This project involved designing and implementing an embedded railway crossing control system using the Zybo 7010 development board. It combined finite state machine (FSM) design with event-driven programming principles, and included both local hardware control and remote communication with a centralized substation server.

## Learning Objectives

- Understand and apply event-driven programming.
- Design and implement finite state machines (FSMs).
- Develop control systems using event-driven FSMs.
- Communicate with a remote server using UART-based messaging.
- Integrate peripherals such as LEDs, pushbuttons, servo motors, potentiometers, and switches.

## System Description

The system models a real-world railway crossing and manages:
- Traffic light signals (red-yellow-green).
- A pedestrian crossing system with two pushbuttons.
- A gate controlled by a servo motor.
- A maintenance mode activated by a key or remotely.
- Communication with a substation that informs the controller of train status and remote maintenance commands.

## Features

- **Finite State Machine (FSM):** Manages various states such as normal traffic flow, pedestrian crossing, train arrival, train clear, and maintenance.
- **LED Indicators:**
  - RGB LEDs simulate traffic lights.
  - Blue LED flashes during maintenance.
- **Servo Motor:** Controls the gate arm (90-degree swing).
- **Pushbuttons:** Simulate pedestrian crossing buttons.
- **Switches:** Simulate train arrival/clear and manual maintenance activation.
- **Potentiometer:** Simulates the manual gate control wheel.
- **OLED Display / Terminal:** Displays current system status such as gate state, maintenance mode, and train status.
- **Timer-Based Logic:** Ensures accurate timing for traffic flow, pedestrian crossing, and maintenance delay.


### FSM Overview

The system's FSM handles transitions between the following states:
- `TRAFFIC_ON`: Green light, cars allowed to pass.
- `YELLOW`: Warning signal before stopping traffic.
- `PEDESTRIAN`: Pedestrians allowed to cross for 30 seconds.
- `TRAIN_COMING`: Gate closes and red light turns on.
- `TRAIN_GONE`: Waits 10 seconds after train passes before resuming traffic.
- `MAINTENANCE`: Gate is manually closed; blue light flashes.

### Communication Protocol
The system uses `UDP` to communicate with a remote substation server performing the following operations:
- Check train status periodically using an `UPDATE` message (with id = 0).
- Receive global maintenance commands.
- Echo `PING` messages for connectivity checks.

Messages follow a structured packet format and are sent/received via `uart_0_handler` and `uart_1_handler`.

### Substation Interaction
- A provided Linux program (`substation.c`) allows developers to simulate train arrival and maintenance commands.
- The embedded system polls the substation 10 times per second to detect train arrival or maintenance requests.
- The substation response is decoded and used to transition system states accordingly.

### Timing & Precision
- Traffic green light minimum: **10 seconds** (replaces 3 minutes)
- Pedestrian cross time: **10 seconds** (replaces 20 seconds)
- Timing managed by **TTC** with accuracy up to **1/10th of a second**

## Hardware Setup
- **Zybo Z7-10 board**
- **RGB and yellow LEDs** for traffic and maintenance signals
- **Pushbuttons** for pedestrian input
- **Switches** to simulate train arrival/clear and key insertion
- **Servo motor** for the crossing gate
- **Potentiometer** simulates manual gate wheel
- **I2C Display** and **UART terminal** for system status feedback
---

## Testing & Verification

- FSM was verified through simulations and live hardware testing.
- PING messages confirmed communication with the substation.
- Server responses were decoded and handled appropriately.
- System behavior was observed in all expected states (normal traffic, pedestrian crossing, train arriving, maintenance).

## Conclusion

This project provided hands-on experience in building a real-time embedded system using FSMs and event-driven design. It integrated both hardware and software elements and included networking capabilities through UART-based messaging and interaction with a centralized substation.

---
