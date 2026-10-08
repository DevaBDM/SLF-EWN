## Technical Documentation: Subterranean Landslide Node SPI LoRa Radio Simulation

This documentation details the architecture, configuration, and validation of the Through-Hole Technology (THT) compliant custom SPI LoRa transmission simulation system designed for a ground-up subterranean early-warning network node.

---

## 1. System Overview

Modern wireless telemetry modules (such as the Semtech SX1276-based RFM95W radio module) utilize high-frequency layouts packaged exclusively as compact surface-mount technology (SMT) sub-assemblies. To evaluate the communication subsystem under strict THT physical constraints, a dual-chip architecture was modeled in Proteus.

The primary ATmega328P acts as the SPI Master, formatting and packing real-time environmental data streams. A secondary ATmega328P inside an isolated Proteus subcircuit block serves as an SPI Slave, intercepting data payloads via an active FIFO register command layer (`0x8D`) and reassembling raw SPI stream characters into standard string data over an internal terminal display layer.

---

## 2. Bus Architecture & Pin Mapping

The communication system uses a multi-peripheral shared SPI bus layout running off a power-gated 3.6V (Li-SOCl2) network rail. Paced transmission gaps are explicitly hardcoded into the Master MCU's firmware to prevent simulation data overruns or write collisions during data transfers.

## 📐 Physical Wiring Interconnections

```unset
[Main ATmega328P - Master]              [Subcircuit Fake LoRa Module - Slave]
┌────────────────────────┐              ┌───────────────────────────────────┐
│         PB5 (SCK/Pin19)├─────────────►│ SCK   (SPI Clock Input)           │
│        PB3 (MOSI/Pin17)├─────────────►│ MOSI  (Serial Data Input)         │
│        PB4 (MISO/Pin18)│◄────────────┤ MISO  (Serial Data Output)        │
│         PB1 (SS/Pin15) ├─────────────►│ NSS   (Active-Low Chip Select)    │
│         PD3 (INT1/Pin5)│◄────────────┤ DIO0  (Packet Flag Handshake)     │
└────────────────────────┘              └───────────────────────────────────┘
```

## 📋 Full Interface Interconnection Routing Table

|Function|Master Pin (U1)|Subcircuit Terminal|Slave Pin (U2)|Proteus IO Type|
|---|---|---|---|---|
|SPI Clock|Pin 19 (PB5)|`SCK`|Pin 19 (PB5)|INPUT|
|Master Output|Pin 17 (PB3)|`MOSI`|Pin 17 (PB3)|INPUT|
|Slave Output|Pin 18 (PB4)|`MISO`|Pin 18 (PB4)|OUTPUT|
|LoRa NSS Select|Pin 15 (PB1)|`NSS`|Pin 16 (PB2)|INPUT|
|Transmit Complete|Pin 5 (PD3)|`DIO0`|Pin 4 (PD2)|OUTPUT|
|Local Debug TXD|—|—|Pin 31 (PD1)|OUTPUT (Wired to Terminal)|

---

## 3. High-Precision Local Terminal Configuration (Child Sheet)

To clear historical timing artifacts, mismatched baud rates, or variable data corruption (`?????` lines) inside the Proteus engine, a double-speed hardware-timed serial engine was integrated into the Slave.

## ⚙️ Virtual Terminal Properties Specification

Ensure the diagnostic terminal block inside the child sheet matches these parameters exactly:

- Baud Rate: `9600` (Synchronized to `UBRR0L = 103` on the 8MHz slave core)
- Data Bits: `8`
- Parity: `NONE`
- Stop Bits: `1`
- Invert RXD / Invert TXD: `FALSE` / Unchecked (Flipping this values corrupts the ASCII frame formatting)

## ⚡ Critical Internal Power Configuration

To bypass the invisible subcircuit boundary isolation bug where Proteus flags internal sub-sheets as unpowered prior to active MOSFET gating transitions:

- Property String modification: The simulator chip `U2` text field properties must be set to `VCC=VCC` to force continuous logic execution.
- CKDIV8 Fuse: The "Divide clock by 8" internal fuse is set to Disabled / Unprogrammed to allow the chip to compute instructions at a stable, native 8 MHz internal frequency.

---

## 4. Telemetry Stream Output & Validation

The telemetry output maps active sensor coordinate registers (X, Y, Z axes) alongside a bare-metal 10-bit analog-to-digital converter checking soil moisture depth variables directly into the streaming radio text format.

## 📬 Parent Sheet Virtual Terminal (Main MCU Diagnostics)

```unset
=== INTEGRATED ENVIRONMENTAL LANDSLIDE NODE ONLINE ===

--- Starting Sensor Read Session ---
Local Readouts -> Motion[X:0, Y:0, Z:0] | Moisture: 48%
Forwarding global payload packet to LoRa Transceiver...
[STATUS] DIO0 Handshake Pin detected HIGH
```

## 📬 Child Sheet Virtual Terminal (Fake LoRa Radio Intercept)

```unset
--- FAKE LORA SIMULATOR ONLINE ---
Awaiting incoming SPI radio waves from Master Node...

[RADIO TX ANT OUT] -> NODE01_DATA:X_0:Y_0:Z_0:MOIST_48%:SEQ_2
[RADIO TX ANT OUT] -> NODE01_DATA:X_0:Y_0:Z_0:MOIST_48%:SEQ_3
```

---

## 5. Summary Technical Accomplishments

- Asynchronous Processing: The Fake LoRa state machine samples data inputs using a high-efficiency `SPSR & (1 << SPIF)` flag layout that completely avoids write collisions and data overrun warnings.
- Paced SPI Serialization: The Master MCU enforces a `_delay_us(250)` padding gap inside character loops, ensuring full synchronization across the shared data highway.
- Modular Verification: Subcircuit encapsulation keeps the top-level schematic diagram optimized, paving a direct translation path into hardware design suites (such as KiCad or EasyEDA) using the physical 16-pin RFM95W footprints.