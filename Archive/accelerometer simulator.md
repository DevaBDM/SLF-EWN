> **Archived draft.** This note describes the early LIS3DH fake accelerometer. The built node uses ADXL345 — see [[Proteus Hub]] and [[Transmitter Simulation]].

# Accelerometer simulation (Proteus)

Early notes on the fake 3-axis accelerometer used in Proteus for the buried node.

## 1. Overview

MEMS digital accelerometers such as the STMicroelectronics LIS3DH only come in surface-mount packages (LGA-16). To test the firmware under the THT-only rule before the board existed, the sensor was modelled with two chips in Proteus.

A primary ATmega328P is the SPI master. A second ATmega328P, inside an isolated Proteus subcircuit, is the SPI slave. It reproduces the LIS3DH register map, response bytes, and timing over SPI Mode 3.

## 2. Bus and pin mapping

The two chips share one SPI bus at 3.6 V (Li-SOCl₂). Power gating is included so the simulation matches the 3–7 year battery target: a P-channel MOSFET switches the sensor rail from a bare-metal GPIO.

### Wiring

```text
[Main ATmega328P - Master]              [Subcircuit fake accelerometer - Slave]
┌────────────────────────┐              ┌─────────────────────────────────────┐
│          PB5 (SCK/Pin19)├─────────────►│ SCL / SPC (SPI clock input)         │
│         PB3 (MOSI/Pin17)├─────────────►│ SDA / SDI (Serial data input)       │
│         PB4 (MISO/Pin18)│◄────────────┤ SDO (Serial data output)            │
│          PB1 (SS/Pin15) ├─────────────►│ CS (Active-low chip select)         │
│         PD2 (INT0/Pin4) │◄────────────┤ INT1 (Interrupt output)             │
│         PB0 (GPIO/Pin14)├┐            │                                     │
└────────────────────────┘│             └─────────────────────────────────────┘
                          │             ┌─────────────────────────────────────┐
                          └────────────►│ Source -> gate [ZVP2106A P-MOSFET]  │
                                        └─────────────────────────────────────┘
```

### Interface map

| Function | Master pin (U1) | Subcircuit terminal | Slave pin (U3) | Mode |
|---|---|---|---|---|
| SPI clock | Pin 19 (PB5) | `SCL` | Pin 19 (PB5) | Master out / slave in |
| Master output | Pin 17 (PB3) | `SDA` | Pin 17 (PB3) | Master out / slave in |
| Slave output | Pin 18 (PB4) | `SDO` | Pin 18 (PB4) | Slave out / master in |
| Chip select | Pin 15 (PB1) | `CS` | Pin 16 (PB2) | Active-low pulse |
| Hardware alarm | Pin 4 (PD2) | `INT1` | Pin 4 (PD2) | Active-low interrupt |
| Power gate | Pin 14 (PB0) | — | VCC rail | MOSFET drain |

## 3. Child-sheet controls

The child sheet has six digital inputs so each simulated axis can be moved on its own:

- X axis: PD4 (+) and PD5 (−).
- Y axis: PD6 (+) and PD7 (−).
- Z axis: PB0 (+) and PC3 (−).
- Each button has a 10 kΩ THT axial pull-up to stop floating inputs in the simulator.
- A 150 ms software poll (`_delay_ms(150)`) debounces the buttons and stops register drift from multiple triggers.

## 4. Driver logic

The firmware is bare-metal C on avr-libc. It avoids the larger Arduino layers and keeps RAM use inside the ATmega328P limits.

### Slave state machine

To clear the `SPDR Write Collision` and `Data Overrun` warnings in Proteus, the slave uses a non-blocking flag check:

1. It polls the `SPSR` `SPIF` flag in the main loop.
2. When an address byte arrives, it extracts the 6-bit index (`addr & 0x3F`).
3. If bit 7 of the command byte is set (`0x80`, a read), the slave preloads `SPDR` with the requested byte before the master sends the next clock. That keeps the data shift clean.

### Identity check (`WHO_AM_I`)

At boot the master reads register `0x0F`. The fake returns `0x33`, the LIS3DH factory signature. Main execution blocks until the check passes, so a bad sensor read cannot enter the packet.

## 5. Verification

Telemetry goes to a Proteus Virtual Terminal at 9600 baud (8-N-1) on master pin PD1 (TXD).

```text
--- LANDSLIDE NODE DEBUG BOOT ---
Powering on MOSFET rail... ON
SPI Bus Master Status... READY
Pinging Accelerometer WHO_AM_I... SUCCESS (ID: 0x33 Verified)
X:    0 | Y:    0 | Z:    0
X:   50 | Y:    0 | Z:    0
X:  100 | Y:  -50 | Z:    0

[ALERT] Hardware Motion Interrupt Triggered on INT1!
```

## What this validated

- The Proteus pipeline runs without collision warnings.
- The subcircuit keeps the parent schematic readable and the wiring maps directly onto a PCB tool.
- Pulling `PB0` high drops the sensor rail to zero in the schematic, which matches the intended sleep behaviour of the node.
