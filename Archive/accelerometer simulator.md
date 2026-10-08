## Technical Documentation: Subterranean Landslide Node SPI Accelerometer Simulation

This documentation details the successful implementation, architecture, and validation of the Through-Hole Technology (THT) compliant 3-axis accelerometer simulation system designed for a ground-up subterranean landslide warning network node.

---

## 1. System Overview

Due to physical constraints—primarily that modern MEMS digital accelerometers (such as the STMicroelectronics LIS3DH) are manufactured exclusively in surface-mount (SMD) packages (e.g., LGA-16)—a dual-chip prototyping strategy was developed for verification in Proteus.

A primary ATmega328P operates as the SPI Master, while a secondary ATmega328P within an isolated Proteus subcircuit acts as an SPI Slave, accurately mimicking the hardware registers, response codes, and timing criteria of a physical LIS3DH sensor over an SPI bus in Mode 3.

---

## 2. Bus Architecture & Pin Mapping

The system relies on a unified, shared SPI bus architecture operating at a system voltage of 3.6V (Li-SOCl2). To ensure an optimized power footprint matching the 3-to-7 year battery longevity goal, the system includes power-gating controlled by a bare-metal driven P-channel MOSFET.

## 📐 Physical Wiring Interconnections

```unset
[Main ATmega328P - Master]              [Subcircuit Fake Accelerometer - Slave]
┌────────────────────────┐              ┌─────────────────────────────────────┐
│          PB5 (SCK/Pin19)├─────────────►│ SCL / SPC (SPI Clock Input)         │
│         PB3 (MOSI/Pin17)├─────────────►│ SDA / SDI (Serial Data Input)       │
│         PB4 (MISO/Pin18)│◄────────────┤ SDO (Serial Data Output)            │
│          PB1 (SS/Pin15) ├─────────────►│ CS (Active-Low Chip Select)         │
│         PD2 (INT0/Pin4) │◄────────────┤ INT1 (Primary Interrupt Output)     │
│         PB0 (GPIO/Pin14)├┐            │                                     │
└────────────────────────┘│             └─────────────────────────────────────┘
                          │             ┌─────────────────────────────────────┐
                          └────────────►│ Source -> Gate [ZVP2106A P-MOSFET]  │
                                        └─────────────────────────────────────┘
```

## 📋 Full Interface Interconnection Register Map

|Function|Master Pin (U1)|Subcircuit Terminal|Slave Pin (U3)|Functional Mode|
|---|---|---|---|---|
|SPI Clock|Pin 19 (PB5)|`SCL`|Pin 19 (PB5)|Master Out / Slave In|
|Master Output|Pin 17 (PB3)|`SDA`|Pin 17 (PB3)|Master Out / Slave In|
|Slave Output|Pin 18 (PB4)|`SDO`|Pin 18 (PB4)|Slave Out / Master In|
|Chip Select|Pin 15 (PB1)|`CS`|Pin 16 (PB2)|Active-Low Pulse Gate|
|Hardware Alarm|Pin 4 (PD2)|`INT1`|Pin 4 (PD2)|Active-Low Interrupt|
|Power Gate|Pin 14 (PB0)|—|VCC Rail|MOSFET Drain Connection|

---

## 3. Subcircuit Interaction Mechanics (Child Sheet Controls)

To facilitate intuitive environmental validation inside the Proteus workspace, the child sheet encapsulates six distinct digital inputs grouped to modify simulated physical displacement variables independently.

- X-Axis Control Layer: Driven by pins PD4 (+) and PD5 (-). Adjusts virtual roll values.
- Y-Axis Control Layer: Driven by pins PD6 (+) and PD7 (-). Adjusts virtual pitch values.
- Z-Axis Control Layer: Driven by pins PB0 (+) and PC3 (-). Adjusts raw gravitational vectors.
- Electrical Stability: Every interaction button features a 10kΩ THT axial pull-up resistor to prevent floating states in the simulation engine.
- Debounce Window: Software-implemented polling buffers (`_delay_ms(150);`) guarantee clean data increments, preventing multi-trigger register drift.

---

## 4. Driver Logic & Firmware Architecture

The implementation uses strict bare-metal C targeting avr-libc to fulfill strict memory allocation limits and skip bloated prototyping layers.

## 🔬 Asynchronous Slave State Machine

To completely resolve `SPDR Write Collision` and `Data Overrun` warnings inside Proteus, the slave utilizes a non-blocking hardware flag interrogation layout:

1. It constantly scans the `SPSR` register's `SPIF` flag instead of looping inside an isolated thread.
2. The instant an address byte is received, the slave extracts the 6-bit bounded index (`addr & 0x3F`).
3. If bit 7 of the command byte is asserted (`0x80`), indicating an active READ command, the slave preloads its `SPDR` register with the target byte data _before_ the master finishes preparing its next clock pulse. This yields error-free, microsecond-accurate data shifts.

## 📬 Identity Validation (`WHO_AM_I`)

Upon system boot, the Master issues a read request to register address `0x0F`. The Fake Accelerometer firmware returns an immutable byte value of `0x33` (the standard factory signature for LIS3DH devices). Main execution blocks until this validation is successful, protecting the network from corrupt sensor reads.

---

## 5. Verification & Telemetry Output

Telemetry capture is routed to a Proteus Virtual Terminal instruments block set to 9600 Baud (8-N-1) wired directly to Master pin PD1 (TXD).

```unset
--- LANDSLIDE NODE DEBUG BOOT ---
Powering on MOSFET rail... ON
SPI Bus Master Status... READY
Pinging Accelerometer WHO_AM_I... SUCCESS (ID: 0x33 Verified)
X:    0 | Y:    0 | Z:    0
X:   50 | Y:    0 | Z:    0
X:  100 | Y:  -50 | Z:    0

[ALERT] Hardware Motion Interrupt Triggered on INT1!
```

## 🏆 Operational Accomplishments

- Zero Log Conflicts: Implementation completely clears out historical Proteus pipeline collision warnings.
- Hardware Isolation: The subcircuit design keeps the parent schematic layout exceptionally clean, making it simple to transfer directly to your PCB design tools (like KiCad or EasyEDA). [1]
- Power Conservation Validation: Pulling `PB0` high explicitly drops the sensor rail to zero volts in the schematic, validating the node's long-term field survivability.
