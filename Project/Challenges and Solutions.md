---
theme: white
width: 1100
height: 700
margin: 0.06
minScale: 0.2
maxScale: 2.0
slideNumber: true
controls: true
progress: true
center: false
loop: false
transition: fade
transitionSpeed: default
enableLinks: true
enableOverview: true
enableChalkboard: false
notesSeparator: "note:"
---

## How to open this deck (Slides Extended)

1. Install / enable community plugin **Slides Extended** (fork of Advanced Slides).
2. Open this note: `Project/Challenges and Solutions.md`.
3. Command palette → **Slides Extended: Show slide preview** (wording may vary slightly by version).
4. Click into the preview, then use **arrow keys**. Press **O** for overview.

**Separators:** a line that is only `---`, with a blank line above and below, starts a new horizontal slide.

**Speaker script:** [[Challenges Presentation Script]]

**If preview is blank:** close preview, ensure this file’s YAML frontmatter has no custom `separator` regex, then reopen preview.

---

## SLF-EWN

### Phase 1 engineering challenges, quantitative analysis, and design responses

*(Phase 2/3: short review only — full steps in [[Phase 2 and Phase 3 plan]])*

**Subterranean Landslide and Flood Early-Warning Network**

This deck is the **challenges and solutions** record for the prototype (Phase 1): why each hard constraint forced a specific technical response. It is meant for technical mentor review, not a marketing overview.

Each major section follows **problem → analysis → implementation → status**.


| Item | Content |
|:-----|:--------|
| Problem domain | Subsurface moisture and motion before slope failure |
| Hardware | ATmega328P · ADXL345 · capacitive moisture · Ra-02 (SX1278) 433 MHz · W25Q64 · 1-layer THT |
| Software | Bare-metal AVR-GCC · Qt 6 / QML host |
| Method on every topic | **Problem → analysis (equations / limits) → implementation → residual risk** |

Technical design presentation — full detail

*Presenter tip: Long-form technical deck. Use overview (O) to jump sections; each slide is self-contained.*

---

## How this deck is organised

**Primary focus:** Phase 1 challenges—the conflicts we faced and the quantitative or architectural solutions in the current node, tower, and host design.

**Secondary:** residual Phase 1 validation risk, then a **short** Phase 2/3 look-ahead (not a full second project pitch).

| Section | Topics |
|:--------|:-------|
| **A** | Architecture, constraints, method |
| **B** | Air frame size and protocol |
| **C** | Why sub-GHz and why LoRa |
| **D** | Zones, tracking, thresholds, **capacitive vs resistive moisture** |
| **E** | Energy: cell, duty cycle, P-MOSFET, supercap |
| **F** | Dual wake: WDT and accelerometer INT |
| **G** | Soil path loss, antenna, PCB RF noise |
| **H** | PCB size, 1-layer rules, HF placement |
| **I** | Flash, collisions, pre-flash, encryption, **firmware supply-chain trust** |
| **J** | Risk register, Phase 1 validation, **brief Phase 2/3 review**, conclusions |

**Related project notes:** [[Project Overview]] · [[Firmware Hub]] · [[Code Hub]] · [[Dashboard Hub]] · [[Status and open issues]] · [[Phase 2 and Phase 3 plan]] · [[Data path future architecture]] · [[Full Proposal]]

---

## A1 — System architecture

### What this system is solving

SLF-EWN is a **buried early-warning chain**: nodes at about 1 m sense moisture and motion, send a short LoRa packet, the tower logs it, and the host turns IDs into map places and alarms. Every later challenge (energy, RF, frame size) is constrained by this split of responsibilities.

```mermaid
flowchart TB
  subgraph node["Buried node · depth ≈ 1 m"]
    M[Capacitive moisture · ADC1]
    A[ADXL345 · SPI Mode 3]
    N[ATmega328P · 8 MHz internal RC · 3.6 V]
    M --> N
    A --> N
  end
  AIR["Air interface · 12-byte PDU · LoRa 433 MHz"]
  subgraph tower["Surface tower"]
    R[ATmega328P · 16 MHz crystal · USB 5 V]
    F[W25Q64 SPI NOR · 16-byte records]
    R --> F
  end
  HMI["Host PC · Qt 6/QML · poll 115200 8N1 · 0xAA framing"]
  N --> AIR --> R --> HMI
```

**End-to-end behaviour**

1. Node wakes on timer or motion interrupt, enables sensor/radio rail, samples, packs a **12-byte** frame, transmits, returns to power-down.
2. Tower validates CRC, appends a **16-byte** flash record (record ID + PDU).
3. Host **polls** the tower; it does not rely on an unsolicited 12-byte stream.
4. Host maps **Device ID** to zones, thresholds, and map pins in SQLite.

---

## A2 — Segment parameters

### Four segments, four power and clock stories

The node must last years on a primary cell; the tower may stay powered and listen continuously; the air segment is lossy and shared; the host holds geography and policy. Confusing those roles is how projects grow bloated frames or always-on radios.

| Segment | Clock / power | Primary responsibility |
|:--------|:--------------|:-------------------------|
| Node | 8 MHz RC, Li-SOCl₂ 3.6 V | Sense, pack, TX, µA-class sleep |
| Air | 433 MHz LoRa (SX1278 / Ra-02) | Fixed 12-byte payload through soil then air |
| Tower | 16 MHz, USB → 3.3 V | Continuous RX path, NOR log, command server |
| Host | Desktop | Places/zones, per-node limits, charts, map |

**Production vs simulation**

- Same application-level frame layout in field and Proteus.
- Proteus may use fake SPI slaves (`0x8D` FIFO path); field firmware uses real SX1278 registers.
- Do not ship the simulation-only SPI contract to hardware.

---

## A3 — Global design constraints

### Hard bounds that drove part choices

These are not soft preferences. Burial depth, 1-layer THT, and multi-year life force primary chemistry, short airtime, and socketed pre-flash. Shop stock forced ADXL345 instead of the lower-µA LIS3DH option.

| Constraint | Quantitative / hard bound | Design consequence |
|:-----------|:--------------------------|:-------------------|
| Service life | 3–7 years without opening capsule | Primary cell; average current in low µA |
| Burial | ≈ 1 m | Soil RF path + subsurface moisture/tilt |
| PCB process | **1 copper layer**, 100% THT | Jumpers only; no 2-layer escape routing |
| MCU | ATmega328P, bare-metal C | Fixed binary protocols; no RTOS |
| Air occupancy | Prefer short time-on-air | Fixed 12-byte PDU |
| Tower autonomy | PC may be disconnected | SPI NOR logging, not host-side filesystem |
| Parts | Local distributor shelf | ADXL345 adopted; LIS3DH remains future option |

**Assumptions called out later:** soil attenuation coefficients are **literature-scale** until barrel/pit calibration; sleep current is **designed** but must be **measured** on hardware.

---

## A4 — Method used on every challenge

### How this deck stays technical

Each challenge is treated the same way so a mentor can audit the reasoning: conflict of requirements, quantitative bound, what is implemented, what is still open. That pattern repeats from frame size through RF and energy.

| Step | Question answered on the slide |
|:-----|:-------------------------------|
| **1. Problem** | Which requirements conflict? |
| **2. Analysis** | Equation, datasheet limit, or literature bound? |
| **3. Implementation** | What is in schematic, firmware, or Qt **now**? |
| **4. Status** | Verified / designed / open measurement or code? |

This keeps the review auditable: numbers first, then the part choice, then residual risk.

---

## B1 — Frame size: problem

### Why payload size is an engineering challenge

Every extra field on the air costs energy, extends collision windows, and weakens the post-soil link margin. Geotechnical wishlists (voltage, temperature, GNSS, free text) conflict with a sealed µA-class node.

### Observability versus energy, collisions, and soil margin

```mermaid
flowchart LR
  Rich[Rich telemetry<br/>voltage · temp · GNSS · logs] --> ToA[Longer time-on-air]
  ToA --> E[More joules per report]
  ToA --> C[Wider collision window]
  ToA --> L[Harder link after soil loss]
  Fixed[Fixed 12-byte PDU] --> Short[Bounded ToA]
  Short --> P[Simple CRC · deterministic parse · lower energy]
```

**Problem.** A geotechnical wishlist wants many fields. A buried node on a primary cell, sharing one LoRa demodulator with neighbours, and pushing energy through wet soil, cannot treat payload size as free.

**Design decision.** Freeze a **12-byte** protocol data unit (PDU) with CRC-8. Put GIS, labels, and rich history on the **host**, not in every RF packet.

---

## B2 — Frame size: bit-level layout

### Frozen 12-byte PDU

The solution is a fixed layout: identity, moisture, three-axis tilt, status, CRC. Nothing variable-length on the air. Host software owns labels and history.

```
Offset   0         2         4         6         8        10    11
         [ ID u16 ] [ moist ] [ X i16 ] [ Y i16 ] [ Z i16 ] [st] [crc]
         multi-byte fields are big-endian
```

| Offset | Type | Semantic | Notes |
|:------:|:-----|:---------|:------|
| 0–1 | u16 BE | Device ID | Default `0x0104` |
| 2–3 | u16 BE | Moisture 0…10000 | From 10-bit ADC scaled in firmware |
| 4–5 | i16 BE | Axis X | Tilt / acceleration encoding |
| 6–7 | i16 BE | Axis Y | |
| 8–9 | i16 BE | Axis Z | Host uses max \|axis\| vs angle thresholds |
| 10 | u8 | Status | `0x80` = emergency (INT) wake; seq; accel fault bit |
| 11 | u8 | CRC-8 | Poly-style `0x07`, init `0x00`, over bytes 0–10 |

Tower **discards** frames failing CRC. Same 12 bytes appear inside each 16-byte flash record.

Canonical tables: [[Firmware Hub#Data formats]]

---

## B3 — Frame size: host and storage wrappers

### Where the extra bytes live

Flash and USB add structure around the PDU without enlarging the RF packet. The tower stores 16-byte records; the host uses `0xAA` framed replies so dumps are not a raw 12-byte stream.

| Layer | Size | Contents |
|:------|:-----|:---------|
| Air PDU | **12 B** | ID, moisture, XYZ, status, CRC |
| Flash record | **16 B** | u32 record ID ‖ copy of 12 B PDU |
| USB reply | variable | Magic **`0xAA`** ‖ cmd ‖ u32 length ‖ payload |

**Why wrappers exist**

- Flash needs a monotonic record index for dump/incremental poll (`'D'` / `'N'`).
- USB must support **command/response** (status, full dump, incremental, clear)—not a continuous 12-byte firehose.
- Host can structure polling between commands.

**Open if encryption is added:** ciphertext expansion changes ToA and possibly flash record size; both must be re-budgeted.

---

## C1 — Why not 2.4 GHz for a buried node

### Frequency is a soil-penetration challenge

Compact IoT culture defaults to 2.4 GHz. For a metre of wet clay, that default is the wrong starting point: attenuation rises with frequency, and association-heavy stacks waste energy.

### Carrier frequency and soil attenuation

Wet soil is a **lossy dielectric**. In the bands used by compact IoT radios, path loss through wet clay **worsens as frequency rises**.

| Technology | Typical role | Fit for 1 m wet soil + long life? |
|:-----------|:-------------|:----------------------------------|
| Wi-Fi 2.4 GHz | High throughput, AP infrastructure | No — energy, association, soil loss |
| BLE 2.4 GHz | Short-range PAN | No — range and soil path |
| LoRa 2.4 GHz | Some modern modules | Better protocol, still higher soil loss than sub-GHz |
| **LoRa 433 MHz** | Low-rate long-range ISM | **Yes — selected** |

**Literature-scale attenuation (wet clay, sub-GHz):**

$$
\alpha \sim 10\text{–}40\ \mathrm{dB\,m^{-1}}
\quad\Rightarrow\quad
L(d{=}1\,\mathrm{m}) \sim 20\text{–}40\ \mathrm{dB}
$$

**Segment targets used in design**

| Segment | Target |
|:--------|:-------|
| Vertical through soil | ≈ **1 m** |
| After breakout (air) | **100–200 m** with elevated tower antenna |

Open-air LoRa marketing figures (many km) **do not** apply to the buried metre.

---

## C2 — Why LoRa modulation (CSS), not any 433 MHz FSK

### Sub-GHz alone is not enough

433 MHz helps the path; **LoRa CSS** helps the demodulator recover weak signals at low rate. Simple FSK at the same frequency would not buy the same sensitivity class for a 12-byte telemetry burst.

### Sensitivity at low information rate

```mermaid
flowchart TB
  P[12-byte PDU · low duty cycle] --> CSS[LoRa chirp spread spectrum]
  CSS --> SEN[High receiver sensitivity<br/>order of −148 dBm class devices]
  CSS --> TOA[Time-on-air set by SF, BW, CR, payload]
  SEN --> LINK[Close link after soil attenuation]
  TOA --> BUDGET[Bound energy and collision duration]
```

| LoRa property | Use in SLF-EWN |
|:--------------|:---------------|
| High sensitivity | Recovers weak post-soil signals |
| Low bit rate acceptable | Telemetry is small and infrequent |
| Configurable SF/BW | Trade ToA vs robustness (SF7–9 class target) |
| SX1278 ecosystem | SPI control from AVR; Ra-02 module on shelf |

**What we do not need from LoRa:** video, IP mesh, or phone-centric provisioning.

**Implementation:** Ra-02 (SX1278) on 2.54 mm headers; production drivers in `transmitter_lora.c` / `receiver_radio.c`; RF settings must match on both ends (frequency, SF, BW, sync, etc.).

---

## C3 — RF configuration checklist

### Matched parameters on both ends

TX and RX must agree on frequency, spreading, bandwidth, sync, and payload length. Mismatched SF/BW is a silent system failure. ToA and TX power still sit inside the energy budget from section E.

| Parameter | Design target |
|:----------|:--------------|
| Frequency | **433.0 MHz** ISM |
| Module | **Ra-02** (SX1278) |
| Spreading / bandwidth | SF7–9 class, **125 kHz** (TX/RX matched) |
| TX power | ≈ **14–20 dBm** (PA_BOOST class) |
| Payload | **12 bytes** + CRC policy as implemented |
| ToA preference | **&lt; 500 ms** per packet when possible |
| Tower antenna | Vertical polarisation, **1.5–2 m** AGL, omni |

**Conceptual link budget**

$$
P_{\mathrm{rx}} = P_{\mathrm{tx}} + G_{\mathrm{tx}} + G_{\mathrm{rx}} - L_{\mathrm{soil}} - L_{\mathrm{air}} - L_{\mathrm{misc}}
$$

**Status:** architecture and drivers exist; **measured path loss on our helix + soil is still open (H4)**.

---

## D1 — Zones and identity: problem

### Identity on the wire, geography on the host

The air frame can carry a Device ID; it cannot carry polygons, place names, or per-site threshold tables without breaking the energy and ToA story.

### RF packets cannot carry a full GIS model

```mermaid
flowchart TB
  ID[Device ID in every 12-byte PDU] --> NOR[Tower flash]
  NOR --> DB[Host SQLite]
  DB --> PROF[Node profile]
  PROF --> PLACE[Places · zone polygon]
  PROF --> LIM[Per-node thresholds]
  PROF --> MAP[Map pin · OSM / satellite]
```

**Problem.** Operators need site name, zone, category (mine wall vs village slope), and map position. Putting all of that on-air would enlarge the PDU and still not replace a real GIS database.

**Split of responsibility**

| Information | Lives where | Why |
|:------------|:------------|:----|
| Who transmitted | **Device ID** on air + flash | Stable primary key, 2 bytes |
| Zone / place name | Host **Places** | Polygons, labels, no ToA cost |
| Category | Host profile / planning | Drives default thresholds |
| Coordinates | Host map (click or lat/lon) | No GNSS in node BOM |

---

## D2 — Tracking after a landslide

### Physical location may move; ID must not

After a slide, the capsule may translate or rotate. The network still needs a stable identity for logging and alerts. Map pins and zone membership are host-side and may need operator update; the node does not run GNSS.

### Geometry can change; identity must not

```mermaid
flowchart LR
  INST[As-installed map pin and zone] --> EVT[Mass movement]
  EVT --> DISP[Unknown translation / rotation of capsule]
  DISP --> SAME[Same Device ID continues to TX]
  SAME --> HIST[Entire history remains under that ID]
```

**Problem.** A real failure may move the capsule. If identity were tied only to map coordinates, time-series continuity would break.

**Stable after the event**

- Device ID in firmware and every PDU
- NOR and SQLite history keyed by ID
- Emergency status when motion INT wakes the node

**Expected to be updated by humans after survey**

- Map pin position
- Zone membership if the node leaves a polygon
- Interpretation of tilt relative to a new rest attitude

**Residual limitation:** no onboard GNSS — displacement is **not** estimated in the node; only motion **detection** + ID-continuous logging.

---

## D3 — Why moisture and tilt at about 1 m depth

### What the node is allowed to observe

Surface-only instruments can miss saturation and motion developing underground. Depth ≈ 1 m is both a useful sensing horizon and the same path the radio must cross. This is early-warning instrumentation, not a full slope FEM.

### Surface instruments often see failure late

```mermaid
flowchart TB
  RAIN[Rain / infiltration] --> SAT[Saturation at depth]
  SAT --> STR[Effective shear strength drops]
  STR --> MOT[Creep or rapid acceleration]
  SAT --> M[Capacitive moisture]
  MOT --> A[ADXL345]
  M --> AL[Host thresholds + alerts]
  A --> AL
  A --> IRQ[Hardware INT · status 0x80]
```

| Observable | Engineering role | Explicit non-role |
|:-----------|:-----------------|:------------------|
| Moisture | Saturation **proxy** | Full hydro-mechanical FEM |
| Tilt / shock | Deformation / impact **indicator** | Survey-grade inclinometer log |
| Depth ≈ 1 m | Design burial + defines soil RF path | Variable-depth instrument string |
| Overburden / weight | **Site category** in planning | On-node load cell |

This build is an **instrumented early-warning prototype**, not a replacement for a full slope-stability study.

---

## D3b — Moisture probe: why not resistive?

### Corrosion is a lifetime challenge

Resistive forks inject current through pore water. In permanent wet soil that becomes electrolysis and electrode damage—unacceptable for a sealed multi-year burial.

### Resistive probes fail in permanent wet soil

```mermaid
flowchart LR
  R[Two bare electrodes<br/>in soil] --> DC[DC current through<br/>pore water]
  DC --> EL[Electrolysis · metal loss]
  EL --> COR[Corrosion · oxide films]
  COR --> DR[Reading drifts · open/short]
  DR --> LIFE[Short service life at 1 m burial]
```

| Issue | What happens with a **resistive** (conductivity) probe |
|:------|:------------------------------------------------------|
| **Galvanic path** | Measurement injects current through the soil between exposed metal |
| **Electrolysis** | Wet soil acts as electrolyte → electrode material dissolves or plates |
| **Corrosion** | Oxide and deposits change resistance → false “dry/wet” |
| **Maintenance** | Probes need cleaning or replacement — conflicts with **3–7 year sealed** goal |
| **Long bias** | If left powered, corrosion accelerates |

**Problem for SLF-EWN.** A sealed node at ≈ 1 m for years cannot accept a sensor that **eats itself** in the same water we are trying to measure.

---

## D3c — Capacitive moisture: design response

### Dielectric sensing plus gated power

Capacitive probes sense wetness through insulation; FET gating removes continuous bias. The host still treats the value as a **proxy** for thresholds, not a metrology certificate.

### Insulated electrodes · dielectric sensing · gated power

```mermaid
flowchart TB
  SOIL[Soil dielectric<br/>ε_r rises with water] --> CAP[Capacitive probe<br/>electrodes behind insulator]
  CAP --> AC[AC / charge transfer<br/>no sustained DC through soil]
  AC --> ADC[MCU ADC1 · 0…10000 scale]
  FET[P-MOSFET VCC_SW] --> CAP
  FET -.->|off most of 15 min| OFF[No continuous probe bias]
```

| Decision | Why it helps |
|:---------|:-------------|
| **Capacitive (v1.2 class) probe** | Senses change in **dielectric constant**; metal is not a DC electrode pair in the pore water |
| **Insulated contact with soil** | Greatly reduces electrolysis vs bare resistive forks |
| **Power only on VCC_SW** | Probe is biased only for the short measure window — less continuous electrochemical stress |
| **Sealed capsule + potting path** | Mechanical isolation of electronics from free water |
| **Host calibration / invert (F3)** | Map ADC to display %; fix polarity for this probe family |

**Status.** Capacitive part is **selected and in the BOM**. Residual work: two-point wet/dry calibration and polarity invert in firmware (**F3**), not a change of sensor physics.

**Not claimed.** Laboratory-grade volumetric water content (VWC) with temperature compensation — the node reports a **saturation proxy** for thresholds, not a metrology certificate.

---

## D4 — Threshold hierarchy

### Policy without enlarging the RF frame

Alarm bands are a **host policy** challenge: the node only reports numbers and an emergency bit. Different sites need different warn/critical levels without changing the 12-byte air layout.

### One global limit is wrong for every site


```mermaid
flowchart TB
  CAT[Site category<br/>mine / dam / village hillside] --> Z[Zone / Place polygon]
  Z --> DEF[Default warn / critical]
  DEF --> N[Optional per-node override]
  N --> UI[SAFE · WARNING · CRITICAL]
  IRQ[Status bit 0x80] --> UI
```

**Host defaults in the current software**

| Quantity | Warning | Critical |
|:---------|:-------:|:--------:|
| Moisture (display %) | **38** | **58** |
| Tilt magnitude max of absolute axes in degrees | **8°** | **16°** |
| Status `0x80` (motion-INT wake) | — | **Always CRITICAL** |

**Implemented in Qt:** node cards, alert log, Places, optional per-node limits, charts ([[Dashboard Hub]]).

**Open as workflow:** one-click **category templates** that bulk-apply defaults (mechanism for per-node limits already exists).

---

## E1 — Energy: problem statement

### Peak current versus average current

The LoRa PA needs order ~100 mA for milliseconds to tens of milliseconds. The cell that lasts years cannot supply that continuously. Sleep, gating, and buffering are mandatory—not optional optimisations.

### Peak PA current versus multi-year average current

```mermaid
flowchart TB
  CELL[Li-SOCl₂ AA-class cell<br/>high energy density<br/>limited continuous current]
  PA[Ra-02 / SX1278 PA<br/>order ~120 mA during TX]
  LIFE[3–7 year life ⇒ average current must stay µA-class]
  CELL --> LIFE
  PA --> LIFE
```

**Problem.** The chemistry that survives years underground cannot source LoRa PA peaks continuously. Leaving the radio or moisture sensor biased between reports would dominate the lifetime budget.

**Architecture response (following slides):** primary cell for energy, supercap for peaks, P-MOSFET to remove load bias, long WDT period, motion wake without powering LoRa until needed.

---

## E2 — Energy: cell capacity and average-current bound

### Order-of-magnitude life math

Stored charge sets a hard ceiling on average current. Design targets sit well below that ceiling so temperature, leakage, and self-discharge still leave margin.

**Stored energy (order of magnitude)**

$$
E \approx 3.6\,\mathrm{V} \times 2.4\,\mathrm{Ah} = \mathbf{8.64\,\mathrm{Wh}}
$$

**Hard upper bound on average current for 7 years** (neglect self-discharge):

$$
I_{\mathrm{avg,max}}
= \frac{2400\,\mathrm{mAh}}{7 \times 365 \times 24\,\mathrm{h}}
\approx \mathbf{39\,\mu\mathrm{A}}
$$

| Parameter | Design value | Rationale |
|:----------|:-------------|:----------|
| Chemistry | Li-SOCl₂ | Flat voltage, low self-discharge, temperature range |
| Self-discharge | &lt; 1 %/year typ. @ 20 °C | Calendar life |
| Target $I_{\mathrm{avg}}$ | **&lt; 10 µA** | Margin for cold, PCB leakage, ageing |
| BOD fuses | Off / 1.8 V class | Avoid 2.7 V BOD lock as cell ages |
| Disconnect | 2-pin jumper | Zero drain in storage; no dirt-filled switch |

---

## E3 — Energy: duty cycle — why sleep dominates

### Active charge is small; hibernate decides success

With a ~50 ms active window every 15 minutes, the duty cycle is ~10⁻⁵. Microamps while “asleep” dominate the yearly integral—hence ADXL policy and FET gating.

Take active window $\Delta t_{\mathrm{on}} \approx 50\,\mathrm{ms}$ and production period $T = 15\,\mathrm{min} = 900\,\mathrm{s}$:

$$
D = \frac{\Delta t_{\mathrm{on}}}{T} = \frac{0.05}{900} \approx \mathbf{5.6 \times 10^{-5}}
$$

If the active window draws $I_{\mathrm{on}} \approx 30\,\mathrm{mA}$ average:

$$
I_{\mathrm{on,avg}} = D \cdot I_{\mathrm{on}} \approx \mathbf{1.7\,\mu\mathrm{A}}
$$

**Interpretation for the review**

- Charge spent while awake is **small** in the yearly integral
- **Hibernate current** (MCU power-down + leakages + any always-on accel mode) decides success
- Therefore every always-on microamp needs a safety justification (e.g. motion INT)

```mermaid
stateDiagram-v2
  [*] --> HIBERNATE
  HIBERNATE --> ACTIVE: WDT period elapsed
  HIBERNATE --> ACTIVE: INT0 motion
  ACTIVE --> MEASURE: FET on · sample
  MEASURE --> TRANSMIT: LoRa TX
  TRANSMIT --> HIBERNATE: FET off
```

---

## E4 — P-MOSFET load switch (moisture + LoRa)

### Kill quiescent draw of hungry modules

Moisture and Ra-02 must not sit in standby for 15 minutes. A high-side P-MOS (ZVP2106A) switches **VCC_SW** only for measure/TX. ADXL stays on the always-on rail for motion INT.

### High-side power gating

```mermaid
flowchart LR
  BAT[Li-SOCl₂ 3.6 V] --> FET[ZVP2106A P-channel]
  FET --> RAIL[VCC_SW]
  RAIL --> MOIST[Moisture sensor]
  RAIL --> LORA[Ra-02]
  PIN[MCU PD4 · drive LOW = rail ON] --> FET
```

| Mode | Gate | Moisture and LoRa |
|:-----|:-----|:------------------|
| Hibernate | Held off (pull-up to VCC) | **Unpowered** — no quiescent module current |
| Measure / TX | Driven on for a short window | Powered only when needed |

**Why P-MOS high-side**

- Interrupts the **positive** supply to hungry loads
- Suitable TO-92 leakage is far below leaving modules in standby
- Pull-up keeps the rail off during MCU reset

**Part in BOM:** ZVP2106A (not a random SMD FET that violates the THT / shop constraint).

---

## E5 — Supercap for the TX current edge

### Buffer the PA pulse the cell cannot source cleanly

Charge ΔQ = I·T_air; a small electrolytic cannot hold the rail across LoRa airtime. A **1 F / 5.5 V** supercap on the **transmitter only** supplies the edge; ESR still matters on the bench.

Cell average current is microamps; PA wants ~**120 mA** for milliseconds.

**Charge for 120 mA × 2 ms**

$$
\Delta Q = I \Delta t = 0.12\,\mathrm{A} \times 0.002\,\mathrm{s} = \mathbf{240\,\mu\mathrm{C}}
$$

**Voltage dip**

$$
\Delta V = \frac{\Delta Q}{C}
$$

| $C$ | $\Delta V$ | Verdict |
|:----|:-----------|:--------|
| 100 µF | 2.4 V | Unacceptable on a 3.6 V rail |
| **1 F** | **240 µV** | Negligible |

**Implementation:** parallel **1 F / 5.5 V** supercap (avoid a 3.6 V memory-backup cap held at rated voltage for years). Cell trickle-charges the cap between wakes; cap supplies the PA edge.

**Status:** topology designed; **true sleep current on hardware still open (H3)**; production must clear **`SIM_FAST_WAKE` (F1)**.

---

## F1 — Dual wake: problem

### Timer-only wake is too slow for sudden failure

A pure 15-minute poll can miss rapid acceleration between samples. Continuous high-rate accel would destroy the energy budget. The conflict forces a dual-wake architecture.

### Pure timer sampling misses rapid onset

If the node only wakes every 15 minutes and **all** motion sensing is powered down, a fast failure between samples is invisible until the next WDT wake.

```mermaid
flowchart TB
  PD[MCU power-down] --> WDT[WDT scheduled wake]
  PD --> EXT[INT0 external interrupt]
  ACC[ADXL345 low-power activity / motion] --> EXT
  WDT --> NORM[Normal measure + TX]
  EXT --> EMRG[Same cycle · status 0x80 · host CRITICAL]
```

**Problem.** Safety-relevant sudden-slide detection conflicts with switching everything off for µA sleep.

**Resolution principle.** Gate **moisture and LoRa** hard; keep a **motion-capable** accel configuration and MCU INT0 alive; treat INT wakes as emergency in the PDU and UI.

---

## F2 — Dual wake: implementation behaviour

### WDT for routine reports; INT0 for emergencies

MCU power-down + 8 s WDT ticks build the 15 min (+ stagger) period. ADXL ACTIVITY → INT1 → MCU INT0 sets emergency status. After TX, accel returns to low-power activity mode.

```mermaid
sequenceDiagram
  participant WDT as Watchdog
  participant ACC as ADXL345
  participant MCU as ATmega328P
  participant FET as VCC_SW FET
  participant RF as Ra-02

  Note over MCU: Power-down · WDT and INT0 armed
  WDT->>MCU: period reached (15 min production)
  ACC->>MCU: activity INT (asynchronous)
  MCU->>FET: enable rail
  MCU->>MCU: moisture ADC + accel read · pack 12 B
  MCU->>RF: TX
  MCU->>FET: disable rail
  MCU->>MCU: power-down again
```

| Wake source | Meaning | Host effect |
|:------------|:--------|:------------|
| WDT | Scheduled telemetry | Normal thresholds |
| INT0 from accel | Sudden movement | Status `0x80` → **CRITICAL** |

**Production interval:** 15 minutes via WDT 8 s ticks (112 ticks ≈ 14.9 min).
**Simulation:** `SIM_FAST_WAKE` forces ~8 s — **must be off in field builds**.

**Open (F8):** characterise ADXL345 activity threshold, axis mapping, and current in the chosen low-power INT mode on real silicon.

---

## G1 — Soil path loss: analysis

### The buried metre is the hard RF segment

Path loss through wet soil can be tens of dB per metre at sub-GHz. Design uses literature-scale α until H4 measurements; frequency choice and short packets are the first mitigations.

$$
L_{\mathrm{soil}} \approx \alpha \cdot d
\quad\text{with}\quad
\alpha \sim 10\text{–}40\ \mathrm{dB\,m^{-1}},\quad
d \approx 1\ \mathrm{m}
$$

so the first metre alone can cost **20–40 dB** before the wave reaches air. After breakout, the problem becomes ordinary surface propagation plus antenna gains.

```mermaid
flowchart LR
  N[Node at depth] -->|L_soil| B[Soil-air interface]
  B -->|L_air + misc| T[Tower antenna]
  T --> R[SX1278 RX]
```

**Design levers (priority for this project)**

1. Sub-GHz carrier (433 MHz)
2. Buried antenna dielectric (air pocket)
3. Polarisation match (vertical–vertical)
4. Tower height 1.5–2 m AGL
5. Short ToA / adequate TX power

**Status:** levers implemented in design; **α for our soil still unmeasured (H4)**.

---

## G2 — Antenna: electrical length and clay

### Wavelength and dielectric loading

A quarter-wave at 433 MHz is ~17 cm in air. Wet clay raises effective ε_r and shortens electrical length unpredictably—worst when the slope is wettest.

**Free-space quarter-wave** at 433 MHz, $VF \approx 0.95$:

$$
\frac{\lambda}{4}
= \frac{c}{4f}\cdot VF
= \frac{3\times 10^{8}}{4\times 433\times 10^{6}}\times 0.95
= \mathbf{16.45\ \mathrm{cm}}
$$

**In a dielectric**

$$
\lambda_{\mathrm{med}} = \frac{\lambda_0}{\sqrt{\varepsilon_r}}
$$

| Medium | $\varepsilon_r$ (order) | Effect on a ~16.5 cm wire |
|:-------|:----------------------:|:--------------------------|
| Air cavity | ≈ 1 | Resonance remains usable |
| Wet clay | ≈ 30 | Scale toward **~3 cm**; $\varepsilon_r$ moves with moisture |

**Hard requirement:** the radiating element must live in an **air pocket** for the life of the installation—not packed in clay.

---

## G3 — Antenna: geometry

### Helix in an air pocket

A compact helix on a controlled mandrel fits the capsule better than a straight whip through mud. The critical rule is **air around the radiator**, not soil packing against the element.

| Parameter | Specification |
|:----------|:--------------|
| Type | Helix |
| Wire | Solid copper, cut ≈ **17.0 cm** |
| Mandrel | **5 mm** |
| Turns | ≈ **18–22** close-wound |
| Wound height | ≈ **3.5 cm** (capsule headroom) |
| Environment | Air cavity under top cap |
| Feed | Short board ANT → **SMA-TH** |

```mermaid
flowchart TB
  CAP[Top cap + seal]
  AIR[Air pocket + helix]
  SMA[SMA]
  RADIO[Ra-02]
  CAP --> AIR --> SMA --> RADIO
```

---

## G4 — Antenna: orientation and mechanical order

### Polarisation and stack order

Tower whip vertical; node helix oriented for consistent polarisation as far as the tube allows. Mechanical stack keeps SMA/air pocket above the RF module and moisture exit at the bottom.

Polarisation mismatch discards link margin already spent on soil loss.

```mermaid
flowchart LR
  H[Buried helix axis VERTICAL] --> M[Matched polarisation]
  W[Tower whip VERTICAL] --> M
  M --> G[Lower polarisation loss after breakout]
```

| Rule | Implementation |
|:-----|:---------------|
| Helix axis | **Vertical** |
| Tower whip | **Vertical** omni, 1.5–2 m AGL |
| Capsule | Installed **level** |
| Stack order | Electronics → short RF feed → SMA → helix in upper air volume |

**Status:** rules frozen; final **VSWR / range in assembled capsule open**.

---

## G5 — PCB as an EMI source into the antenna

### The board is part of the antenna problem

SPI edges, PA current, and long ANT traces couple noise into the radiator. Layout rules (short ANT, local decoupling, moisture ADC away from RF, ground pour) are link-budget items, not cosmetics.

### Self-interference on a 50×50 mm one-layer board

```mermaid
flowchart TB
  SPI[SPI clocks · MCU edges] --> CPL[Near-field coupling]
  PA[PA supply current spikes] --> CPL
  CPL --> FEED[Antenna feed / match]
  FEED --> DEG[Raised effective noise · match error]
```

**Problem.** The same small board hosts digital SPI (accel + radio) and a sensitive 433 MHz path. Poor layout spends decibels **before** the wave enters soil.

**Specified mitigations**

| Practice | Purpose |
|:---------|:--------|
| Minimum length ANT → SMA | Loss and loop area |
| 0.1 µF at MCU VCC/AVCC; bulk 10–100 µF at radio VCC | Digital and PA supply integrity |
| Moisture ADC route spatially away from RF/digital | Protect 10-bit samples |
| Ground pour where 1-layer allows | Return path |
| Headered Ra-02 | Controlled placement, field replacement |

**Status:** rules in EasyEDA notes; **post-fab RF/noise measurement open**.

---

## H1 — Transmitter PCB size and capsule

### Form factor is a constraint, not a preference

50×50 mm 1-layer THT must slide into a small-ID tube with battery, supercap, and helix air pocket. Every connector and ICSP header competes with that volume.

| Parameter | Value |
|:----------|:------|
| TX PCB | **50 × 50 mm**, 1-layer THT |
| Capsule ID | **50–75 mm** (clear board + SMA) |
| Length | **200–300 mm** class |
| Section | **Circular** — load distribution, auger installation |

```mermaid
flowchart TB
  T[Top seal · air cavity · helix]
  R[Ra-02 on headers]
  M[ATmega328P in DIP-28 socket]
  S[ADXL345 · ZVP2106A · decoupling]
  C[1 F supercap]
  B[Li-SOCl₂]
  P[Moisture blade through potted bottom]
  T --> R --> M --> S --> C --> B --> P
```

Rectangular boxes concentrate stress at corners under freeze–thaw; the cylinder matches the inline stack.

**Status:** dimensions locked; **SolidWorks deliverables open (M1)**.

---

## H2 — Single-layer PCB technology

### Shop and cost forced 1-layer THT

No inner planes, no fine-pitch escape. Crossings use jumpers; modules sit on female headers; MCU is socketed for pre-flash. Two-layer SMD remains a future process change, not this BOM.

```mermaid
flowchart TB
  X[Required net crossing]
  X --> J[Jumper wire or top-side solder blob]
  X --> L2[Second copper layer]
  J --> OK[Compliant with project rules]
  L2 --> NO[Out of scope for this prototype]
```

| Rule | Specification |
|:-----|:--------------|
| Layer count | **1** |
| Power traces | **30–40 mil** on high-current paths |
| Signal traces | ≈ **10 mil** |
| Decoupling | 0.1 µF within **5 mm** of VCC and AVCC |
| MCU | DIP-28 **socket** — pre-flash in lab |
| Modules | 2.54 mm female headers preferred |
| RX board | **95 × 70 mm** class |

Driven by cost, student fab process, and field repair—not only RF theory.

**Status:** layout in review; **fab not ordered (H1)**.

---

## H3 — Placement for high-frequency and analogue integrity

### Partition digital RF from slow analogue

Keep SPI/radio cluster tight; keep moisture ADC routing quiet; keep power path thick for PA peaks. Placement is the practical EMI control available on one copper layer.

| Cluster | Include | Goal |
|:--------|:--------|:-----|
| **Hot** | MCU SPI, radio SPI, local ceramics, PA bulk | Minimise loop area and supply bounce |
| **Quiet** | Moisture analogue route, probe pads | Reduce digital/RF coupling into ADC |
| **Power** | Cell, supercap, FET, wide traces | Support ~120 mA edges without collapse |

```mermaid
flowchart LR
  subgraph hot["Tight"]
    MCU[MCU]
    RF[Radio]
    DEC[Decoupling]
  end
  subgraph quiet["Separated"]
    ADC[Moisture ADC]
  end
```

---

## I1 — Flash storage without MicroSD

### Tower autonomy without a filesystem

SD/FAT stacks are heavy and failure-prone for a simple append log. W25Q64 raw NOR with 16-byte records matches the fixed PDU and survives PC disconnect.

**Problem.** Tower must log while the PC is absent. FAT on MicroSD is expensive on ATmega328P (RAM, flash, power, failure modes).

**NOR constraint:** bits program **1 → 0** only; restoring 1s requires **erase**.

```mermaid
flowchart LR
  E[Erased state 1] -->|program| P[Programmed 0]
  P -->|sector erase| E
```

| Decision | Detail |
|:---------|:-------|
| Device | **W25Q64** SPI NOR |
| Record | **16 B** = u32 ID + 12 B PDU |
| Policy | Append; sector erase at boundaries; chip erase on full/clear |

$$
N_{\max} = \frac{8\times 10^{6}}{16} = \mathbf{524\,288}
$$

At 96 records/node/day, multi-year capacity exists for a small fleet if erase is not concentrated on one hot sector.

**Status:** production path in `receiver_storage.c`; Proteus may use stand-in — keep behaviours documented separately **(F4)**.

---

## I2 — Multi-node channel contention

### One demodulator, many buried transmitters

Synchronised WDT wakes collide. Stagger $T_i = 15\,\mathrm{min} + (id \bmod 30)\,s$ is implemented; CRC still drops residual overlaps as lost samples, not silent corruption.

One SX1278 demodulates **one** packet at a time. Synchronised WDT wakes produce systematic collisions.

**Temporal stagger (implemented in `transmitter.c`):**

$$
T_i = 15\,\mathrm{min} + (id \bmod 30)\ \mathrm{s}
$$

```mermaid
sequenceDiagram
  participant A as Node A
  participant B as Node B
  participant T as Tower
  A->>T: PDU
  B->>T: overlap
  T-->>T: CRC fail · drop
```

**Today:** stagger spreads wakes by up to 29 s; CRC-8 still drops residual overlaps as **lost samples**.
**Status (F2):** **Done** — `(DEVICE_ID % 30)` folded into WDT tick count. Production builds must clear `SIM_FAST_WAKE` (F1) or the interval is forced to ~8 s.

---

## I3 — Pre-flash manufacturing

### No 5 V ISP on a 3.6 V buried board

DIP-28 socket + lab programming avoids ICSP area and voltage abuse. Field boards receive a pre-flashed MCU; fuse recipes must be written down (tower crystal vs node RC).

**Problem.** In-circuit 5 V ISP on a 3.6 V buried board is a reliability and safety issue; ICSP also consumes scarce 50×50 mm area.

```mermaid
flowchart LR
  LAB[Lab ZIF + ISP + AVRDUDE] --> HEX[Flash image + fuses]
  HEX --> SOCK[Insert into DIP-28 socket]
  SOCK --> FIELD[Buried PCB · no ICSP header]
```

| Topic | Policy |
|:------|:-------|
| Node fuses | L `0xE2`, H `0xDA`, E `0xFD` |
| Field programming | Not supported on the buried assembly |
| Tower | Separate 16 MHz crystal fuse recipe — **still to formalise (H2)** |

---

## I4 — Confidentiality of the air interface

### Cleartext ISM is observable

IDs, moisture, tilt, and emergency flags can be received by third parties. Planned path: lightweight encryption with pre-shared keys at pre-flash; re-budget ToA if the frame grows (**F7** open).

**Problem.** Unencrypted PDUs on ISM spectrum are observable with suitable receivers (IDs, moisture, tilt, emergency flags).

```mermaid
flowchart LR
  S[Sense] --> P[Plaintext fields]
  P --> E[Encrypt on node · planned]
  E --> RF[LoRa]
  RF --> D[Decrypt on tower · planned]
  D --> ST[Flash + USB to Qt]
```

| Constraint | Implication |
|:-----------|:------------|
| Short ToA / small PDU | Prefer compact crypto; re-budget if length grows |
| 8 MHz AVR | Lightweight symmetric primitive, not TLS |
| No online CA | **Pre-shared keys** at pre-flash |
| Tower powered / reachable | Single decrypt point; USB stays local |
| Existing seq / status | Candidate **nonce / replay** binding |

**Status:** requirements stated; algorithm + frame + code **open (F7)**.

---

## I5 — Firmware supply-chain trust (security)

### Third-party libraries are an integrity risk

Opaque Arduino or vendor packs can change behaviour without review. Solution already in force: self-implemented drivers and protocol; only AVR-GCC/avr-libc headers.

### Challenge: third-party headers and opaque libraries

```mermaid
flowchart TB
  EXT[Third-party Arduino / vendor .h+.c packs] --> RISK[Unreviewed behaviour<br/>fork drift · compromised mirror · hidden side effects]
  RISK --> FAIL[Wrong SPI timing · silent data change · unexpected TX]
  OWN[In-tree drivers + protocol] --> REV[Team-readable register maps]
  REV --> TRUST[Auditable node and tower path]
  STD[AVR-GCC / avr-libc headers only] --> OWN
```

**Problem.** Embedded projects often pull LoRa, accelerometer, or flash “libraries.” Those headers and sources can be **edited or substituted** outside the team’s control. For a sealed buried node and a warning path, opaque middleware is a **security and integrity** risk, not only a style preference.

**Solution (as built).**

| Layer | Policy |
|:------|:-------|
| Application / drivers | **Self-implemented** in `Code/` from datasheets |
| Protocol | `slf_protocol.*` owned by the project |
| Allowed includes | Toolchain only: `avr/io.h`, `avr/interrupt.h`, `avr/sleep.h`, `avr/wdt.h`, `stdint.h`, `string.h`, `util/delay.h` |
| Rejected | Arduino core, third-party LoRa/ADXL/W25Q packs, FAT stacks |

| Module implemented in-tree | Role |
|:---------------------------|:-----|
| `transmitter_accelerometer.*` | ADXL345 SPI + low-power ACTIVITY |
| `transmitter_lora.*` / `receiver_radio.*` | SX1278 register TX/RX |
| `receiver_storage.*` | W25Q64 append / erase |
| `slf_protocol.*` | 12-byte frame + CRC-8 |

**Honest limit.** The compiler and avr-libc still must be genuine; this policy removes **application-level** third-party middleware, not formal binary attestation of the whole toolchain.

**Status:** **Done** for the current node/tower firmware path — [[Code Hub#Self-implemented firmware (supply-chain / trust policy)]]. Complements planned **air-frame encryption (F7)**; encryption protects the RF observer, self-built drivers protect the **build integrity** of the device software.

---

## J1 — Residual risk register

### What still undermines a Phase 1 claim if left open

| ID | Item | If deferred | Ties to |
|:---|:-----|:------------|:--------|
| **H3** | Sleep current unmeasured | Multi-year life is only a calculation | Energy slides **E** |
| **H4** | Soil path unmeasured | 1 m / 100–200 m range is only literature-scale | RF slides **G** |
| **F1** | `SIM_FAST_WAKE` left enabled | Field duty cycle is ~8 s, not 15 min + stagger | Dual wake **F**, collisions **I2** |
| **F3** | Moisture ADC polarity | Wet/dry inverted on host thresholds | Capacitive **D3c** |
| **F8** | Accel INT thresholds / current | Missed emergency wakes or excess µA | Dual wake **F** |
| **F2** | TX time jitter | **Done** (`id % 30` in `transmitter.c`); residual overlaps still CRC-drop | **I2** |
| **F7** | No air encryption | Cleartext IDs and samples on ISM | **I4** (planned) |
| **H1** | PCB not fabricated | All RF/power claims stay simulation-heavy | **H** layout |
| **H2** | Tower fuse recipe informal | Brick risk on first 16 MHz tower chip | Tower clock |
| **M1** | Capsule CAD incomplete | Helix air pocket / burial stack incomplete | Antenna **G** |

Authoritative live list: [[Status and open issues]]. This slide is the **honest risk view** after the design solutions above.

---

## J2 — Phase 1 validation sequence

### Close the prototype with measurements, not only slides

```mermaid
flowchart TB
  A[Clear SIM_FAST_WAKE · measure sleep µA]
  B[Moisture invert / two-point wet-dry]
  C[ADXL345 INT threshold + current on HW]
  D[TX stagger · Done F2]
  E[Optional: crypto freeze F7]
  F[PCB fab · ANT feed · EMI check]
  G[Soil-barrel / pit path-loss]
  H[E2E: node → flash → USB → Qt]
  A --> C
  B --> H
  C --> H
  F --> G
  G --> H
```

| Gate | Practical check | Why it still blocks Phase 1 claims |
|:-----|:----------------|:-----------------------------------|
| **F1** | Production hex without `SIM_FAST_WAKE` | Stagger and 15 min interval must be real |
| **H3** | µA meter, FET off, ADXL low-power | Life math needs a measured average class |
| **H4** | Barrel/pit at design depth | Soil loss coefficient not only a table value |
| **H6 / H1** | Nets review + fab POST | ADXL always-on; supercap TX-only; real boards |
| **F3 / F8 / H2** | Wet reading direction; INT wakes MCU; tower fuses | Correct sense path and tower clock |
| **E2E** | `'N'`/`'D'` into Qt, emergency `0x80` | Full chain without Proteus |

**Phase 1 “good enough” demo:** one node · one USB tower · SAFE/WARNING/CRITICAL · motion emergency path.

---

## J3 — Looking ahead (Phase 2 / 3 review only)

### Short review — not the body of this challenges deck

**Phase 1 (this deck):** buried node, USB tower, Qt, and the engineering solutions for energy, RF, protocol, dual wake, flash, and firmware trust.

| Phase | What it adds (summary) | Still assumes Phase 1 core |
|:------|:------------------------|:---------------------------|
| **2** | Solar + battery on tower · local **siren** on `0x80` · tower battery/uptime telemetry · **secondary** LoRa backhaul · **handheld** LCD mule · host **tower/node registry** | Same 12-byte node PDU, primary node RX, USB for service |
| **3** | Richer slope instruments (extensometer/TDR/IPI) · mesh · stronger analytics · production wear-leveling | Phase 2 field access working |

**Suggested Phase 2 start order (detail elsewhere):** siren + telemetry + registry → solar → secondary LoRa + handheld → encryption may trail.

Full step lists: [[Phase 2 and Phase 3 plan]] · design freeze: [[Data path future architecture]].

**Mentor line:** Phase 2/3 extend *operations and access*; they do not rewrite the Phase 1 analysis in sections A–I.

---

## J4 — Conclusions

1. **433 MHz LoRa (SX1278 / Ra-02)** is justified by soil attenuation and sensitivity at low rate—not by generic IoT fashion.
2. **Fixed 12-byte PDUs** bound energy, time-on-air, and collision window; zones, maps, and rich history stay on the host.
3. **Energy architecture** is Li-SOCl₂ + **1 F** buffer + P-MOS load switch (moisture + LoRa only) + long WDT + ADXL activity in sleep.
4. **RF closure** needs air-pocket helix, polarisation, tower height, and PCB EMI discipline—not only a module datasheet.
5. **Phase 1 residual risk** is **validation and fab** (sleep µA, soil loss, boards)—not an undefined architecture.
6. **Firmware supply-chain:** self-implemented drivers and protocol; toolchain headers only ([[Code Hub]]).
7. **Phase 2/3** appear here only as a **short forward look**; this challenges file stays centered on Phase 1 problems and solutions.

---

## J5 — Internal references

| Topic | Note |
|:------|:-----|
| Decisions, BOM, radio narrative | [[Project Overview]] |
| Frame, pins, WDT, pre-flash | [[Firmware Hub]] |
| Source tree, drivers, trust policy | [[Code Hub]] |
| Host thresholds, Places, registry model | [[Dashboard Hub]] |
| Open issue IDs (live) | [[Status and open issues]] |
| Phase 2 / 3 step list (outside this deck) | [[Phase 2 and Phase 3 plan]] |
| Multi-path / siren / solar / handheld design | [[Data path future architecture]] |
| Formal proposal text | [[Full Proposal]] |
| Mentor FAQ / IEC feasibility | [[Mentor readiness]] |

**External inputs used in analysis:** SX1278 / ADXL345 / W25Q64 datasheets; published sub-GHz soil attenuation orders of magnitude; Li-SOCl₂ manufacturer data (self-discharge, temperature).

---

## End

### SLF-EWN — Phase 1 challenges, analysis, and design responses

**Questions and discussion**
