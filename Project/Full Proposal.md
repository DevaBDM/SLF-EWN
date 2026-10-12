---
tags:
  - slf-ewn
  - proposal
  - formal
parent: "[[Project Overview]]"
aliases:
  - Full Project Proposal
---

# Subterranean Landslide and Flood Early-Warning Network

**Working decisions and BOM:** [[Project Overview]] · **Live gaps:** [[Status and open issues]] · **Bytes:** [[Firmware Hub]] · **Sources:** [[Code Hub]]

---

## Abstract

Landslides and flash floods threaten rural communities, mining, and civil infrastructure, yet conventional monitors sit on the surface and often see failure only after collapse has started. The **Subterranean Landslide and Flood Early-Warning Network (SLF-EWN)** places autonomous, battery-powered nodes at **one metre** depth for a **three-to-seven-year** maintenance-free life. Each node measures capacitive soil moisture and three-axis tilt (**ADXL345**), then sends a fixed **12-byte** telemetry frame through soil on **433 MHz LoRa (Ra-02 / SX1278)** to a surface tower. The tower stores frames in **W25Q64 SPI NOR** as **16-byte** records and serves a native **Qt 6 / QML** dashboard over USB-UART. Power management uses deep-sleep hibernation, a through-hole P-channel MOSFET (**ZVP2106A**) to gate sensors, and a **pre-flashed** ATmega328P so the buried PCB has no ICSP header. The build covers 1-layer THT PCBs, bare-metal AVR-GCC firmware, Proteus fake-SPI validation, a SolidWorks printable capsule, and Blender deployment visuals.

**Keywords:** landslide early warning, subsurface sensing, LoRa, ultra-low-power embedded systems, soil moisture, ADXL345, ATmega328P, AVR-GCC, SPI NOR flash, Qt 6, QML.

---

## 1. Introduction

### 1.1 Problem

Landslides and related flooding claim lives and destroy infrastructure every year [1]. Mountain villages, open-pit mines, and large earthworks are exposed to saturation and ground movement that **surface** rain gauges, cameras, and satellite imagery see only indirectly and often too late [2]. Cabled inclinometers fail when machines or weather cut the run.

Subsurface moisture and internal deformation sit closer to the failure mechanism. Putting electronics a metre underground implies a hard power budget, waterproofing, soil pressure and corrosion, and **radio through wet earth** [3]. Commercial systems are expensive or need frequent visits that a buried node cannot accept.

### 1.2 Objectives

1. **Autonomous burial** — 3–7 years without opening the capsule or changing the cell.  
2. **Direct hazards** — moisture saturation and three-axis tilt / vibration as primary indicators.  
3. **Through-earth radio** — ~1 m of soil, clay, or sand; no buried cable.  
4. **Aggregation** — one tower collecting and storing five or more nodes in local non-volatile memory, even with the PC unplugged.  
5. **Visualization** — standalone desktop map, live values, and alarms when thresholds or a hardware interrupt fire.  
6. **Form factor** — rugged waterproof cylinder that fits an auger hole.

### 1.3 Contributions (this build)

- **Pre-flash manufacturing** — chip programmed on a ZIF jig; no ICSP on the buried board; no 5 V programmer on a 3.6 V cell.  
- **Cylindrical stack** — inline layout in a round tube (uniform soil pressure; boxes crack at corners).  
- **Smart sleep** — ~99.9% hibernation, short measure window, ADXL345 INT for emergencies.  
- **Tower log** — W25Q64 instead of a MicroSD filesystem on an 8-bit MCU.  
- **Qt 6 / QML dashboard** — poll tower UART, SQLite, admin/guest, simulation isolated from the COM port.  
- **Proteus framework** — extra ATmega328P chips emulate ADXL345 and Ra-02 so the **same `.hex`** is proven before fab.

---

## 2. System architecture

### 2.1 Three subsystems

```
 Buried node (1 m)                          Surface tower
 ┌─────────────────────┐     433 MHz LoRa    ┌──────────────────┐
 │ Moisture v1.2       │  through earth (5+) │ ATmega328P 16 MHz│
 │ ADXL345             │ ──────────────────► │ Ra-02 RX         │
 │ ATmega328P 8 MHz RC │                     │ W25Q64 flash     │
 │ Ra-02 TX            │                     │ FT232RL USB      │
 │ Li-SOCl₂ + 47 mF RC │                     └────────┬─────────┘
 │ ZVP2106A gate       │                              │ USB-UART
 └─────────────────────┘                              ▼
                                              Qt 6 / QML dashboard
                                              map · telemetry · alerts · SQLite
```

A handheld / Android client that talks to the tower over a second radio is **future work only**.

### 2.2 Buried-node loop

1. **Hibernate** — clocks off, MOSFET open, radio asleep (~99.9% of life).  
2. **Wake** — WDT 8 s ticks (production target **15 min**; Proteus uses `SIM_FAST_WAKE` ≈ 8 s), or INT0 on motion.  
3. **Sensor rail on** — gate low; 5 ms settle; ADC + SPI.  
4. **Acquire** — ADC1 moisture + ADXL345 axes; pack ID, status, CRC-8.  
5. **Transmit** — FET on again; SPI to radio (Proteus: `0x8D` FIFO; field: SX1278 RegFifo).  
6. **Power down** — FET off; back to hibernate.

### 2.3 Tower loop

DIO0 → ISR reads 12 bytes → CRC check → 16-byte flash record (`record_id` + frame). PC **polls** 115200 (`0xAA` replies to `'S'` / `'D'` / `'N'` / `'C'`). Unplugged PC: logging continues until capacity is exhausted.

### 2.4 Collisions

One radio demodulates one packet at a time. ID jitter `15 min + (id % 30)` is **specified**, not yet implemented in `transmitter.c` — see [[Status and open issues#F2]].

Canonical hop sizes and format tables: [[SLF-EWN Hub#Data flow (as built)]] · [[Firmware Hub#Data formats]].

---

## 3. Hardware design

### 3.1 Microcontroller

**ATmega328P** on node, tower, and Proteus slaves:

- Power-down + WDT: sub-µA class sleep.  
- 1.8–5.5 V, matches 3.6 V Li-SOCl₂.  
- SPI (LoRa, accel, flash) + 10-bit ADC (moisture) + GPIO.  
- DIP-28: socket, pre-flash, field swap.

Buried clock: **8 MHz internal RC** (no crystal). Tower: **16 MHz** crystal. BOD off or 1.8 V — Arduino 2.7 V / 4.3 V BOD would lock the chip as the cell sags.

### 3.2 Sensors and gate

**Moisture — capacitive v1.2.** Resistive probes electrolyse and die. Capacitive blade → ADC1. Leads soldered to edge pads. Powered only during the sample via the FET.

**Motion — ADXL345 (as built).** `DEVID 0x00 = 0xE5`. Full-resolution ±2 g; firmware reads 16-bit axes. INT1 → MCU INT0 for motion wake. Status `0x80` = emergency wake; `0x01` = DEVID fail. Shop stocks ADXL345; LIS3DH remains a future low-µA option.

**Gate — ZVP2106A** P-MOSFET, TO-92. GPIO **LOW** connects 3.6 V to `VCC_SW`. Sleep leakage in the nA range.

### 3.3 Radio and antenna

**Ra-02** (SX1278, 433 MHz). Sensitivity to about −148 dBm; TX 14–20 dBm. Airtime target &lt; 500 ms per 15 min cycle.

**λ/4 in air** (VF ≈ 0.95):

$$\lambda/4 = \frac{3\times10^8}{433\times10^6\times 4}\times 0.95 = \mathbf{16.45\ \mathrm{cm}}$$

Wet clay ($\varepsilon_r \approx 30$) collapses electrical length toward ~3 cm and detunes a bare wire. **Keep the helix in an air pocket** inside the capsule: cut 17.0 cm solid copper, 5 mm mandrel, 18–22 turns, ~3.5 cm tall, axis **vertical**.

**Through-soil budget.** Wet clay ~10–40 dB/m; first metre ~20–40 dB before breakout, then ordinary LoRa in air. Design intent: **1 m** through soil, **100–200 m** once in air. Tower whip **1.5–2 m** on a pole. Detail: [[Project Overview#Radio]].

### 3.4 Node power

**3.6 V Li-SOCl₂** (~2400 mAh AA): high energy density, &lt;1%/year self-discharge, flat voltage, −60…+85 °C. TX peaks ~120 mA for ~50 ms → **RC buffer: 1.2 kΩ series + 47 mF / 5.5 V** on the always-on rail ($\Delta V\approx127$ mV, $I_\mathrm{max}=3$ mA, $5\tau\approx4.7$ min ≪ 15 min). **2-pin jumper** for storage and safe MCU insertion (no slide switch in mud). RX has no supercap.

### 3.5 Tower hardware

| Function | Part | Mount |
|:---------|:-----|:------|
| MCU | ATmega328P-PU | DIP-28 socket |
| Radio | Ra-02 | 2.54 mm female |
| Log | W25Q64 | DIP-8 / SOIC adapter |
| USB | FT232RL module | headers |
| 3.3 V | LM1117-3.3 | THT |

USB 5 V primary; optional battery is backup. Shared SPI; radio CS **PB2**, flash CS **PB1**, DIO0 → **INT0**.

### 3.6 PCB rules

1-layer FR-4, 100% THT. TX **50×50 mm**, RX **95×70 mm**. 0.1 µF within 5 mm of VCC/AVCC; electrolytic at radio VCC; power traces 30–40 mil; jumpers at crossings (not a second layer). DIP-28 socket; modules on female headers; moisture on edge pads. Layout: [[EasyEDA Hub]] · [[Transmitter PCB]] · [[Receiver PCB]].

---

## 4. Firmware and protocol

**Software trust policy.** Node and tower firmware use **self-written** device drivers and protocol code. The only non-project includes are AVR-GCC / avr-libc headers (`avr/io.h`, `stdint.h`, etc.). No Arduino core or third-party LoRa/accel libraries — [[Code Hub#Self-implemented firmware (supply-chain / trust policy)]].


Written in **C** (AVR-GCC). **No Arduino.** Modular tree under `Code/` — [[Code Hub]]. Same master hex in Proteus and in the socket.

### 4.1 Air frame (12 bytes, big-endian)

| Off | Field |
|:---:|:------|
| 0–1 | Device ID (`0x0104` default) |
| 2–3 | Moisture 0–10000 |
| 4–9 | Tilt X/Y/Z (i16) |
| 10 | Status (`0x80` emergency, `0x01` accel fail, seq in low bits) |
| 11 | CRC-8/SMBUS over bytes 0–10 |

### 4.2 Flash record (16 bytes)

RID u32 BE + copy of the 12-byte air frame. Append-only; sector erase on sector boundary; chip erase when full or on clear.

### 4.3 Tower ↔ dashboard (115200 8N1)

PC polls with `'S'` / `'D'` / `'N'` / `'C'`. Replies start with magic **`0xAA`**. Full tables: [[Firmware Hub#Data formats]].

### 4.4 Pre-flash

No ICSP on the buried board. Node fuses: L `0xE2`, H `0xDA`, E `0xFD`. Tower crystal fuses are a separate recipe — tracked in [[Status and open issues#H2]].

---

## 5. Dashboard software

Native **Qt 6 / QML** desktop console. Polls the tower; does not mix live COM with simulation mode.

- Node cards, sparklines, SAFE / WARNING / CRITICAL  
- Map (OSM / satellite), Places polygons, per-node alarm limits  
- Alert log, CSV export, admin vs guest (PBKDF2)  
- Default thresholds: moisture 38% / 58%; tilt magnitude 8° / 16°; status `0x80` forces CRITICAL  

Detail: [[Dashboard Hub]].

---

## 6. Mechanical enclosure and visualisation

A **cylinder** (PVC or print) spreads load, fits an auger, and matches the inline PCB. Typical shop pipe ~**50×100 mm** (50×50 mm PCB + 35 mm clearance + 5 mm).. Stack and waterproofing: [[Mechanical Hub]].

Team B delivers SolidWorks printables; Team D renders film from those dimensions only — no new millimetres in Blender.

---

## 7. Validation

### 7.1 Proteus

TX sheet at **3.6 V**, RX at **5 V**. Fake ADXL345 and Ra-02 are child ATmega328P firmwares. Two virtual COM pairs: packet path (9600 raw 12 bytes) and dashboard path (115200). Procedure: [[Proteus Hub#End-to-end test]].

### 7.2 Manufacturing and bench tests

1. ZIF + Arduino as ISP + AVRDUDE; flash and power-cycle check.  
2. Solder socket, passives, FET, headers, SMA, moisture pads, jumper.  
3. Insert pre-flashed MCU, cell, **1.2 kΩ + 47 mF**, modules; POST; remove jumper for storage.  

**Before burial:** dry/wet moisture; motion INT; 1 m soil barrel; sleep **&lt; 10 µA** with `SIM_FAST_WAKE` off; multi-ID dump on the dashboard.

### 7.3 IEC standards and feasibility

**Mentor feasibility check.** The architecture is **feasible** under normal IEC practice: it does not conflict with the main families used for outdoor and environmental electronics. Feasibility means the approach is *designable within those frameworks*, not that accredited certificates are already in hand.

**Statement.** A sealed, battery-powered subsurface node with short-range sub-GHz radio and a surface acquisition unit falls within established scopes for ingress protection (**IEC 60529**), environmental testing (**IEC 60068**), EMC (**IEC 61000** series), ICT safety (**IEC 62368-1**), and primary batteries (**IEC 60086**). Operation on 433 MHz is governed by spectrum regulation (typically ETSI / national short-range device rules), not by an IEC prohibition. Landslide early-warning *process* guidance is **ISO 22327** / **ISO 22328** (complementary, not IEC electrical).

| Domain | Standards | Application to SLF-EWN |
|:-------|:----------|:-----------------------|
| Enclosure | IEC 60529, IEC 62262 | Capsule / tower IP and impact intent |
| Environment | IEC 60068 | Climatic and mechanical qualification path |
| EMC | IEC 61000-6-x, IEC 61000-4-x | Emissions/immunity design target; ESD, RF, surge |
| Safety | IEC 62368-1 | Tower, USB, host path |
| Primary cell | IEC 60086 | Li-SOCl₂ long-life primary |

**Out of prototype scope unless formally claimed:** IEC 61508 functional-safety SIL. Full product compliance requires laboratory testing after the research prototype; this build designs *toward* the standards above and validates seals, sleep current, and RF path by measurement.

Full tables and mapping: [[Project Overview#IEC standards and feasibility]].  
Mentor-oriented pack (spectrum, safety language, cost, power/RF proof status, FAQ): [[Mentor readiness]].

---

## 8. Deployment environments

| Site | Why |
|:-----|:----|
| Mine walls / pit benches | Wall creep, water ingress; 7-year cell matches a mine phase |
| Highway / dam / abutment earthworks | Settlement and liquefaction |
| Village hillsides | Dense cheap grid → evacuation lead time |

---

## 9. Bill of materials and timeline

Node ~$27, tower ~$15.50, pair **&lt; $75**. Line items: [[Project Overview#Bill of materials]].

Twelve working days: design → procurement → firmware → dashboard → assembly → test → docs. Team split: A PCB/Proteus/firmware · B SolidWorks · C dashboard · D Blender.

---

## 10. Challenges and design responses

### 10.1 Flash wear

NOR cells program only 1→0; reuse needs sector erase. Append-only policy spreads writes across 8 MB; chip erase when full. A ring buffer would be better for multi-decade life — not in this build.

### 10.2 Through-soil RF

Wet clay ~10–40 dB/m at 433 MHz. Helix stays in an **air pocket**; tower whip is high and vertical. The design is optimistic until soil-barrel measurements close [[Status and open issues#H4]].

### 10.3 Multi-year battery life

Li-SOCl₂ cannot source the TX peak alone → **47 mF + 1.2 kΩ** RC buffer. Average current must stay well under 10 µA at 15 min interval. Hibernation, FET gating, and real radio sleep are mandatory.

### 10.4 Non-recoverable nodes

~$27 abandoned at depth. Mitigations: long cell life, socketed pre-flashed MCU, dense grid that tolerates individual failures.

### 10.5 Parts availability

Moisture v1.2, ZVP2106A, and Ra-02 are shelf-specific. Substitution would force schematic and Proteus re-validation.

---

## 11. Future work (not this build)

- Draw-wire extensometer, TDR, or IPI-class inclinometer (pads reserved on TX).  
- Production ring-buffer wear-leveling.  
- LIS3DH drop-in if lower active current is required.  
- Mesh in deep valleys; ML false-alarm filtering.

### 11.1 Multi-path data access (agreed architecture)

Full write-up: [[Data path future architecture]].

| Element | Plan |
|:--------|:-----|
| Tower **primary** LoRa | Unchanged — continuous RX from buried nodes |
| Tower **secondary** LoRa | Backhaul to host and/or handheld (avoids missing node packets on one radio) |
| Tower **USB** | **Retained** — dump, debug, firmware update |
| Tower **siren** | Latched loud alarm on critical node frame (`status 0x80`); **host/handheld SILENCE** when a supervising link is active; otherwise alarm **keeps sounding** |
| Tower **power** | **Solar** primary + **battery backup** in the field; USB retained for lab/debug/firmware |
| Tower **telemetry** | Log battery, uptime, storage, siren, radio health; host shows a **tower panel** beside node data |
| **Host registry** | DB + UI: register towers (location), nodes, and **which node belongs to which tower**; map and link paths |
| **Handheld** | Embedded + mini LCD; LoRa and/or USB to tower; USB data-mule to PC; no full map GUI |
| **Host** | Qt advanced UI; LoRa when in range; else import from handheld over USB; Acknowledge → silence |

Backhaul prefers incremental “records after ID *n*” and optional CRITICAL push; bulk history stays on USB. Siren detail: [[Data path future architecture#Tower local alarm (siren)]]. This is **Phase 2** after the USB-based prototype is validated.

Open implementation gaps for the **current** build (jitter, moisture invert, `SIM_FAST_WAKE`, fab, sleep current): [[Status and open issues]].

---

## 12. Conclusion

SLF-EWN is a complete path from a 1-layer buried THT node through through-earth LoRa and flash-backed tower autonomy to a native dashboard, with Proteus validation before fab. Per-node cost in the tens of dollars makes dense grids plausible. Remaining work is measured closure of the open issues list, not a redesign of the architecture.

---

## References

[1] USGS, *Landslide Hazards Program*, annual summaries.  
[2] Petley, D., “Global patterns of loss of life from landslides,” *Geology*, 2012.  
[3] Akyildiz, I. F., and Stuntebeck, E. P., “Wireless underground sensor networks,” *Ad Hoc Networks*, 2006.  
[4] M. Ramesh, “Design and development of a LoRa-based wireless sensor network for landslide monitoring,” *IEEE Sensors J.*, vol. 21, no. 14, pp. 16027–16037, 2021.  
[5] Microchip Technology Inc., *ATmega328P Datasheet*, DS40002061A, 2021.  
[6] Semtech Corporation, *SX1276/77/78/79 Datasheet*, DS1276, 2019.  
[7] Analog Devices, *ADXL345: 3-Axis Digital Accelerometer*, Rev. E, 2022.  
[8] Winbond Electronics, *W25Q64JV Datasheet*, Rev. H, 2022.  
[9] The Qt Company, *Qt 6 Documentation*, 2023.

**Local shelf (Vector4Engineering, Addis Ababa):** Ra-02, capacitive moisture v1.2, and ADXL345 in stock.
