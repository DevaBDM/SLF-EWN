---
tags:
  - slf-ewn
  - project
  - proposal
aliases:
  - Project Proposal
  - Design Changelog
  - Development Notes
parent: "[[SLF-EWN Hub]]"
---

# Project overview

← [[SLF-EWN Hub]] · Formal text: [[Full Proposal]] · Mentor challenges: [[Challenges and Solutions]] · Live gaps: [[Status and open issues]] · Standards: [[Project Overview#IEC standards and feasibility]] · Mentor pack: [[Mentor readiness]]

Buried nodes at **1 m**, **ADXL345** + capacitive moisture, **Ra-02** 433 MHz through soil, tower **W25Q64** + USB, **Qt 6 / QML**. Life target **3–7 years**. Five or more nodes per tower.

---

## What we decided

When notes disagree, **this table wins.** Archive drafts that still say LIS3DH, MicroSD, AO3401, or 2-layer SMD are stale.

| Topic | Not this | **This (as built)** |
|:------|:---------|:--------------------|
| Accel | LIS3DH (`WHO_AM_I` 0x33, ~2 µA) | **ADXL345** (`DEVID` 0xE5; always-on rail for INT wake) |
| Radio | “RFM95W” as the shop part | **Ra-02** (SX1278). Same SPI map as RFM95W in Proteus |
| Tower log | MicroSD / filesystem | **W25Q64** SPI NOR (no FAT) |
| FET | AO3401 SMD | **ZVP2106A** TO-92 |
| PCB | 2-layer SMD | **1-layer THT** + jumpers |
| UI | Qt Widgets / web | **Qt 6 + QML** (practical, not a marketing mock) |
| Tower power | Dual Li-SOCl₂ only | **USB 5 V** (+ optional battery) |
| CAD | Blender as the enclosure | **SolidWorks** print · Blender film |
| Firmware | Arduino core / one `.ino` / third-party device libs | **AVR-GCC C**, self-written drivers; only toolchain `.h` (avr/*, stdint, string) — [[Code Hub#Self-implemented firmware (supply-chain / trust policy)]] |

Shop stocks **ADXL345**; code and fakes target `DEVID 0xE5`. LIS3DH (~2 µA) remains a future drop-in if available — only `transmitter_accelerometer.c` + fake would change.

---

## Why this stack

Surface rain gauges and cameras see failure late. Subsurface moisture and deformation are earlier, but burial means power, waterproofing, soil load, and through-earth radio. Commercial systems are costly or need maintenance.

**Contributions this build actually makes:**

- Pre-flash on an ISP jig — **no ICSP** on the buried board (no 5 V programmer on a 3.6 V cell).
- **99.9%** hibernation: FET-gates **moisture + LoRa only**; ADXL345 stays on always-on rail for INT; WDT 15 min.
- Tower **W25Q64** log so the PC can be unplugged.
- Qt 6 / QML that **polls** the tower UART (`0xAA`) **or** runs simulation (never mixed).
- Proteus fake SPI slaves so the **same `.hex`** is used in sim and in the DIP-28.

---

## Team and constraints

| Team | Owns |
|:-----|:-----|
| **A** | 1-layer PCB, Proteus, AVR firmware |
| **B** | [[Mechanical Hub\|Capsule / SolidWorks]] |
| **C** | [[Dashboard Hub\|Dashboard]] |
| **D** | Film (Blender) — dimensions from Mechanical Hub only |

**Must follow for this prototype**

1. **Dashboard:** practical engineering UI (SAFE / WARNING / CRITICAL). Qt 6 + QML. Match [[Firmware Hub#Data formats]] — no second protocol.  
2. **PCB:** one copper layer; crossings = jumper or top-side solder blob. 100% THT. Socket the MCU; female headers for Ra-02.  
3. **Firmware:** bare-metal AVR-GCC (no Arduino). Same master `.hex` in Proteus and in the socket. Change pins/bytes only in [[Firmware Hub]] / `Code/`, then the matching sim or PCB note.

---

## How a day in the field looks

**Node (as in `transmitter.c`).** Hibernate (WDT 8 s ticks; **`SIM_FAST_WAKE` → 1 tick in the sim build**) **or** INT0 → FET on 5 ms → accel + ADC1 → FET off → 12-byte CRC frame → FET on 2 ms → SPI `0x8D` + payload → FET off. Default ID **`0x0104`**. No `(id % 30)` jitter yet.

**Tower (`receiver.c`).** DIO0 ISR clocks 12 bytes (SPI Mode 1) → CRC → 16-byte flash record. PC **polls** UART 115200 (`0xAA` replies). Unplugged PC: still logs until the 128 KB stand-in is full.

Bytes: [[Firmware Hub#Data formats]]. Pins: [[Transmitter Simulation]] / [[Receiver Simulation]]. Sources: [[Code Hub]]. Flow: [[SLF-EWN Hub#Data flow (as built)]].

---

## Timeline (working days)

| Phase | Days | Output |
|:------|:-----|:-------|
| Design | 1–3 | Schematic, PCB, Proteus |
| Buy / fab | 4–5 | JLCPCB, parts |
| Firmware | 3–6 | Sleep, LoRa, ADXL345, flash |
| Dashboard | 5–7 | Qt, serial, SQLite |
| Assemble | 7–8 | Solder, pre-flash, pot |
| Test | 9–10 | Bench, soil barrel, µA |
| Docs | 10–12 | Report, demo |

---

## Bill of materials

**Pair < $75.** Node ~$27, tower ~$15.50. Board photos: [[Transmitter PCB]] · [[Receiver PCB]].

### Buried node (~$27.45)

| Part | Qty | Est. USD |
|:-----|:---:|--------:|
| ATmega328P-PU + DIP-28 socket | 1+1 | 2.80 |
| Ra-02 433 MHz | 1 | 5.00 |
| Capacitive moisture v1.2 | 1 | 3.50 |
| ADXL345 module | 1 | 2.50 |
| ZVP2106A TO-92 | 1 | 0.25 |
| Li-SOCl₂ AA ~2400 mAh | 1 | 4.00 |
| Supercap 1 F / 5.5 V | 1 | 1.50 |
| FR-4 1-layer 50×50 mm | 1 | 2.00 |
| PVC ~60×250 mm + caps | 1 | 3.00 |
| SMA-TH + female 2.54 mm headers | — | 1.40 |
| Passives (~15) | — | 1.50 |

Plus helix wire (~17 cm) and 2-pin power jumper.

### Surface tower (~$15.50)

| Part | Qty | Est. USD |
|:-----|:---:|--------:|
| ATmega328P-PU + socket | 1+1 | 2.80 |
| Ra-02 | 1 | 5.00 |
| W25Q64 8 MB | 1 | 1.20 |
| FT232RL module | 1 | 3.00 |
| LM1117-3.3 TO-220 | 1 | 0.50 |
| FR-4 1-layer 95×70 mm | 1 | 2.00 |
| Passives + 16 MHz + 22 pF | — | 1.00 |

Shared jig: Arduino as ISP ~$25 (one for the lab). Optional outdoor 433 MHz whip for the tower.

---

## Power (node)

**Li-SOCl₂ 3.6 V (AA ~2400 mAh typical):**

- Energy density up to **~710 Wh/kg** — small capsule.
- Self-discharge **<1%/year** at 20 °C — required for 3–7 years.
- **−60…+85 °C** — buried temperature swings.
- **Flat** voltage until end of life — simpler brown-out story than Li-ion sag.

TX peaks **~120 mA** when the PA turns on. A thionyl cell’s continuous current is far below that, so a parallel **1 F / 5.5 V** supercap on the **transmitter** trickle-charges and supplies the burst. Do **not** use a 3.6 V-rated memory-backup cap sitting at its ceiling — life collapses. The **receiver / tower does not use this supercap** (USB-powered). Circuit split: [[EasyEDA Hub#Circuit decisions (TX vs RX)]].

**2-pin jumper (master disconnect):** storage and shipping (zero drain), safe handling while inserting the socketed MCU, no slide switch to fill with dirt.

BOD: Arduino factory 4.3 V / 2.7 V will **lock** the chip as the cell ages. Buried fuses: [[Firmware Hub#Pre-flash (off the PCB)]].

**Tower:** USB 5 V → LM1117-3.3. Optional battery is backup, not the primary design. Simulation may omit the regulator (5 V parts). Real Ra-02 on the PCB needs 3.3 V.

---

## Radio

Open-air LoRa can be **10–15 km**. Soil is a different channel.

| Effect | What happens |
|:-------|:-------------|
| Wet clay | ~**10–40 dB/m** attenuation (moisture dependent) |
| First metre | ~**20–40 dB** gone before the wave reaches air |
| Soil–air boundary | Wave **refracts** and spreads along the surface |
| After breakout | Ordinary LoRa in air — hundreds of metres |

Design target: **1 m through soil**, **100–200 m** once in air.

Airtime: SF7–9, 125 kHz, payload **< 500 ms**. Typical ~**290 bps**; SF10 time-on-air ~**330 ms**. Sensitivity to about **−148 dBm**; node TX **14–20 dBm**.

**λ/4 in air** (VF ≈ 0.95 for copper):

$$\lambda/4 = \frac{3\times10^8}{433\times10^6\times 4}\times 0.95 = \mathbf{16.45\ \mathrm{cm}}$$

In a material: $\lambda_{\mathrm{med}} = \lambda_{\mathrm{air}}/\sqrt{\varepsilon_r}$.

| Medium | $\varepsilon_r$ | Electrical length |
|:-------|:---------------:|:------------------|
| Air pocket | 1 | stays ~16.5 cm — **use this** |
| Wet clay | ~30 | shrinks ~80% → ~**3 cm**, and moisture **moves** $\varepsilon_r$ |

A bare wire in clay is never resonant for long. Wind **17.0 cm** of solid copper on a **5 mm** mandrel, **18–22** close turns → **~3.5 cm** tall. Keep that coil in an **air-filled** plastic volume. Helix **vertical**; tower whip **vertical**. Tower antenna **1.5–2 m** on a pole (dirt at ground level eats the incoming wave). Buried capsule **level**. Tower: high-gain **omnidirectional outdoor** whip — the node helix is electrically small; the tower is not size-limited.

---

## Where we deploy

| Site | Why |
|:-----|:----|
| Mine walls / pit benches | Wall creep, water ingress; 7-year cell matches a mine phase; no cable for machines to snag |
| Highway / dam / abutment earthworks | Settlement and liquefaction during and after construction |
| Village hillsides | Dense cheap grid → evacuation lead time |

---

## Shop (Vector4Engineering)

![[Ra-02 vector4engineering.png]]
![[ADXL345 vector4engineering.png|ADXL345 — as-built accel]]
![[capacitive soil vector4engineering.png|Moisture v1.2]]

## Others
![[atmega328p.png]]
![[li-socl battery.png]]
![[Lora module.png]]
![[antena.png]]

Datasheets to keep: ATmega328P, ADXL345, SX1278/Ra-02, W25Q64, moisture v1.2.

---

## IEC standards and feasibility

**Mentor check:** confirm the project is **feasible** under normal IEC practice.  
**Conclusion:** yes. The concept (buried moisture + tilt node, 433 MHz LoRa, primary cell, surface tower, PC dashboard) sits inside established IEC domains. That means the approach is **allowed and designable** under those frameworks — not that we already hold accredited lab certificates.

### Feasibility statement (quote for reviews)

> SLF-EWN is feasible with respect to IEC practice: a sealed, battery-powered subsurface sensor with short-range sub-GHz radio and a surface acquisition unit falls within established IEC scopes for ingress protection (**IEC 60529**), environmental testing (**IEC 60068**), EMC (**IEC 61000**), ICT safety (**IEC 62368-1**), and primary batteries (**IEC 60086**). Radio operation is constrained by spectrum regulation (typically ETSI / national SRD rules), not by an IEC prohibition.

### IEC families mapped to this build

| Area | Standard family | Role for SLF-EWN |
|:-----|:----------------|:-----------------|
| Enclosure / water / dust | **IEC 60529** (IP code) | Capsule and tower housings; buried/wet intent → IP67/IP68-class design target; outdoor tower often IP65+ |
| Environmental robustness | **IEC 60068** series | Temperature, humidity, vibration, shock qualification path |
| Mechanical impact | **IEC 62262** (IK code) | Outdoor tower boxes |
| EMC (generic) | **IEC 61000-6-1** immunity, **IEC 61000-6-3** emissions | Residential / light-industrial baseline; industrial sites may use 6-2 / 6-4 |
| EMC (basic tests) | **IEC 61000-4-2** ESD, **-4-3** radiated RF, **-4-4** EFT, **-4-5** surge, **-4-6** conducted RF | Lab methods if formal EMC is required; layout already aims at quiet RF and decoupling |
| Electrical safety | **IEC 62368-1** | Strongest fit for **tower + USB + host**; outdoor ICT provisions; buried 3.6 V primary node is low electrical hazard |
| Primary batteries | **IEC 60086** series | Li-SOCl₂ long-life primary cells |

### Not IEC blockers

| Topic | Note |
|:------|:-----|
| 433 MHz LoRa | Spectrum use → **national rules + ETSI** (e.g. EN 300 220), not an IEC ban |
| Landslide EWS *process* | **ISO 22327:2018** / **ISO 22328** series (early-warning guidelines) — supports system role, not PCB design |
| Functional safety SIL | **IEC 61508** only if we formally claim a safety integrity level — **not** required for this prototype |
| Student prototype | IEC is written for products; we **reference** and design *toward* it; full compliance is a later industrial step |

### Mapping to our design choices

| Choice | Feasibility note |
|:-------|:-----------------|
| Sealed capsule ~1 m | **IEC 60529** + **IEC 60068** path |
| Li-SOCl₂, FET gating, µA sleep | Primary-cell practice under **IEC 60086** context |
| Short 12-byte 433 MHz frames | Equipment EMC under **IEC 61000**; air interface under spectrum rules |
| Tower + USB + Qt | **IEC 62368-1**; outdoor surge/ESD under **IEC 61000-4-x** |
| Host thresholds / zones | Aligns with EWS process (**ISO 22327**); not an electrical conflict |

### What feasible does *not* mean

| Claim | Status |
|:------|:------:|
| Concept aligns with IEC domains | **Yes** |
| We already hold IEC test reports | **No** |
| Multi-year life without measurement | **No** — still measure sleep current, seals, soil RF |
| SIL claim under IEC 61508 | **Not claimed** |

Formal product certification would need an accredited lab later. This prototype phase designs toward these standards and closes critical risks by measurement (see [[Status and open issues]]).

Also summarised in [[Full Proposal#7.3 IEC standards and feasibility]].

**Broader mentor questions** (spectrum, safety claims, cost, power proof, ethics, FAQ): [[Mentor readiness]].

---

## Later (not this build)

Pads on the TX board for an **external digital extensometer** — not stuffed in this prototype.

**Draw-wire.** High-tensile steel from the capsule to stable rock or a deep stake. A rotary encoder measures pull. ~**0.1 mm** of creep is visible as counts on the AVR. Compact; SPI/I²C or quadrature depending on the module.

**TDR.** Cheap coax through the slope. A shear plane pinches the cable and changes impedance; a pulse locates the pinch to centimetres. Needs a fast analog front-end — good as **report future work**, not this BOM.

**In-place inclinometer.** SCA103T-class (or similar) logs slow angular creep over days. Qt charts a rising slope hours before acceleration. ADXL345 still covers **sudden** shocks.

**Out of scope for this prototype BOM (planned Phase 2):** see [[Data path future architecture]] —

- **Secondary LoRa on the tower** (node RX stays on primary Ra-02; backhaul does not block listening).  
- **USB on the tower kept** for dump, debug, firmware update.  
- **Tower siren:** critical node message (`0x80`) latches loud alarm; host/handheld **SILENCE** when linked; otherwise alarm continues.  
- **Tower field power:** **solar** primary + **battery backup** (USB kept for lab/service).  
- **Tower self-telemetry:** battery, uptime, flash fill, siren/radio health logged and shown on host beside node data.  
- **Host registry:** SQLite + dashboard register **towers** (location/data), **nodes**, and **node↔tower** links.  
- **Handheld** MCU + mini LCD: concise status; collect via LoRa or USB from tower; USB data-mule to host.  
- **Host LoRa** for direct tower link when in range; otherwise handheld → USB → Qt.  
- Mesh hops in valleys; ML false-alarm filtering; extensometer/TDR/IPI as above.

Tracked gaps and phase status: [[Status and open issues]].
