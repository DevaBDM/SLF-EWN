> **Archived draft.** This note describes the early RFM95W fake radio. The built node uses Ra-02 — see [[Proteus Hub]] and [[Transmitter Simulation]].

# LoRa radio simulation (Proteus)

Early notes on the fake SPI LoRa transmitter used in Proteus for the buried node.

## 1. Overview

Modules such as the Semtech SX1276-based RFM95W are surface-mount assemblies with high-frequency layouts. To test the communication path under the THT-only rule, the radio was modelled with two chips in Proteus.

The primary ATmega328P is the SPI master and packs the sensor data. A second ATmega328P inside an isolated subcircuit is the SPI slave. It accepts the 12-byte payload through a FIFO command (`0x8D`) and prints the bytes on its own virtual terminal.

## 2. Bus and pin mapping

The two chips share one SPI bus on a power-gated 3.6 V (Li-SOCl₂) rail. Firmware inserts short gaps between bytes so the simulator does not overrun or collide.

### Wiring

```text
[Main ATmega328P - Master]              [Subcircuit fake LoRa module - Slave]
┌────────────────────────┐              ┌───────────────────────────────────┐
│         PB5 (SCK/Pin19)├─────────────►│ SCK   (SPI clock input)           │
│        PB3 (MOSI/Pin17)├─────────────►│ MOSI  (Serial data input)         │
│        PB4 (MISO/Pin18)│◄────────────┤ MISO  (Serial data output)        │
│         PB1 (SS/Pin15) ├─────────────►│ NSS   (Active-low chip select)    │
│         PD3 (INT1/Pin5)│◄────────────┤ DIO0  (Packet flag handshake)     │
└────────────────────────┘              └───────────────────────────────────┘
```

### Interface map

| Function | Master pin (U1) | Subcircuit terminal | Slave pin (U2) | Proteus I/O |
|---|---|---|---|---|
| SPI clock | Pin 19 (PB5) | `SCK` | Pin 19 (PB5) | INPUT |
| Master output | Pin 17 (PB3) | `MOSI` | Pin 17 (PB3) | INPUT |
| Slave output | Pin 18 (PB4) | `MISO` | Pin 18 (PB4) | OUTPUT |
| LoRa NSS | Pin 15 (PB1) | `NSS` | Pin 16 (PB2) | INPUT |
| Transmit complete | Pin 5 (PD3) | `DIO0` | Pin 4 (PD2) | OUTPUT |
| Local debug TXD | — | — | Pin 31 (PD1) | OUTPUT (to terminal) |

## 3. Child-sheet terminal

The serial engine in the slave is hardware-timed and runs at double speed to clear timing artefacts, baud mismatches, and the `?????` lines that appear when the terminal settings are wrong.

### Virtual terminal settings

- Baud rate: `9600` (`UBRR0L = 103` on the 8 MHz slave core).
- Data bits: `8`.
- Parity: `NONE`.
- Stop bits: `1`.
- Invert RXD / Invert TXD: `FALSE` / unchecked. Flipping either corrupts the ASCII frame.

### Internal power settings

Proteus flags sub-sheets as unpowered before the MOSFET gate turns on. Two settings bypass that:

- Chip `U2` property: `VCC=VCC`, so the subcircuit logic keeps running.
- CKDIV8 fuse: disabled / unprogrammed, so the chip runs at its native 8 MHz internal clock.

## 4. Telemetry output

The payload carries the X, Y, Z registers and the 10-bit ADC soil-moisture reading, formatted as text for the terminal.

### Parent sheet terminal (main MCU)

```text
=== INTEGRATED ENVIRONMENTAL LANDSLIDE NODE ONLINE ===

--- Starting Sensor Read Session ---
Local Readouts -> Motion[X:0, Y:0, Z:0] | Moisture: 48%
Forwarding global payload packet to LoRa Transceiver...
[STATUS] DIO0 Handshake Pin detected HIGH
```

### Child sheet terminal (fake radio)

```text
--- FAKE LORA SIMULATOR ONLINE ---
Awaiting incoming SPI radio waves from Master Node...

[RADIO TX ANT OUT] -> NODE01_DATA:X_0:Y_0:Z_0:MOIST_48%:SEQ_2
[RADIO TX ANT OUT] -> NODE01_DATA:X_0:Y_0:Z_0:MOIST_48%:SEQ_3
```

## What this validated

- The fake radio samples `SPSR & (1 << SPIF)` without write collisions or overrun warnings.
- The master inserts a `_delay_us(250)` gap between bytes, which keeps the shared bus in sync.
- The subcircuit keeps the top-level schematic readable and maps directly onto a PCB tool using the physical 16-pin RFM95W footprint.
