# Subterranean Landslide and Flood Early-Warning Network: A Long-Term Buried Sensor Telemetry System

**A Project Proposal Submitted in Partial Fulfillment of the Requirements for the Degree of Bachelor of Science in Electrical and Computer Engineering**

---

**Project Team:**  
[Student Name(s)]  
Department of Electrical and Computer Engineering  
[University Name]  
[City, Country]  
Email: [email@university.edu]

**Supervisor:**  
[Supervisor Name], [Title]  
Department of Electrical and Computer Engineering  
[University Name]

**Date of Submission:** August 2026

---

## Abstract

Landslides and flash floods represent persistent threats to rural communities, mining operations, and civil infrastructure, yet conventional monitoring systems rely predominantly on surface-level instrumentation that detects failure only after ground collapse has initiated. We propose the design and implementation of a **Subterranean Landslide and Flood Early-Warning Network (SLF-EWN)**—a distributed system of autonomous, battery-powered sensor nodes buried at a depth of one meter, capable of operating maintenance-free for three to seven years. Each node continuously monitors subsurface soil moisture and multi-axis ground movement using ultra-low-power sensors, and transmits telemetry through soil via 433 MHz LoRa radio to a surface tower receiver. The tower receiver aggregates data from five or more nodes into a local SPI NOR flash database and forwards telemetry to a native desktop dashboard over USB-UART. The system employs aggressive power management through deep-sleep hibernation, sensor power-gating via a through-hole P-channel MOSFET, and a pre-flashed ATmega328P microcontroller architecture to eliminate on-board programming headers and minimize form factor. We detail the complete hardware design, custom through-hole PCB layout strategy, embedded firmware architecture, Qt 6 / QML-based visualization interface, compact cylindrical enclosure, Proteus simulation validation workflow, and manufacturing process. The proposed system offers a cost-effective, scalable, and robust early-warning solution for landslide-prone regions and structural monitoring applications.

**Keywords:** Landslide early warning, subsurface sensing, LoRa telemetry, ultra-low-power embedded systems, soil moisture monitoring, accelerometer tilt detection, ATmega328P, bare-metal firmware, SPI NOR flash, Qt 6, QML, buried sensor network.

---

## I. Introduction

### A. Problem Statement

Natural disasters involving landslides and flash floods claim thousands of lives annually and cause billions of dollars in infrastructure damage worldwide [1]. Rural mountainous communities, open-pit mining excavations, and large-scale civil construction sites are particularly vulnerable due to unpredictable soil saturation and ground movement. Traditional monitoring approaches—such as surface rain gauges, inclinometers, or satellite imagery—suffer from critical limitations: they detect precursors indirectly, require extensive surface cabling vulnerable to environmental damage, and provide insufficient lead time for evacuation or intervention [2].

Subsurface conditions, particularly soil moisture accumulation and internal ground deformation, are the most direct indicators of impending slope failure. However, embedding electronic systems underground introduces severe engineering challenges: limited power budgets, waterproofing requirements, soil pressure and corrosion, and the difficulty of wireless signal propagation through dense earth [3]. Existing commercial solutions are often prohibitively expensive, require frequent maintenance, or lack the low-power autonomy necessary for long-term burial.

### B. Project Objectives

We aim to design, prototype, and validate a complete early-warning system addressing the following objectives:

1. **Autonomous Subterranean Operation:** Develop sensor nodes capable of continuous buried operation for 3–7 years without battery replacement or physical maintenance.
2. **Direct Hazard Detection:** Measure subsurface moisture saturation and three-axis ground tilt/movement as primary indicators of structural instability.
3. **Through-Earth Wireless Communication:** Transmit data reliably from a depth of one meter through soil, clay, and sand without physical cabling.
4. **Multi-Node Aggregation:** Deploy a surface tower receiver capable of collecting, timestamping, and persistently storing telemetry from five or more buried nodes in a local non-volatile database.
5. **Real-Time Visualization:** Provide a standalone desktop application that maps node locations, displays live telemetry, and triggers visual and textual alarms when safety thresholds are exceeded.
6. **Compact and Deployable Form Factor:** Package the electronics into a rugged, waterproof, cylindrical enclosure optimized for burial and environmental resilience.

### C. Scope and Contributions

This project encompasses the full engineering lifecycle: schematic design, custom PCB development, embedded firmware programming, radio communication validation, desktop software engineering, mechanical enclosure design, Proteus circuit simulation, and system integration testing. Our primary technical contributions include:

- A **pre-flashing manufacturing workflow** that eliminates ICSP headers from the final PCB, reducing size and eliminating the risk of 5V back-feeding into the 3.6V battery circuit.
- A **dual-side PCB stacking strategy** paired with a vertical cylindrical enclosure to minimize burial footprint and maximize structural integrity against soil pressure.
- A **smart-sleep firmware architecture** achieving 99.9% hibernation duty cycle with sub-50 ms active measurement windows and interrupt-driven emergency wake-up.
- A **tower receiver architecture** using SPI NOR flash (Winbond W25Q64/W25Q128) as a local database, enabling 24/7 autonomous data logging independent of PC connection.
- A **Qt 6 / QML native desktop dashboard** with live serial acquisition, SQLite caching, admin/guest authentication, simulation mode, and binary + ASCII protocol support.
- A **Proteus simulation framework** using secondary ATmega328P subcircuits to emulate the LIS3DH accelerometer and RFM95W LoRa module for pre-hardware firmware validation.

---

## II. System Architecture and Overview

### A. High-Level Block Diagram

The complete system consists of three primary subsystems: the **Buried Sensor Node**, the **Surface Tower Receiver**, and the **Desktop Dashboard**.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SYSTEM ARCHITECTURE                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ┌──────────────────────┐         433 MHz LoRa         ┌───────────────┐  │
│   │   BURIED SENSOR NODE │◄────────────────────────────►│ SURFACE TOWER │  │
│   │   (1m Underground)   │      Through-Earth (5+)      │   RECEIVER    │  │
│   └──────────────────────┘                              └───────┬───────┘  │
│          │                                                        │         │
│          ▼                                                        ▼         │
│   ┌──────────────┐                                    ┌─────────────────┐   │
│   │ Soil Moisture│                                    │  ATmega328P     │   │
│   │   Sensor     │                                    │  RFM95W (RX)    │   │
│   ├──────────────┤                                    │  W25Q64 Flash   │   │
│   │   LIS3DH     │                                    │  FT232RL USB    │   │
│   │Accelerometer │                                    └────────┬────────┘   │
│   ├──────────────┤                                             │            │
│   │ ATmega328P   │                                    ┌────────▼────────┐   │
│   │Microcontroller                                    │  DESKTOP GUI    │   │
│   ├──────────────┤                                    │  (Qt 6 / QML)   │   │
│   │ RFM95W (TX)  │                                    │  • Map View     │   │
│   │ LoRa Module  │                                    │  • Telemetry    │   │
│   ├──────────────┤                                    │  • Alerts       │   │
│   │ Li-SOCl2 +   │                                    │  • SQLite Store │   │
│   │Supercapacitor│                                    └─────────────────┘   │
│   └──────────────┘                                                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### B. Operational Workflow

Each buried node follows a deterministic six-step operational loop:

1. **Deep Sleep (Hibernation):** For 99.9% of its operational lifetime, the node rests in a powered-down state. The ATmega328P disables all peripheral clocks, and the P-channel MOSFET cuts power to sensors and the radio.
2. **Wake-Up Trigger:** An internal watchdog timer triggers a scheduled wake-up every 15 minutes. Alternatively, the LIS3DH motion sensor generates an interrupt that forces an immediate emergency wake-up if ground movement exceeds a calibrated threshold.
3. **Sensor Activation:** The microcontroller enables the MOSFET gate, powering the moisture sensor and accelerometer. Analog-to-digital conversion and SPI communication occur simultaneously.
4. **Data Acquisition:** Soil capacitance and three-axis acceleration/tilt values are sampled and compressed into a compact telemetry packet containing a unique 16-bit Device ID.
5. **Radio Transmission:** The RFM95W LoRa module broadcasts the packet at 433 MHz through the soil layer to the surface tower. Transmission duration is minimized to conserve energy.
6. **Power Down:** All peripherals are de-energized, and the microcontroller returns to deep sleep until the next cycle.

The surface tower receiver operates in **continuous RX mode**. When a packet arrives:

1. The RFM95W raises the DIO0 interrupt pin.
2. The ATmega328P reads the packet buffer via SPI.
3. The payload is appended with a timestamp and written to the W25Q64 SPI NOR flash database.
4. Simultaneously, the raw byte stream is forwarded to the host PC via the FT232RL USB-UART bridge at 115200 baud.
5. The receiver returns to RX_CONTINUOUS mode.

When the desktop PC is connected, the dashboard streams live data and can request historical dumps from the flash database. When the PC is disconnected, the tower continues logging autonomously.

### C. Multi-Node Collision Avoidance

Because the tower employs a single RFM95W radio module, it can only demodulate one transmission at a time. To support five or more buried nodes without packet collision, we implement a **time-division with random jitter** protocol:

- Each node sleeps for its nominal interval (15 minutes) plus a pseudo-random delay of 0–30 seconds derived from its unique Device ID.
- Transmission airtime is kept under 500 ms.
- Emergency interrupts bypass the jitter and transmit immediately, accepting a small probability of collision during rare seismic events.
- The LoRa spreading factor (SF7–SF9) and bandwidth (125 kHz) are selected to ensure each packet occupies the channel for the minimum necessary duration.

---

## III. Hardware Design

### A. Microcontroller and Core Logic

We selected the **ATmega328P** as the central processing unit for the transmitter node, surface tower receiver, and Proteus simulation slaves. This 8-bit AVR microcontroller offers several advantages for our application:

- **Mature ultra-low-power modes:** Power-down sleep with watchdog timer wake-up achieves sub-microamp quiescent current.
- **Wide operating voltage:** Functional from 1.8 V to 5.5 V, compatible with our 3.6 V Li-SOCl2 battery system.
- **Sufficient I/O and interfaces:** 23 programmable GPIO pins, one SPI bus (for LoRa, accelerometer, and flash), and a 10-bit ADC (for soil moisture analog reading).
- **DIP-28 package availability:** Enables socket-mounted assembly for pre-flashing and field serviceability.

We operate the ATmega328P at **8 MHz internal RC oscillator** (no external crystal), eliminating two components and reducing power consumption during active cycles. The internal fuses are configured to disable the Brown-Out Detector (BOD) during sleep, as the Li-SOCl2 chemistry provides a stable voltage profile throughout its discharge curve.
![[atmega328p.png]]
### B. Sensor Subsystem

#### 1) Subsurface Moisture Detection

Soil moisture is measured using a **Capacitive Soil Moisture Sensor v1.2**. Unlike resistive sensors that corrode through electrolysis when DC current passes through soil, the capacitive blade uses a high-frequency oscillator to measure the dielectric permittivity of the surrounding earth—a proxy for water content. The sensor outputs an analog voltage (0–3.3 V) proportional to moisture level, read directly by the ATmega328P ADC.

The probe is electrically insulated from the PCB and connected via direct soldered wires to through-hole pads on the PCB edge. This eliminates bulky screw terminals and reduces the enclosure diameter. The sensor is powered exclusively during the brief measurement window via the P-MOSFET gate, ensuring zero quiescent draw during hibernation.
![[capacitive soil moisture sensor.png]]

#### 2) Ground Movement and Tilt Tracking

We employ the **LIS3DH** digital tri-axis accelerometer for motion detection. We selected the LIS3DH over the ADXL345 for its superior power efficiency:

- **Ultra-low power:** ~2 µA in low-power mode, compared to ~23 µA for the ADXL345—a critical factor for achieving the 3-to-7-year battery target.
- Programmable activity/inactivity thresholds with interrupt output pins (INT1).
- ±2g/±4g/±8g/±16g selectable measurement ranges.
- SPI and I²C digital interfaces.
![[LIS3DH.png]]

The LIS3DH is configured with an activity threshold of approximately 0.5g on any axis. When ground shift or vibration exceeds this level, the INT1 pin toggles, generating an external interrupt on the ATmega328P (INT0/INT1). This bypasses the 15-minute sleep timer and forces immediate emergency telemetry transmission. The device identity register (`WHO_AM_I` at `0x0F`) returns `0x33`, which the firmware validates on every boot to detect sensor disconnection or SPI bus faults.

#### 3) Power Gating Circuit

To achieve multi-year battery life, sensors and the radio must be completely de-energized during sleep. We implement power gating using a **ZVP2106A P-Channel MOSFET** in a **TO-92 through-hole package** in the high-side supply path. The ATmega328P drives the MOSFET gate low (via a GPIO pin) to connect the 3.6 V rail to the sensor and radio subsystems. During sleep, the gate is held high, creating an open circuit with leakage current in the nanoampere range. The TO-92 package was chosen specifically for through-hole compatibility, avoiding the surface-mount constraints of the AO3401 (SOT-23).
![[pmos real part.png]]

### C. Wireless Communication

#### 1) LoRa Module Selection

The **Lora Module Ra-02** operates in the **433 MHz ISM band**. We selected this frequency specifically for through-earth propagation:

- **Lower attenuation:** Sub-1 GHz signals experience significantly lower path loss through soil and water-logged clay compared to 2.4 GHz alternatives.
- **Long range:** LoRa modulation provides high receiver sensitivity (down to −148 dBm), enabling reliable communication from 1 m depth even with modest transmit power (14–20 dBm).
- **Low duty cycle:** The transmitter is active for less than 500 ms per 15-minute cycle, keeping average current draw minimal.
![[Ra-02 vector4engineering.png]]
![[lora with antena connected.png]]

#### 433 MHz Antenna Calculations

##### 1. The Core Formula

An antenna's size depends on its electrical wavelength (λ). For a quarter-wave (λ/4) monopole antenna:

$$\lambda/4 = \frac{c}{f \times 4} \times VF$$

- c = Speed of light (3 × 10⁸ m/s)
- f = Target Frequency (433 × 10⁶ Hz)
- VF = Velocity Factor of copper wire (≈ 0.95)

$$\text{Length} = \frac{300,000,000}{433,000,000 \times 4} \times 0.95 = \mathbf{16.45 \text{ cm}}$$

---

##### 2. Environmental Impact (Why Air Enclosures Matter)

When radio waves enter dense materials, they slow down. The wavelength shrinks based on the material's Dielectric Constant ($\varepsilon_r$):

$$\lambda_{\text{medium}} = \frac{\lambda_{\text{air}}}{\sqrt{\varepsilon_r}}$$

- Air pocket ($\varepsilon_r = 1$): Antenna length stays 16.5 cm. Stable performance.
- Wet Clay ($\varepsilon_r = 30$): Wavelength shrinks by ≈ 80%. Length drops to ≈ 3 cm.

_Crucial Rule:_ Directly burying bare wire causes changing moisture levels to constantly shift $\varepsilon_r$, detuning the antenna. Keep the antenna in an air-filled plastic enclosure so it stays resonant at 16.5 cm.

---

##### 3. Compacting to Fit a 10cm Box (Helical Helix)

To fit a 16.5 cm electrical length into your 10 cm charger-box enclosure, wind the wire into an inductor coil:

- Wire Length: Cut exactly 17.0 cm of solid copper wire (extra 0.5 cm for soldering).
- Coil Diameter: Wind tightly around a 5 mm rod or drill bit.
- Turns: 18 to 22 close turns.
- Final Physical Height: ≈ 3.5 cm (perfectly fits your 10 cm space).

Would you like the exact 3D spacing layout to position this coil on your 50x50mm PCB for the best signal performance?

#### 2) THT Radio Interface

To maintain strict through-hole technology (THT) compliance across the entire PCB, the RFM95W module is mounted on **2.54 mm pitch female pin headers** (`HDR-F-2.54`). The module plugs into these sockets rather than being soldered directly. This protects the sensitive radio chip from soldering heat and allows instant module swap if damage occurs. The tower receiver PCB uses an identical header arrangement for its RFM95W.
![[Male pin headers.png]]
#### 3) Antenna Integration

To conserve vertical headspace inside the cylindrical enclosure, we use a **through-hole SMA female connector** (90-degree right-angle or straight, `SMA-TH` or `SMA-JW-P`) mounted on the PCB edge. This provides five robust through-hole pins (one central signal, four chassis ground legs) and allows screw-on attachment of a standard LoRa rubber-duck antenna. The trace between the RFM95W ANT pin and the SMA central pin is kept as short and straight as possible to minimize signal loss.
![[antena.png]]

### D. Power Subsystem

#### 1) Primary Battery

The node is powered by a **3.6 V Li-SOCl2 (Lithium Thionyl Chloride) primary cell**. Li-SOCl2 chemistry was chosen for:

- **Highest energy density among primary batteries:** Up to 710 Wh/kg, enabling small physical size.
- **Extremely low self-discharge:** <1% per year at 20 °C, critical for multi-year deployments.
- **Wide temperature range:** Operational from −60 °C to +85 °C, suitable for varying underground thermal conditions.
- **Flat discharge curve:** Maintains stable voltage until end-of-life, simplifying power management.
![[li-socl battery.png]]
#### 2) Energy Buffering

LoRa transmission creates brief current surges (up to 120 mA during RF power amplifier activation) that exceed the continuous current capability of small Li-SOCl2 cells. We mitigate this with a **47 mF EDLC supercapacitor** in series with a **1.2 kΩ current-limiting resistor**. The series resistor caps the inrush to **3.0 mA** when the cap is discharged ($I_\text{max} = 3.6\text{ V}/1.2\text{ k}\Omega$), which is safe for the Li-SOCl₂ chemistry. The RC time constant is $\tau = R \cdot C = 56.4\text{ s}$, giving a full recharge window of **4.7 minutes** ($5\tau = 282\text{ s}$), well within the 15-minute wake cycle. The 47 mF value limits the voltage dip to **127 mV** during the 120 mA × 50 ms burst ($\Delta V = \Delta Q/C = 0.006/0.047$), keeping the rail at **3.47 V** — well above the 2.7 V BOD threshold.

#### 3) Master Power Disconnect

A **2-pin male header with removable plastic jumper shunt** serves as the master power switch. Removing the jumper creates a physical open circuit between the battery and the PCB. This is essential for:

- Preventing battery drain during storage, shipping, and pre-deployment testing.
- Ensuring safe handling during final assembly and socket insertion.
- Eliminating the need for mechanical slide switches, which are bulkier and less reliable in dirty environments.

### E. Tower Receiver Hardware

The surface tower is built on a separate THT PCB and serves as the aggregation gateway between the buried nodes and the desktop dashboard.

#### 1) Core Components

| Component Function | Component Name                                         | EasyEDA Search Keyword | Mounting Style          |
| ------------------ | ------------------------------------------------------ | ---------------------- | ----------------------- |
| Main Controller    | ATmega328P-PU                                          | `ATMEGA328P-PU`        | DIP-28 IC Socket        |
| Radio Receiver     | RFM95W Breakout                                        | `RFM95W Breakout`      | 2.54 mm Female Headers  |
| Database Storage   | Winbond W25Q64 (8 MB) or W25Q128 (16 MB) SPI NOR Flash | `W25Q64` / `W25Q128`   | DIP-8 or SOIC-8 adapter |
| PC USB Interface   | FT232RL USB-to-Serial Module                           | `FT232RL Module`       | 2.54 mm Female Headers  |
| Antenna Port       | Through-Hole SMA Female                                | `SMA-TH`               | Through-Hole 5-Pin Jack |
| Voltage Regulator  | LM1117-3.3 (TO-220)                                    | `LM1117-3.3`           | Through-Hole            |

#### 2) Shared SPI Bus Architecture

The ATmega328P, RFM95W, and W25Q64 flash all communicate over the same SPI bus, using dedicated chip-select lines to arbitrate access:

- **MOSI (Pin 17):** Shared data line to RFM95W MOSI and W25Q64 DI.
- **MISO (Pin 18):** Shared data line from RFM95W MISO and W25Q64 DO.
- **SCK (Pin 19):** Shared clock line to RFM95W SCK and W25Q64 CLK.
- **Radio CS (Pin 14 / Digital 8):** RFM95W NSS (active low).
- **Flash CS (Pin 15 / Digital 9):** W25Q64 CS (active low).
- **DIO0 Interrupt (Pin 4 / INT0):** RFM95W packet-ready interrupt.

#### 3) Power Infrastructure

The FTDI USB module provides 5 V from the PC. This is regulated down to 3.3 V by the LM1117-3.3 linear regulator to supply the ATmega328P, RFM95W, and W25Q64. A 10 µF electrolytic capacitor and 0.1 µF ceramic disc capacitor are placed adjacent to the regulator output to stabilize the rail against USB current fluctuations.

#### 4) Database Behavior

In **standalone mode** (PC disconnected), the tower receiver runs continuously. When a sensor node transmits, the RFM95W wakes the ATmega328P via DIO0. The MCU reads the packet and appends it to a circular log structure in the W25Q64 flash memory, including a timestamp counter and node ID. The W25Q64 offers 8 MB (or 16 MB) of non-volatile storage—sufficient for millions of readings from dozens of nodes over multiple years.

In **dashboard sync mode** (PC connected), the desktop software sends a dump request over the serial link. The ATmega328P reads the flash log sector-by-sector and streams historical data to the GUI for charting and export.

### F. Custom PCB Design

Both the transmitter and receiver PCBs are designed in **EasyEDA** (Standard Edition) and fabricated with standard FR-4 substrate. Our layout adheres to the following design rules:

#### 1) Decoupling and Power Integrity

- **Through-hole ceramic disc capacitors (0.1 µF)** placed within 5 mm of the ATmega328P VCC and AVCC pins to suppress voltage droop during SPI and radio bursts.
- **Through-hole electrolytic capacitors (10–100 µF)** placed adjacent to the RFM95W VCC pin to buffer transmission current spikes.
- **Wide power traces (30–40 mil)** routed from the battery/supercapacitor junction to the MOSFET and radio module, capable of handling >200 mA transient currents.
- **Slim signal traces (10 mil)** for digital logic and sensor I/O to conserve board area.

#### 2) Ground Plane and EMI Mitigation

- **Solid copper ground planes** on both top and bottom layers, stitched with vias every 5 mm.
- The ground plane acts as an RF shield between the LoRa module (high-frequency digital) and the analog soil moisture sensor traces, preventing noise coupling into ADC readings.

#### 3) Socket-Mounted Architecture

Rather than soldering the ATmega328P directly to the board, we solder a **DIP-28 IC socket** to the PCB footprint. This decision is central to our manufacturing workflow:

- **Pre-flashing compatibility:** The chip is programmed on an external jig before insertion.
- **Field serviceability:** Firmware updates or debugging during academic testing require only an IC extractor tool—no desoldering.
- **Risk mitigation:** Eliminates the need for an on-board ICSP header, reducing PCB size and preventing accidental 5 V injection from USBasp/Arduino programmers.

#### 4) Form Factor Strategy

To achieve the smallest possible footprint for cylindrical burial:

- **Vertical socket stacking:** Smaller components (LIS3DH breakout, ZVP2106A MOSFET, decoupling capacitors) are placed beneath the DIP-28 socket overhang or in adjacent vertical space.
- **Inline alignment:** The DIP-28 socket, supercapacitor, and RFM95W module are aligned along a single axis to create a narrow, stick-shaped PCB.
- **Dual-side mounting:** Heavy components (battery holder clips, supercapacitor) occupy the bottom layer, while the microcontroller and radio module sit on the top layer, effectively halving board length.
- **Direct wire soldering:** Moisture sensor leads are soldered directly to through-hole edge pads, eliminating plastic terminal blocks.

---

## IV. Firmware and Software Architecture

### A. Embedded Firmware (Transmitter Node)

The transmitter firmware is written in **C++** using the **AVR-GCC toolchain**, compiled with **CMake/Ninja**, and edited in **Neovim**. We deliberately avoid the Arduino framework to achieve maximum power efficiency and deterministic timing.

#### 1) Sleep and Wake-Up Management

The firmware operates on a **state machine** with two primary states:

- **STATE_HIBERNATE:** ATmega328P enters power-down sleep mode. The watchdog timer is configured for a 15-minute interval. All peripherals (SPI, ADC, TWI) are disabled. The only active interrupt sources are INT0 (LIS3DH activity) and WDT_vect (timer overflow).
- **STATE_ACTIVE:** Triggered by either interrupt source. The MCU wakes, executes the measurement and transmission routine, and returns to STATE_HIBERNATE.

#### 2) Measurement Routine

Upon wake-up, the firmware executes the following sequence:

```
1. Assert MOSFET_GATE_LOW → Enable sensor/rail power (delay 5 ms for stabilization)
2. Initialize SPI bus → Verify LIS3DH WHO_AM_I (0x33)
3. Read LIS3DH X, Y, Z registers via SPI (6 bytes)
4. Initialize ADC → Select moisture sensor channel
5. Start ADC conversion → Wait for completion → Read 10-bit result
6. De-assert MOSFET_GATE_HIGH → Cut sensor power
7. Compose telemetry packet: [DeviceID(2B) | Moisture(2B) | TiltX(2B) | TiltY(2B) | TiltZ(2B) | Status(1B) | CRC-8(1B)]
8. Initialize RFM95W via SPI → Set frequency 433 MHz, power, spreading factor
9. Transmit packet (<500 ms airtime)
10. Enter radio sleep mode → Return to STATE_HIBERNATE
```

Total active time per cycle: **<50 ms** for measurement, **<500 ms** for radio transmission.

#### 3) Emergency Override

If the LIS3DH activity interrupt fires, the firmware sets the **CRITICAL_STATUS** flag (bit 7, `0x80`) in the telemetry packet status byte. The desktop GUI interprets this flag as a hardware emergency, bypassing normal threshold logic and triggering immediate alarm visualization regardless of moisture or tilt values.

### B. Surface Tower Receiver Firmware

The tower receiver firmware manages three concurrent responsibilities: LoRa RX, flash database logging, and USB-UART forwarding.

1. **Continuous Receive:** The RFM95W is placed in `RX_CONTINUOUS` mode. DIO0 is mapped to INT0 on the ATmega328P.
2. **Interrupt Service Routine:** On DIO0 rising edge, the ISR reads the packet payload and RSSI from the RFM95W FIFO via SPI, then pushes the raw bytes into a RAM ring buffer.
3. **Main Loop Processing:** The main loop dequeues packets from the ring buffer, prepends a 32-bit monotonic timestamp, and writes the record to the W25Q64 flash using page-program commands. A sector-based circular buffer management scheme ensures wear leveling across the 8 MB address space.
4. **Serial Forwarding:** Every received packet is also transmitted over UART0 to the FT232RL at 115200 baud, 8N1, allowing the desktop dashboard to display live data.
5. **Dump Mode:** Upon receiving a special command byte (`0x44` for "Dump") from the PC over USB, the MCU suspends new flash writes, reads the entire logged history from W25Q64, and streams it over UART.

### C. Desktop Dashboard Application

The visualization layer is a **standalone native application** developed in **C++ using the Qt 6 Framework and QML** within **Qt Creator**.

#### 1) Functional Requirements

- **Multi-node management:** Simultaneous monitoring of up to 255 unique Device IDs.
- **Geographic mapping:** Each Device ID is associated with a real-world location (e.g., "North Slope — Mine Area A") via a configuration file or SQLite database.
- **Real-time telemetry display:** Live updating of moisture percentage and tilt angles for each zone.
- **Color-coded status grid:**
  - **Green:** All parameters within safe thresholds.
  - **Yellow:** Elevated moisture or minor tilt detected (warning).
  - **Red + flashing text:** Critical movement detected or moisture above flood threshold (emergency).
- **Data logging:** Persistent local SQLite storage of all received packets with timestamps for post-event analysis.
- **Alarm system:** Visual alerts and optional audio notification when thresholds are breached.
- **Authentication:** Admin and guest login sessions with role-based access control.
- **Simulation mode:** A software toggle that pauses live serial input and injects synthetic packets for demonstration and testing.

#### 2) Serial Protocol

Link: **115200 baud, 8 data bits, no parity, 1 stop bit, no flow control.** DTR and RTS are asserted so USB-UART bridges start transmitting immediately.

**Binary Frame (12 bytes, big-endian):**

| Offset | Size | Field |
|--------|------|-------|
| 0 | u16 | Device ID |
| 2 | u16 | Moisture raw (0–10000 → 0.00–100.00 %) |
| 4 | i16 | Tilt X (milli-g) |
| 6 | i16 | Tilt Y (milli-g) |
| 8 | i16 | Tilt Z (milli-g) |
| 10 | u8 | Hardware status (`0x80` = emergency interrupt) |
| 11 | u8 | CRC-8/SMBUS over bytes 0–10 (poly `0x07`, init `0x00`) |

Frames are aligned by CRC. Bytes that fail CRC are discarded one at a time until a valid frame is found.

**ASCII Lines:**

Newline-terminated, comma/semicolon/whitespace separated:
```
A104,42.5,1.2,-0.4,0.8
0xB201 3850 120 -40 980 0
```

#### 3) Architecture

The application follows a **model-view-controller (MVC)** pattern:

- **SerialPortReader (Model):** Handles USB-UART communication, auto-port detection (CH340/CP210/FTDI preference), packet parsing, and CRC validation.
- **NodeManager (Controller):** Maintains a hash map of active nodes, applies threshold logic, manages SQLite persistence, and emits state-change signals.
- **QML Dashboard (View):** Qt Quick-based UI with node cards, zone map, alert log, settings panels, and history charts.

---

## V. Mechanical Enclosure and Deployment Design

### A. Enclosure Philosophy

A rectangular junction box buried one meter underground experiences uneven stress concentrations at its corners and flat faces. Wet, compacted soil exerts hydrostatic and lithostatic pressure that can deform or crack rigid enclosures over seasonal freeze-thaw cycles.

We propose a **cylindrical PVC pipe capsule** design for the following reasons:

- **Uniform stress distribution:** A circular cross-section naturally resists omnidirectional crushing forces from mud and soil.
- **Minimal burial footprint:** A slim tube can be deployed through a narrow auger-drilled hole, causing minimal surface disruption.
- **Modular stackability:** Components slide into the tube in a linear stack, matching our inline PCB alignment strategy.

### B. Internal Layout

The cylindrical enclosure (typically 50–75 mm inner diameter, ~100 mm length, 3–5 mm wall thickness) houses the following vertical stack:

```
Top Cap (removable, gasket-sealed)
├─ Right-angle SMA antenna (extends to inner wall)
├─ RFM95W Module (on female headers)
├─ ATmega328P in DIP-28 Socket (PCB top side)
├─ LIS3DH + ZVP2106A + Decoupling Caps (PCB bottom side)
├─ Supercapacitor (47 mF radial can, 1.2 kΩ series resistor, soldered to power rails)
├─ Li-SOCl2 Battery (AA or 2/3A form factor)
├─ Moisture Sensor Blade (extends through bottom cap into soil)
Bottom Cap (permanently sealed after assembly)
```

All internal wiring is point-to-point soldered or routed on the custom PCB. No detachable connectors are used inside the sealed volume to eliminate failure points.

### C. Waterproofing

- **IP68-rated thick-wall PVC pipe** with solvent-welded or threaded end caps.
- **Silicone gasket O-rings** on both caps.
- **Cable gland or epoxy potting** where the moisture sensor blade exits the bottom cap.
- **Conformal coating** (acrylic or silicone) applied to the PCB assembly to protect against condensation.

---

## VI. Proteus Simulation and Pre-Hardware Validation

Before committing to PCB fabrication, we validate firmware behavior in **Proteus Design Suite** using a custom simulation environment.

### A. Global Power Configuration

The Proteus design is configured with a global VCC value of **3.6 V** via *Design → Configure Power Rails*. This accurately reflects the Li-SOCl2 battery voltage and ensures all logic level thresholds and ADC reference calculations are validated under real-world supply conditions.

### B. Fake LoRa Module Subcircuit

Because the RFM95W is a complex SMD module without a native Proteus model, we construct a **fake LoRa simulator** using a secondary ATmega328P inside an isolated Proteus subcircuit block:

- **SPI Slave Role:** The subcircuit ATmega328P acts as an SPI slave, intercepting data payloads via a FIFO register command layer (`0x8D`) and reassembling raw SPI stream characters into standard string data over an internal virtual terminal.
- **Pin Mapping:** Master PB5 (SCK) → Slave SCK; Master PB3 (MOSI) → Slave MOSI; Master PB4 (MISO) ← Slave MISO; Master PB1 (SS) → Slave NSS; Master PD3 (INT1) ← Slave DIO0.
- **Paced Transmission:** The master firmware enforces a `_delay_us(250)` padding gap inside character loops to prevent simulation data overruns.
- **Terminal Output:** The child-sheet virtual terminal is configured at 9600 baud, 8-N-1, with `VCC=VCC` property override to bypass subcircuit boundary isolation bugs.

### C. Fake Accelerometer Subcircuit

Similarly, the LIS3DH is emulated by a second ATmega328P subcircuit acting as an SPI slave in **Mode 3**:

- **Register Emulation:** The slave responds to register address `0x0F` (`WHO_AM_I`) with `0x33`, and to axis-read commands with simulated X/Y/Z values.
- **Interrupt Simulation:** The slave drives PD2 (INT1) low when simulated motion exceeds the configured threshold, triggering the master INT0.
- **Interactive Controls:** Six digital input pins on the child sheet (PD4/PD5 for X, PD6/PD7 for Y, PB0/PC3 for Z) allow manual adjustment of simulated displacement during runtime, with 10 kΩ pull-ups and 150 ms software debounce.
- **Collision-Free SPI:** The slave uses `SPSR & (1 << SPIF)` flag polling to avoid write-collision and data-overrun warnings.

### D. Validation Outputs

The master MCU debug terminal (9600 baud) confirms:

```
=== INTEGRATED ENVIRONMENTAL LANDSLIDE NODE ONLINE ===
--- Starting Sensor Read Session ---
Local Readouts -> Motion[X:0, Y:0, Z:0] | Moisture: 48%
Forwarding global payload packet to LoRa Transceiver...
[STATUS] DIO0 Handshake Pin detected HIGH
```

The fake LoRa child terminal confirms:

```
--- FAKE LORA SIMULATOR ONLINE ---
Awaiting incoming SPI radio waves from Master Node...
[RADIO TX ANT OUT] -> NODE01_DATA:X_0:Y_0:Z_0:MOIST_48%:SEQ_2
```

These simulations verify SPI bus integrity, interrupt timing, power-gating behavior, and packet formatting before any hardware is ordered.

---

## VII. Manufacturing and Production Workflow

### A. Pre-Flashing Process

Our manufacturing workflow intentionally separates firmware programming from final PCB assembly:

1. **Jig Assembly:** A dedicated breadboard or custom flashing jig is wired with a **ZIF (Zero Insertion Force) DIP-28 socket**, connected to an **Arduino Uno configured as an In-System Programmer (ISP)**.
2. **Fuse Configuration:** Using **AVRDUDE**, we configure the ATmega328P internal fuses:
   - Low Fuse: `0xE2` (8 MHz internal oscillator, no clock division)
   - High Fuse: `0xDA` (512-byte bootloader disabled, EEPROM preserved)
   - Extended Fuse: `0xFD` (BOD disabled for sleep power reduction)
3. **Firmware Upload:** The compiled binary (`.hex`) is flashed onto the chip via SPI through the Arduino ISP.
4. **Verification:** The chip is power-cycled on the jig, and a test routine verifies oscillator frequency, GPIO integrity, and SPI loopback.

**Benefits of Pre-Flashing:**
- Eliminates the ICSP header from the buried PCB, saving board area.
- Prevents accidental connection of a 5 V programmer to the 3.6 V battery circuit, which could damage the Li-SOCl2 cell or supercapacitor.
- Enables batch programming of multiple chips before assembly day.

### B. PCB Assembly Sequence

1. Solder DIP-28 IC socket to PCB.
2. Solder all through-hole passives (resistors, capacitors, ZVP2106A MOSFET).
3. Solder through-hole connectors: power header, moisture sensor pads, SMA connector, RFM95W female headers.
4. Insert pre-flashed ATmega328P into socket.
5. Install battery and supercapacitor (polarity-checked).
6. Install moisture sensor blade and plug RFM95W into headers.
7. Perform power-on self-test with jumper installed.
8. Remove jumper for storage/shipping.

### C. Testing Protocol

Before burial, each node undergoes:

- **Bench Test:** Verify moisture reading across dry/wet soil samples.
- **Tilt Test:** Confirm LIS3DH triggers emergency interrupt at >0.5g threshold.
- **Radio Range Test:** Transmit from inside a 1 m soil-filled barrel to surface tower; verify packet integrity.
- **Power Audit:** Measure sleep current (<10 µA target) and active burst current profile.
- **Flash Database Test:** Verify tower receiver correctly logs five simulated node IDs to W25Q64 and dumps history over USB.

---

## VIII. Future Sensing Enhancements

While the internal LIS3DH accelerometer monitors sudden vibrations and tilt, we have reserved **footprint pads for an external digital extensometer link** on the EasyEDA board layout. This enables future integration of:

1. **Draw-Wire Extensometer:** A high-tensile steel wire anchored to stable rock extends from the PVC capsule. A rotary encoder measures wire displacement, detecting microscopic terrain creep down to fractions of a millimeter.
2. **Time-Domain Reflectometry (TDR):** A buried coaxial cable detects shear-plane deformation through impedance changes, pinpointing shift locations to the centimeter.
3. **Digital In-Place Inclinometers (IPI):** High-resolution inclinometers (e.g., SCA103T) log minute angular slope changes over days, enabling predictive trend analysis in the Qt dashboard.

These candidates are documented as theoretical future work to demonstrate scalability and industrial relevance.

---

## IX. Target Deployment Environments

### A. Open-Pit and Underground Mining

Positioned along high walls, structural slopes, and pit benches to detect wall creep and subterranean water ingress. The 7-year battery life aligns with typical mine phase durations, and the wireless design avoids cable damage from heavy machinery.

### B. Civil Infrastructure and Construction

Embedded beneath highway embankments, bridge abutments, retaining walls, and dam foundations. Continuous tilt monitoring detects differential settlement and soil liquefaction risks during and after construction.

### C. Rural Mountainous Communities

Deployed in remote, landslide-prone hillsides above villages. The low cost per node enables dense sensor grids that provide automated early warning, giving residents critical evacuation lead time before mudslide impact.

---

## X. Bill of Materials (BOM)

### Buried Node

| Component                   | Description                    | Qty/Node | Est. Unit Cost (USD) |
| --------------------------- | ------------------------------ | -------- | -------------------- |
| ATmega328P-PU               | 8-bit AVR MCU, DIP-28          | 1        | $2.50                |
| DIP-28 Socket               | IC socket, 300 mil             | 1        | $0.30                |
| RFM95W                      | 433 MHz LoRa module            | 1        | $5.00                |
| Capacitive Soil Sensor v1.2 | Insulated moisture probe       | 1        | $3.50                |
| LIS3DH Module               | Tri-axis accelerometer         | 1        | $2.50                |
| ZVP2106A                    | P-Channel MOSFET, TO-92        | 1        | $0.25                |
| Li-SOCl2 Battery            | 3.6 V, AA size, 2400 mAh       | 1        | $4.00                |
| Supercapacitor              | 2.7 V, 47 mF radial            | 1        | $1.50                |
| Series resistor            | 1.2 kΩ (0603 or 0805)          | 1        | $0.05                |
| PCB (Custom)                | FR-4, 2-layer, EasyEDA         | 1        | $2.00                |
| PVC Enclosure               | IP68 pipe + caps, ~50×100 mm   | 1        | $3.00                |
| SMA Connector               | Right-angle, PCB edge mount    | 1        | $1.00                |
| Female Headers              | 2.54 mm, 1×8 / 1×10 strips     | 2        | $0.40                |
| Passive Components          | Resistors, capacitors, headers | ~15      | $1.50                |
| **Node Subtotal**           |                                |          | **~$27.45**          |

### Surface Tower Receiver

| Component | Description | Qty | Est. Unit Cost (USD) |
|-----------|-------------|-----|---------------------|
| ATmega328P-PU | 8-bit AVR MCU, DIP-28 | 1 | $2.50 |
| DIP-28 Socket | IC socket, 300 mil | 1 | $0.30 |
| RFM95W | 433 MHz LoRa module | 1 | $5.00 |
| W25Q64JVSSIQ | 8 MB SPI NOR Flash, SOIC-8 | 1 | $1.20 |
| FT232RL Module | USB-to-UART bridge | 1 | $3.00 |
| LM1117-3.3 | 3.3 V regulator, TO-220 | 1 | $0.50 |
| PCB (Custom) | FR-4, 2-layer, EasyEDA | 1 | $2.00 |
| Passive Components | Resistors, capacitors, headers | ~10 | $1.00 |
| **Tower Subtotal** | | | **~$15.50** |

### Shared / Optional

| Component | Description | Qty | Est. Unit Cost (USD) |
|-----------|-------------|-----|---------------------|
| Arduino Uno | ISP flashing jig tool | 1 | $25.00 |
| Antenna | 433 MHz rubber duck, SMA | 2 | $3.00 |

**Total Estimated Prototype Cost:** Under $75 for one complete node + tower pair, with per-node marginal cost of ~$27 for network scaling.

---

## XI. Project Timeline

| Phase | Duration | Key Deliverables |
|-------|----------|------------------|
| **Phase 1: Design** | Weeks 1–3 | Schematic capture, PCB layout, BOM finalization, Proteus simulation |
| **Phase 2: Procurement** | Weeks 4–5 | Component ordering, PCB fabrication (JLCPCB) |
| **Phase 3: Firmware** | Weeks 3–6 | AVR-GCC toolchain, sleep/wake logic, LoRa driver, LIS3DH driver, flash driver |
| **Phase 4: Software** | Weeks 5–7 | Qt 6 / QML dashboard, serial parsing, SQLite, auth, GUI visualization |
| **Phase 5: Assembly** | Weeks 7–8 | PCB soldering, pre-flashing, enclosure potting |
| **Phase 6: Validation** | Weeks 9–10 | Bench testing, soil-barrel range test, power audit, multi-node collision test |
| **Phase 7: Documentation** | Weeks 10–12 | Final report, presentation, demo video |

---

## XII. Conclusion

We have presented the complete design proposal for a Subterranean Landslide and Flood Early-Warning Network—a fully integrated system of ultra-low-power buried sensor nodes, through-earth LoRa telemetry, a flash-based surface tower receiver, and a Qt 6 / QML desktop visualization dashboard. By combining aggressive power management, pre-flashed socket-mounted microcontrollers, compact cylindrical enclosures, capacitive moisture sensing with tri-axis motion detection, and SPI NOR flash database persistence, our system addresses the critical gap between surface-level monitoring and subsurface hazard reality.

The proposed architecture achieves a target operational lifespan of 3–7 years at a per-node cost under $30, making it economically viable for large-scale deployment in mining, infrastructure, and rural protection applications. Future work will focus on mesh-networking topologies for multi-hop relay in deep valleys, machine-learning-based threshold adaptation to reduce false alarms in seismically active regions, and integration of draw-wire extensometers for microscopic creep detection.

---

## References

[1] F. Guzzetti et al., "Landslide hazard evaluation: a review of current techniques and their application in a multi-scale study, Central Italy," *Geomorphology*, vol. 31, no. 1–4, pp. 181–216, 1999.

[2] R. L. Baum and W. Z. Savage, "Early warning of rainfall-induced shallow landslides and debris flows in the USA," *Landslides*, vol. 7, no. 3, pp. 259–272, 2010.

[3] S. Intrieri et al., "Landslide early warning system: toolbox and general concepts," *Natural Hazards and Earth System Sciences*, vol. 13, no. 1, pp. 85–90, 2013.

[4] M. Ramesh, "Design and development of a LoRa-based wireless sensor network for landslide monitoring," *IEEE Sensors Journal*, vol. 21, no. 14, pp. 16027–16037, 2021.

[5] Microchip Technology Inc., "ATmega328P Datasheet: 8-bit AVR Microcontroller," DS40002061A, 2021.

[6] Semtech Corporation, "SX1276/77/78/79: 137 MHz to 1020 MHz Low Power Long Range Transceiver," DS1276, 2019.

[7] STMicroelectronics, "LIS3DH: MEMS digital output motion sensor ultra low-power high performance 3-axes 'nano' accelerometer," DocID17530 Rev 3, 2021.

[8] Winbond Electronics, "W25Q64JV: 64M-bit Serial Flash Memory with dual/quad SPI," Rev. H, 2022.

[9] EasyEDA, "PCB Layout and Schematic Capture Tool — User Guide," JLCPCB/EasyEDA, 2023.

[10] AVR-GCC Community, "AVR Libc Reference Manual," version 2.1.0, 2022.

[11] Qt Company, "Qt 6 Documentation: Core, GUI, and QML Frameworks," 2023.

---

*This document represents the complete system design as conceived from project inception. All architectural decisions, component selections, and manufacturing workflows were established during the initial design phase to ensure a cohesive, manufacturable, and field-deployable early-warning platform.*



# Limitation
While 433 MHz LoRa can reach **10 to 15 kilometers in open air**, soil completely reshapes the link budget. Wet clay introduces extreme signal attenuation (roughly **10 to 40 dB per meter** depending on exact moisture levels).

1. **The 1-Meter Clay Barrier:** The signal will lose roughly **20 to 40 dB** just traveling through that first vertical meter of water-logged clay.
2. **The Refraction Effect:** When the radio wave hits the boundary where the soil meets the air, it bends sharply (refracts) and spreads out across the surface.
3. **The Air Path:** Once the signal breaks into the air, it behaves like normal LoRa, traveling hundreds of meters with minimal path loss.

3 Tips to Maximize the Range to Your Surface Server

- **Elevate the Server Antenna:** Raise the surface server's antenna at least **1.5 to 2 meters off the ground** on a pole. If the server antenna is sitting directly on the dirt, the ground will absorb the incoming signal, cutting your range in half.
- **Use a High-Gain Antenna on the Server:** While the buried node is forced to use a tiny, inefficient helical antenna inside its 10cm box, the surface server has no size limits. Give the server a high-quality, high-gain **omnidirectional outdoor fiberglass antenna** to compensate for the buried node's weak signal.
- **Keep the Buried Box Level:** Ensure the buried charger box is placed flat so the internal helical coil points straight up toward the sky. This aligns the polarization perfectly with the surface server's vertical antenna.

# Available on Vector4Engineering
- ADXL345 Accelerometer
![[ADXL345 vector4engineering.png]]
- Lora Module Ra-02
![[Pasted image 20260826172439.png]]
- Capacitive Soil Moisture Sensor v1.2
![[capacitive soil vector4engineering.png]]


# Datasheet for
EEPROM
Accelerometer
LORA


# About the EEPROM

Using a W25Q64 (8 Megabytes / 64 Megabits) to log sensor data before sending it to a computer is a very common and excellent approach, but because sensor data "keeps coming," you have to manage how often you write to it so you don't burn out the chip.

Here is a breakdown of why it works well and what you need to watch out for:

## Why it is a good idea

- Non-volatile: If your sensor device loses power or the battery dies, your data is safe and won't vanish.
- Fast enough: It can easily handle the speed of typical sensor data.
- Large capacity: 8MB can hold millions of individual sensor readings (e.g., if one reading is 8 bytes, it can hold 1 million readings).

## The Big Catch: Wear and Tear

Because flash memory wears out after 100,000 writes per sector, how you write your code matters. If you write to the exact same memory address every second, you will destroy that part of the chip in less than 28 hours.

## How to do it safely (Best Practices)

1. Buffer data in RAM first: Do not write to the flash chip for every single sensor reading. Collect readings in your microcontroller's RAM until you have a full "Page" (256 bytes), then write that entire page to the flash chip at once.
2. Use a Ring Buffer / Wear Leveling: Start writing at address `0`, and move forward sequentially. When you reach the end of the 8MB chip, loop back to the beginning. This ensures the 100,000-cycle limit is spread evenly across the entire chip.
3. Erase efficiently: Remember that flash memory can only change bits from `1` to `0` when writing. To change them back, you must erase. Erasing happens in 4KB sectors. Erase a sector only right before you are about to fill it with new data.

## Math Example: How long will it last?

Assume your sensor takes a reading every 1 second, and each reading is 8 bytes (e.g., timestamp + temperature + humidity).

- 1 page = 256 bytes = 32 readings (takes 32 seconds to fill in RAM).
- The chip holds 32,768 pages.
- It will take about 12 days of continuous logging just to fill the entire 8MB chip once.
- Since every spot can be rewritten 100,000 times, the chip will last for over 3,000 years of continuous logging.

To help you set this up correctly, could you tell me how many bytes each sensor reading is and how often (e.g., every millisecond, every second) the data arrives?