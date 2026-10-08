# Subterranean Landslide and Flood Early-Warning Network: A Long-Term Buried Sensor Telemetry System

---

## Abstract

Landslides and flash floods represent persistent threats to rural communities, mining operations, and civil infrastructure, yet conventional monitoring systems rely predominantly on surface-level instrumentation that detects failure only after ground collapse has initiated. We propose the design and implementation of a **Subterranean Landslide and Flood Early-Warning Network**—a distributed system of autonomous, battery-powered sensor nodes buried at a depth of one meter, capable of operating maintenance-free for three to seven years. Each node continuously monitors subsurface soil moisture and multi-axis ground movement using ultra-low-power sensors, and transmits telemetry through soil via 433 MHz LoRa radio to a surface receiver connected to a native desktop dashboard. The system employs aggressive power management through deep-sleep hibernation, sensor power-gating via P-channel MOSFETs, and a pre-flashed ATmega328P microcontroller architecture to eliminate on-board programming headers and minimize form factor. We detail the complete hardware design, custom PCB layout strategy, embedded firmware architecture, Qt-based visualization interface, compact cylindrical enclosure, and manufacturing workflow. The proposed system offers a cost-effective, scalable, and robust early-warning solution for landslide-prone regions and structural monitoring applications.

**Keywords:** Landslide early warning, subsurface sensing, LoRa telemetry, ultra-low-power embedded systems, soil moisture monitoring, accelerometer tilt detection, ATmega328P, bare-metal firmware, Qt desktop application, buried sensor network.

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
4. **Real-Time Visualization:** Provide a standalone desktop application that maps node locations, displays live telemetry, and triggers visual and textual alarms when safety thresholds are exceeded.
5. **Compact and Deployable Form Factor:** Package the electronics into a rugged, waterproof, cylindrical enclosure optimized for burial and environmental resilience.

### C. Scope and Contributions

This project encompasses the full engineering lifecycle: schematic design, custom PCB development, embedded firmware programming, radio communication validation, desktop software engineering, mechanical enclosure design, and system integration testing. Our primary technical contributions include:

- A **pre-flashing manufacturing workflow** that eliminates ICSP headers from the final PCB, reducing size and eliminating the risk of 5V back-feeding into the 3.6V battery circuit.
- A **dual-side PCB stacking strategy** paired with a vertical cylindrical enclosure to minimize burial footprint and maximize structural integrity against soil pressure.
- A **smart-sleep firmware architecture** achieving 99.9% hibernation duty cycle with sub-50ms active measurement windows and interrupt-driven emergency wake-up.

---

## II. System Architecture and Overview

### A. High-Level Block Diagram

The complete system consists of two primary subsystems: the **Buried Sensor Node** and the **Surface Station with Desktop Dashboard**.

```
				┌─────────────────────────────────────────────────────────────────────────┐
				│                         SYSTEM ARCHITECTURE                             │
				├─────────────────────────────────────────────────────────────────────────┤
				│                                                                         │
				│   ┌──────────────────────┐         433 MHz LoRa         ┌───────────┐   │
				│   │   BURIED SENSOR NODE │◄────────────────────────────►│  SURFACE  │   │
				│   │   (1m Underground)   │         Through-Earth        │  RECEIVER │   │
				│   └──────────────────────┘                              └─────┬─────┘   │
				│          │                                                    │         │
				│          ▼                                                    ▼         │
				│   ┌──────────────┐                                    ┌─────────────┐   │
				│   │ Soil Moisture│                                    │   USB/UART  │   │
				│   │   Sensor     │                                    │   Bridge    │   │
				│   ├──────────────┤                                    └──────┬──────┘   │
				│   │  ADXL345     │                                           │          │
				│   │Accelerometer │                                    ┌───────▼───────┐ │
				│   ├──────────────┤                                    │  DESKTOP GUI  │ │
				│   │ ATmega328P   │                                    │  (Qt 6 / C++) │ │
				│   │Microcontroller                                    │  • Map View   │ │
				│   ├──────────────┤                                    │  • Telemetry  │ │
				│   │ SX1276/RFM95W│                                    │  • Alerts     │ │
				│   │ LoRa Module  │                                    └───────────────┘ │
				│   ├──────────────┤                                                      │
				│   │ Li-SOCl2 +   │                                                      │
				│   │Supercapacitor│                                                      │
				│   └──────────────┘                                                      │
				│                                                                         │
				└─────────────────────────────────────────────────────────────────────────┘
```

### B. Operational Workflow

The buried node follows a deterministic six-step operational loop:

1. **Deep Sleep (Hibernation):** For 99.9% of its operational lifetime, the node rests in a powered-down state. The ATmega328P disables all peripheral clocks, and the P-channel MOSFET cuts power to sensors and the radio.
2. **Wake-Up Trigger:** An internal watchdog timer triggers a scheduled wake-up every 15 minutes. Alternatively, the ADXL345 motion sensor generates an interrupt that forces an immediate emergency wake-up if ground movement exceeds a calibrated threshold.
3. **Sensor Activation:** The microcontroller enables the MOSFET gate, powering the moisture sensor and accelerometer. Analog-to-digital conversion and SPI communication occur simultaneously.
4. **Data Acquisition:** Soil capacitance and three-axis acceleration/tilt values are sampled and compressed into a compact telemetry packet containing a unique 16-bit Device ID.
5. **Radio Transmission:** The SX1276 LoRa module broadcasts the packet at 433 MHz through the soil layer to the surface receiver. Transmission duration is minimized to conserve energy.
6. **Power Down:** All peripherals are de-energized, and the microcontroller returns to deep sleep until the next cycle.

---

## III. Hardware Design

### A. Microcontroller and Core Logic

We selected the **ATmega328P** as the central processing unit for both the transmitter node and surface receiver. This 8-bit AVR microcontroller offers several advantages for our application:

- **Mature ultra-low-power modes:** Power-down sleep with watchdog timer wake-up achieves sub-microamp quiescent current.
- **Wide operating voltage:** Functional from 1.8V to 5.5V, compatible with our 3.6V Li-SOCl2 battery system.
- **Sufficient I/O and interfaces:** 23 programmable GPIO pins, one SPI bus (for LoRa and accelerometer), one I²C bus (for ADXL345 alternative routing), and a 10-bit ADC (for soil moisture analog reading).
- **DIP-28 package availability:** Enables socket-mounted assembly for pre-flashing and field serviceability.

We operate the ATmega328P at **8 MHz internal RC oscillator** (no external crystal), eliminating two components and reducing power consumption during active cycles. The internal fuses are configured to disable the Brown-Out Detector (BOD) during sleep, as the Li-SOCl2 chemistry provides a stable voltage profile throughout its discharge curve.

### B. Sensor Subsystem

#### 1) Subsurface Moisture Detection

Soil moisture is measured using a **Capacitive Soil Moisture Sensor v1.2**. Unlike resistive sensors that corrode through electrolysis when DC current passes through soil, the capacitive blade uses a high-frequency oscillator to measure the dielectric permittivity of the surrounding earth—a proxy for water content. The sensor outputs an analog voltage (0–3.3V) proportional to moisture level, read directly by the ATmega328P ADC.

The probe is electrically insulated from the PCB and connected via direct soldered wires to through-hole pads on the PCB edge. This eliminates bulky screw terminals and reduces the enclosure diameter.

![[capacitive soil moisture sensor.png]]
#### 2) Ground Movement and Tilt Tracking

We employ the **ADXL345** digital tri-axis accelerometer for motion detection. Key specifications relevant to our design:

- Ultra-low power: ~23 µA in measurement mode at 100 Hz.
- Programmable activity/inactivity thresholds with interrupt output pins.
- ±2g/±4g/±8g/±16g selectable measurement ranges.
- SPI and I²C digital interfaces.

The ADXL345 is configured with an activity threshold of approximately 0.5g on any axis. When ground shift or vibration exceeds this level, the INT1 pin toggles, generating an external interrupt on the ATmega328P (INT0/INT1). This bypasses the 15-minute sleep timer and forces immediate emergency telemetry transmission.

![[ADXL345.png]]
#### 3) Power Gating Circuit

To achieve multi-year battery life, sensors and the radio must be completely de-energized during sleep. We implement power gating using an **AO3401 P-Channel MOSFET** in the high-side supply path. The ATmega328P drives the MOSFET gate low (via a GPIO pin) to connect the 3.6V rail to the sensor and radio subsystems. During sleep, the gate is held high, creating an open circuit with leakage current in the nanoampere range.

![[pmos real part.png]]
### C. Wireless Communication

#### 1) LoRa Module Selection

The **SX1276** transceiver (mounted on the **RFM95W** breakout module) operates in the **433 MHz ISM band**. We selected this frequency specifically for through-earth propagation:

- **Lower attenuation:** Sub-1 GHz signals experience significantly lower path loss through soil and water-logged clay compared to 2.4 GHz alternatives.
- **Long range:** LoRa modulation provides high receiver sensitivity (down to -148 dBm), enabling reliable communication from 1m depth even with modest transmit power (14–20 dBm).
- **Low duty cycle:** The transmitter is active for less than 500 ms per 15-minute cycle, keeping average current draw minimal.

![[RFM9x 900MHz.png]]
#### 2) Antenna Integration

To conserve vertical headspace inside the cylindrical enclosure, we use a **right-angle SMA connector** mounted on the PCB edge. This allows the antenna to run parallel to the battery and supercapacitor, rather than protruding vertically. A quarter-wave monopole or helical antenna is connected externally.
![[antena.png]]
### D. Power Subsystem

#### 1) Primary Battery

The node is powered by a **3.6V Li-SOCl2 (Lithium Thionyl Chloride) primary cell**. Li-SOCl2 chemistry was chosen for:

- **Highest energy density among primary batteries:** Up to 710 Wh/kg, enabling small physical size.
- **Extremely low self-discharge:** <1% per year at 20°C, critical for multi-year deployments.
- **Wide temperature range:** Operational from -60°C to +85°C, suitable for varying underground thermal conditions.
- **Flat discharge curve:** Maintains stable voltage until end-of-life, simplifying power management.
![[li-socl battery.png]]
#### 2) Energy Buffering

LoRa transmission creates brief current surges (up to 120 mA during RF power amplifier activation) that exceed the continuous current capability of small Li-SOCl2 cells. We mitigate this with a **high-capacity supercapacitor** (typically 1–10 F, 5.5V) in parallel with the battery. The supercapacitor acts as an energy reservoir: it trickle-charges between transmissions and delivers burst current during radio activation, preventing battery voltage collapse and extending cell life.

#### 3) Master Power Disconnect

A **2-pin male header with removable plastic jumper shunt** serves as the master power switch. Removing the jumper creates a physical open circuit between the battery and the PCB. This is essential for:

- Preventing battery drain during storage, shipping, and pre-deployment testing.
- Ensuring safe handling during final assembly and socket insertion.
- Eliminating the need for mechanical slide switches, which are bulkier and less reliable in dirty environments.

### E. Custom PCB Design

The custom PCB is designed in **EasyEDA** (Standard Edition) and fabricated with standard FR-4 substrate. Our layout adheres to the following design rules:

#### 1) Decoupling and Power Integrity

- **0805/0603 SMD 0.1 µF ceramic capacitors** placed within 2 mm of the ATmega328P VCC and AVCC pins to suppress voltage droop during SPI and radio bursts.
- **Wide power traces (30–40 mil)** routed from the battery/supercapacitor junction to the MOSFET and radio module, capable of handling >200 mA transient currents.
- **Slim signal traces (10 mil)** for digital logic and sensor I/O to conserve board area.

#### 2) Ground Plane and EMI Mitigation

- **Solid copper ground planes** on both top and bottom layers, stitched with vias every 5 mm.
- The ground plane acts as an RF shield between the LoRa module (high-frequency digital) and the analog soil moisture sensor traces, preventing noise coupling into ADC readings.

#### 3) Socket-Mounted Architecture

Rather than soldering the ATmega328P directly to the board, we solder a **DIP-28 IC socket** to the PCB footprint. This decision is central to our manufacturing workflow:

- **Pre-flashing compatibility:** The chip is programmed on an external jig before insertion.
- **Field serviceability:** Firmware updates or debugging during academic testing require only an IC extractor tool—no desoldering.
- **Risk mitigation:** Eliminates the need for an on-board ICSP header, reducing PCB size and preventing accidental 5V injection from USBasp/Arduino programmers.

#### 4) Form Factor Strategy

To achieve the smallest possible footprint for cylindrical burial:

- **Vertical socket stacking:** Smaller components (ADXL345 breakout, AO3401 MOSFET, decoupling capacitors) are placed beneath the DIP-28 socket overhang or in adjacent vertical space.
- **Inline alignment:** The DIP-28 socket, supercapacitor, and LoRa module are aligned along a single axis to create a narrow, stick-shaped PCB.
- **Dual-side mounting:** Heavy components (battery holder clips, supercapacitor) occupy the bottom layer, while the microcontroller and radio module sit on the top layer, effectively halving board length.
- **Direct wire soldering:** Moisture sensor leads are soldered directly to through-hole edge pads, eliminating plastic terminal blocks.

---

## IV. Firmware and Software Architecture

### A. Embedded Firmware (Transmitter Node)

The transmitter firmware is written in **C++** using the **AVR-GCC toolchain**, compiled with **CMake/Ninja**, and edited in **Neovim**. We deliberately avoid the Arduino framework to achieve maximum power efficiency and deterministic timing.

#### 1) Sleep and Wake-Up Management

The firmware operates on a **state machine** with two primary states:

- **STATE_HIBERNATE:** ATmega328P enters power-down sleep mode. The watchdog timer is configured for a 15-minute interval. All peripherals (SPI, ADC, TWI) are disabled. The only active interrupt sources are INT0 (ADXL345 activity) and WDT_vect (timer overflow).
- **STATE_ACTIVE:** Triggered by either interrupt source. The MCU wakes, executes the measurement and transmission routine, and returns to STATE_HIBERNATE.

#### 2) Measurement Routine

Upon wake-up, the firmware executes the following sequence:

```
1. Assert MOSFET_GATE_LOW → Enable sensor/rail power (delay 5ms for stabilization)
2. Initialize SPI bus → Configure ADXL345 (if not already initialized)
3. Read ADXL345 X, Y, Z registers via SPI (6 bytes)
4. Initialize ADC → Select moisture sensor channel
5. Start ADC conversion → Wait for completion → Read 10-bit result
6. De-assert MOSFET_GATE_HIGH → Cut sensor power
7. Compose telemetry packet: [DeviceID(2B) | Moisture(2B) | TiltX(2B) | TiltY(2B) | TiltZ(2B) | Status(1B) | CRC(1B)]
8. Initialize SX1276 via SPI → Set frequency, power, spreading factor
9. Transmit packet (typically <200 bytes payload)
10. Enter radio sleep mode → Return to STATE_HIBERNATE
```

Total active time per cycle: **<50 milliseconds** for measurement, **<500 milliseconds** for radio transmission.

#### 3) Emergency Override

If the ADXL345 activity interrupt fires, the firmware sets a **CRITICAL_STATUS** flag in the telemetry packet. The desktop GUI interprets this flag as an emergency condition, bypassing normal threshold logic and triggering immediate alarm visualization.

### B. Surface Receiver Firmware

The surface station uses an identical ATmega328P + RFM95W hardware pair. It operates in **continuous receive mode**, listening on the 433 MHz channel. When a packet is received:

1. The SX1276 raises the DIO0 interrupt pin.
2. The MCU reads the packet buffer via SPI.
3. The raw byte stream is forwarded to the host computer via USB-UART bridge (FT232RL or CH340) at 115200 baud.
4. The receiver returns to RX_CONTINUOUS mode.

### C. Desktop Dashboard Application

The visualization layer is a **standalone native application** developed in **C++ using the Qt 6 Framework** within **Qt Creator**.

#### 1) Functional Requirements

- **Multi-node management:** Simultaneous monitoring of up to 255 unique Device IDs.
- **Geographic mapping:** Each Device ID is associated with a real-world location (e.g., "North Slope — Mine Area A") via a configuration file or database.
- **Real-time telemetry display:** Live updating of moisture percentage and tilt angles for each zone.
- **Color-coded status grid:**
  - **Green:** All parameters within safe thresholds.
  - **Yellow:** Elevated moisture or minor tilt detected (warning).
  - **Red + flashing text:** Critical movement detected or moisture above flood threshold (emergency).
- **Data logging:** Persistent storage of all received packets with timestamps for post-event analysis.
- **Alarm system:** Visual alerts and optional audio notification when thresholds are breached.

#### 2) Architecture

The application follows a **model-view-controller (MVC)** pattern:

- **SerialPortReader (Model):** Handles USB-UART communication, packet parsing, and CRC validation.
- **NodeManager (Controller):** Maintains a hash map of active nodes, applies threshold logic, and emits state-change signals.
- **DashboardWidget (View):** Qt Widgets-based grid displaying node cards with color-coded status indicators and trend graphs.

---

## V. Mechanical Enclosure and Deployment Design

### A. Enclosure Philosophy

A rectangular junction box buried one meter underground experiences uneven stress concentrations at its corners and flat faces. Wet, compacted soil exerts hydrostatic and lithostatic pressure that can deform or crack rigid enclosures over seasonal freeze-thaw cycles.

We propose a **cylindrical PVC pipe capsule** design for the following reasons:

- **Uniform stress distribution:** A circular cross-section naturally resists omnidirectional crushing forces from mud and soil.
- **Minimal burial footprint:** A slim tube can be deployed through a narrow auger-drilled hole, causing minimal surface disruption.
- **Modular stackability:** Components slide into the tube in a linear stack, matching our inline PCB alignment strategy.

### B. Internal Layout

The cylindrical enclosure (typically 50–75 mm inner diameter, 200–300 mm length, 3–5 mm wall thickness) houses the following vertical stack:

```
Top Cap (removable, gasket-sealed)
├─ Right-angle SMA antenna (extends to inner wall)
├─ LoRa Module (RFM95W)
├─ ATmega328P in DIP-28 Socket (PCB top side)
├─ ADXL345 + AO3401 + Decoupling Caps (PCB bottom side)
├─ Supercapacitor (radial can, soldered to power rails)
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

## VI. Manufacturing and Production Workflow

### A. Pre-Flashing Process

Our manufacturing workflow intentionally separates firmware programming from final PCB assembly:

1. **Jig Assembly:** A dedicated breadboard or custom flashing jig is wired with a **ZIF (Zero Insertion Force) DIP-28 socket**, connected to an **Arduino Uno configured as an In-System Programmer (ISP)**.
2. **Fuse Configuration:** Using **AVRDUDE**, we configure the ATmega328P internal fuses:
   - Low Fuse: 0xE2 (8 MHz internal oscillator, no clock division)
   - High Fuse: 0xDA (512-byte bootloader disabled, EEPROM preserved)
   - Extended Fuse: 0xFD (BOD disabled for sleep power reduction)
3. **Firmware Upload:** The compiled binary (`.hex`) is flashed onto the chip via SPI through the Arduino ISP.
4. **Verification:** The chip is power-cycled on the jig, and a test routine verifies oscillator frequency and GPIO integrity.

**Benefits of Pre-Flashing:**
- Eliminates the ICSP header from the buried PCB, saving ~25 mm² of board area.
- Prevents accidental connection of a 5V programmer to the 3.6V battery circuit, which could damage the Li-SOCl2 cell or supercapacitor.
- Enables batch programming of multiple chips before assembly day.

### B. PCB Assembly Sequence

1. Solder DIP-28 IC socket to PCB.
2. Solder all SMD passives (resistors, capacitors, MOSFET) using reflow or hand-soldering.
3. Solder through-hole components: power header, moisture sensor pads, SMA connector.
4. Insert pre-flashed ATmega328P into socket.
5. Install battery and supercapacitor (polarity-checked).
6. Install moisture sensor blade and LoRa module.
7. Perform power-on self-test with jumper installed.
8. Remove jumper for storage/shipping.

### C. Testing Protocol

Before burial, each node undergoes:

- **Bench Test:** Verify moisture reading across dry/wet soil samples.
- **Tilt Test:** Confirm ADXL345 triggers emergency interrupt at >0.5g threshold.
- **Radio Range Test:** Transmit from inside a 1m soil-filled barrel to surface receiver; verify packet integrity.
- **Power Audit:** Measure sleep current (<10 µA target) and active burst current profile.

---

## VII. Target Deployment Environments

### A. Open-Pit and Underground Mining

Positioned along high walls, structural slopes, and pit benches to detect wall creep and subterranean water ingress. The 7-year battery life aligns with typical mine phase durations, and the wireless design avoids cable damage from heavy machinery.

### B. Civil Infrastructure and Construction

Embedded beneath highway embankments, bridge abutments, retaining walls, and dam foundations. Continuous tilt monitoring detects differential settlement and soil liquefaction risks during and after construction.

### C. Rural Mountainous Communities

Deployed in remote, landslide-prone hillsides above villages. The low cost per node enables dense sensor grids that provide automated early warning, giving residents critical evacuation lead time before mudslide impact.

---

## VIII. Bill of Materials (BOM)

| Component                   | Description                    | Qty/Node | Est. Unit Cost (USD) |
| --------------------------- | ------------------------------ | -------- | -------------------- |
| ATmega328P-PU               | 8-bit AVR MCU, DIP-28          | 1        | $2.50                |
| DIP-28 Socket               | IC socket, 300 mil             | 1        | $0.30                |
| RFM95W                      | 433 MHz LoRa module            | 1        | $5.00                |
| Capacitive Soil Sensor v1.2 | Insulated moisture probe       | 1        | $3.50                |
| ADXL345                     | Tri-axis accelerometer module  | 1        | $2.00                |
| AO3401                      | P-Channel MOSFET, SOT-23       | 1        | $0.15                |
| Li-SOCl2 Battery            | 3.6V, AA size, 2400 mAh        | 1        | $4.00                |
| Supercapacitor              | 5.5V, 1F radial                | 1        | $1.50                |
| PCB (Custom)                | FR-4, 2-layer, EasyEDA         | 1        | $2.00                |
| PVC Enclosure               | IP68 pipe + caps, ~60×250 mm   | 1        | $3.00                |
| SMA Connector               | Right-angle, PCB edge mount    | 1        | $1.00                |
| Passive Components          | Resistors, capacitors, headers | ~15      | $1.50                |
| **Node Subtotal**           |                                |          | **~$26.45**          |
| Arduino Uno (ISP)           | Flashing jig tool (shared)     | 1        | $25.00               |
| FT232RL Module              | USB-UART for surface receiver  | 1        | $3.00                |
| Surface Receiver PCB        | Identical radio + MCU pair     | 1        | $15.00               |

**Total Estimated Prototype Cost:** Under $70 for one complete node + surface station pair, with per-node marginal cost of ~$27 for network scaling.

---

## IX. Project Timeline

| Phase                      | Duration    | Key Deliverables                                                       |
| -------------------------- | ----------- | ---------------------------------------------------------------------- |
| **Phase 1: Design**        | Weeks 1–3   | Schematic capture, PCB layout, BOM finalization                        |
| **Phase 2: Procurement**   | Weeks 4–5   | Component ordering, PCB fabrication (JLCPCB)                           |
| **Phase 3: Firmware**      | Weeks 3–6   | AVR-GCC toolchain setup, sleep/wake logic, LoRa driver, ADXL345 driver |
| **Phase 4: Software**      | Weeks 5–7   | Qt dashboard, serial parsing, GUI visualization                        |
| **Phase 5: Assembly**      | Weeks 7–8   | PCB soldering, pre-flashing, enclosure potting                         |
| **Phase 6: Validation**    | Weeks 9–10  | Bench testing, soil-barrel range test, power audit                     |
| **Phase 7: Documentation** | Weeks 10–12 | Final report, presentation, demo video                                 |

---

## X. Conclusion

We have presented the complete design proposal for a Subterranean Landslide and Flood Early-Warning Network—a fully integrated system of ultra-low-power buried sensor nodes, through-earth LoRa telemetry, and a native desktop visualization dashboard. By combining aggressive power management, pre-flashed socket-mounted microcontrollers, compact cylindrical enclosures, and capacitive moisture sensing with tri-axis motion detection, our system addresses the critical gap between surface-level monitoring and subsurface hazard reality.

The proposed architecture achieves a target operational lifespan of 3–7 years at a per-node cost under $30, making it economically viable for large-scale deployment in mining, infrastructure, and rural protection applications. Future work will focus on mesh-networking topologies for multi-hop relay in deep valleys, and machine-learning-based threshold adaptation to reduce false alarms in seismically active regions.

---

## References

[1] F. Guzzetti et al., "Landslide hazard evaluation: a review of current techniques and their application in a multi-scale study, Central Italy," *Geomorphology*, vol. 31, no. 1–4, pp. 181–216, 1999.

[2] R. L. Baum and W. Z. Savage, "Early warning of rainfall-induced shallow landslides and debris flows in the USA," *Landslides*, vol. 7, no. 3, pp. 259–272, 2010.

[3] S. Intrieri et al., "Landslide early warning system: toolbox and general concepts," *Natural Hazards and Earth System Sciences*, vol. 13, no. 1, pp. 85–90, 2013.

[4] M. Ramesh, "Design and development of a LoRa-based wireless sensor network for landslide monitoring," *IEEE Sensors Journal*, vol. 21, no. 14, pp. 16027–16037, 2021.

[5] Microchip Technology Inc., "ATmega328P Datasheet: 8-bit AVR Microcontroller," DS40002061A, 2021.

[6] Semtech Corporation, "SX1276/77/78/79: 137 MHz to 1020 MHz Low Power Long Range Transceiver," DS1276, 2019.

[7] Analog Devices, "ADXL345: 3-Axis, ±2 g/±4 g/±8 g/±16 g Digital Accelerometer," Rev. E, 2020.

[8] EasyEDA, "PCB Layout and Schematic Capture Tool — User Guide," JLCPCB/EasyEDA, 2023.

[9] AVR-GCC Community, "AVR Libc Reference Manual," version 2.1.0, 2022.

[10] Qt Company, "Qt 6 Documentation: Core and GUI Frameworks," 2023.

---


# New updates

- The development of the software dashboard we used C++ Qt6 with QML  
- Your proteus set the global VCC value to 3.6 voltage it's under design ->Configure Power Rails and select the VCC/VDD and set the value to the 3.6v and update it

- There will a device that are buried underground so that they will transmite the data like more that 5+ devices
- There will be a tower that acceptes those connections and sends to mcu that have a db that store the data come to it, this can be connected to the desktop or anything that can connect to it using a usb and run the software to analyse the data or a dedicated pc that shows the data.
- and there will be another device that uses the wireless communication to communicate with this tower and grab the data and display it i mean transfer it throught serially it can be android or a desktop who ever running the software (this is an optional device for now we will do the above two things the transmitter and the tower with dedicated pc/display running the dashboard software).
- The absolute best choice for large data capacity and fast SPI performance on an ATmega328P is a **NOR SPI Flash Memory chip**, specifically the **Winbond W25Q64 (8 Megabytes)** or **W25Q128 (16 Megabytes)**. It provides massive storage, communicates at the maximum SPI clock speed of the ATmega328P (8 MHz), and bypasses the heavy RAM/Flash overhead of an SD card file system.

- the reciver will keep recieving the data from the transmitters and store it to its mcus databases, and when ever the display device turn on or connected to it will display that data by grabbing it from the mvus db
# Components update

- for simulating the LoRa module we will use a subcircuit that have another atmega328p to fake the LoRa communication and displays suing the virtual terminal
- **Accelerometer Upgrade**: Replaced the baseline ADXL345 module with the **LIS3DH Module**. This cuts your system sleep current drastically from **23 µA down to 2 µA**, ensuring you meet your 3-to-7 year battery life goal.
![[LIS3DH.png]]
- S**MOSFET Optimization**: wapped out the surface-mount AO3401 (SOT-23) for the **ZVP2106A (TO-92)**. This gives you a true, classic through-hole P-channel MOSFET to perform your sensor power-gating safely.
- **Radio Interface**: Standardized the **RFM95W module on 2.54 mm headers** so it drops straight into standard THT footprints instead of requiring surface-mount castellated pads.
- **(Periodic Check)** with the Capacitive Soil Moisture v1.2 module is the most reliable path for your ground-up PCB design. This method ensures the sensor only draws power when needed, which is critical for achieving your multi-year battery goals.

## Future candidates for sensing the movement of the land 
1. The Pull-Wire Extensometer (Best for Compact IoT Nodes)

Instead of measuring the node itself, this sensor measures the distance between the node and a fixed point across an underground fault line.

- **The Sensor:** A **Draw-Wire Sensor (or Rotary Encoder Extensometer)**. A thin, high-tensile steel wire extends from your small PVC capsule and anchors to a stable rock, deep stake, or adjacent layer of soil. 
- **How it catches tiny slides:** If the unstable earth slides even **0.1 millimeters**, it pulls the wire slightly. The rotary encoder inside your capsule turns, giving your ATmega328P a precise, mathematical measurement of exactly how far the land shifted. 
- **Prototyping Advantage:** High-precision digital draw-wire encoders are compact and communicate easily with a microchip over standard digital communication interfaces.

---

2. Time-Domain Reflectometry (TDR) Cables

This is the premier industrial standard for detecting deep, tiny sub-surface shear planes.

- **The Sensor:** A cheap, rugged **coaxial cable** buried vertically or horizontally through the soil layers. 
- **How it catches tiny slides:** When a tiny slide happens, the moving layer of earth pinches, bends, or stretches the buried cable. This micro-deformation alters the electrical properties (impedance) at that exact spot on the wire. By bouncing an electrical pulse down the cable, the system can pinpoint exactly where the tiny shift occurred down to the centimeter.
- **Prototyping Advantage:** This requires an advanced high-speed pulse circuit, making it a great feature to include in your project's theoretical future-work documentation to impress a grading panel.

---

3. Digital In-Place Inclinometers (IPI)

Instead of looking at raw vibration, this focuses on long-term angle deviation.

- **The Sensor:** Digital high-resolution inclinometers (like a highly sensitive version of the ADXL345, or specialized chips like the **SCA103T**).
- **How it catches tiny slides:** It constantly records the gravity baseline vectors. If the ground experiences microscopic "creep" (moving a few millimeters a week), the module logs the minute change in angular slope over days. Your Qt computer application can graph this steady, slow angle increase to predict a landslide hours before it accelerates.

---

The most practical approach is adding **Footprint Pads for an External Digital Extensometer Link** onto your EasyEDA board.

for note: "While the internal accelerometer monitors sudden vibrations, the board features a digital input port connected to an underground wire extensometer to measure microscopic terrain creep down to fractions of a millimeter."


# Coding

1. Selecting of the port connected to it from the device list
2. Communicating using a serial communication with the reciever device connected using usb
3. Using a virtual port and the proteus and the dashboard will talk using that
4. Uses a sqlite for storing datas
5. the software have admin and a guest login page with authentication
6. 