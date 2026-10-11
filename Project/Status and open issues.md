---
tags:
  - slf-ewn
  - status
  - open-issues
aliases:
  - Status
  - Open issues
  - TODO
parent: "[[SLF-EWN Hub]]"
status: active
---

# Status and open issues

← [[SLF-EWN Hub]] · Decisions: [[Project Overview#What we decided]] · Sources: [[Code Hub]] · Mentor pack: [[Mentor readiness]] · Plan: [[Phase 2 and Phase 3 plan]]

Last reviewed: **2026-10-06**. When code lands a fix, tick the item here and update the matching hub — do not leave a second truth elsewhere.

---

## Overall phase

| Area | Status |
|:-----|:-------|
| Architecture / decisions | **Locked** — [[Project Overview#What we decided]] |
| AVR firmware (node + tower) | **Working** in Proteus; production path present in `Code/` |
| Protocol (12 B / 16 B / `0xAA`) | **Stable** — [[Firmware Hub#Data formats]] |
| Proteus TX / RX sheets | **Usable** end-to-end with COMPIM |
| EasyEDA TX / RX | Schematics done · layouts in review · **fab pending** |
| Qt dashboard | Functional (poll, map, Places, alerts) — polish ongoing |
| SolidWorks capsule | Specs set · **printables not delivered** |
| Blender film | Shot list set · **renders not delivered** |
| Field / soil-barrel test | **Not started** |
| IEC / mentor feasibility pack | **Documented** — [[Mentor readiness]] · [[Project Overview#IEC standards and feasibility]] |

---

## Open issues (firmware / protocol)

| ID | Issue | Severity | Notes |
|:---|:------|:---------|:------|
| F1 | **`SIM_FAST_WAKE` still `1`** | High before burial | Forces ~8 s wakes. `#undef` / remove for production — [[Firmware Hub#Watchdog]] |
| F2 | **TX time jitter** | Done | Implemented: `T_i = 15 min + (id % 30) s` → WDT ticks in `transmitter.c`. CRC still drops residual overlaps |
| F3 | **Moisture ADC not inverted** | Medium | Capacitive v1.2 / sim pot: wet → lower volts. Code maps linear; wet reads as **low %**. Invert or calibrate in firmware |
| F4 | **Flash path dual story** | Medium | Proteus uses 25AA1024 stand-in (no sector erase in that path). Production `receiver_storage.c` targets W25Q64 with sector/chip erase. Keep sim and field behaviours documented separately |
| F5 | **Proteus LoRa is FIFO `0x8D` only** | Low (sim) | Field `.hex` uses real SX1278 registers (`transmitter_lora.c` / `receiver_radio.c`). Do not ship the fake SPI contract to hardware |
| F6 | **Tower flash stops when full** | Low | Append-only; chip erase on full / clear. No ring buffer yet |
| F7 | **Air-frame encryption (planned)** | Medium | Encrypt on node, decrypt on tower. Pre-shared keys at pre-flash; lightweight crypto for 8 MHz AVR; final frame size TBD — [[Challenges and Solutions]] |
| F8 | **ADXL345 low-power INT on hardware** | Medium | Motion INT → INT0 while moisture/LoRa FET-off. Verify activity threshold and accel sleep current in that mode |

---

## Open issues (hardware / fab)

| ID | Issue | Severity | Notes |
|:---|:------|:---------|:------|
| H1 | **PCB fab not ordered** | High | 1-layer THT TX 50×50 mm, RX 95×70 mm — [[EasyEDA Hub]] |
| H2 | **Pre-flash fuse recipe for tower** | Medium | Node fuses documented (`0xE2` / `0xDA` / `0xFD`). Tower **16 MHz crystal** fuses are a separate recipe — write them down before first tower board |
| H3 | **Sleep current unmeasured** | High before burial | Target **&lt; 10 µA** with `SIM_FAST_WAKE` off. Needs µA meter on real hardware |
| H4 | **Through-soil RF unproven** | High | Design assumes 1 m soil + 100–200 m air. Needs soil-barrel / pit test |
| H5 | **Single-vendor parts risk** | Medium | Moisture v1.2, ZVP2106A, Ra-02 from local shelf — [[Project Overview#Shop]] |
| H6 | **Confirm EasyEDA: ADXL always-on; supercap TX-only** | Medium | Docs fixed 2026-10-06. Before fab verify schematic nets — [[EasyEDA Hub#Circuit decisions (TX vs RX)]] |

---

## Open issues (dashboard / mechanical)

| ID | Issue | Severity | Notes |
|:---|:------|:---------|:------|
| D1 | No separate Dashboard README | Low | Build notes live in [[Dashboard Hub]] only |
| M1 | SolidWorks deliverables unchecked | Medium | Capsule, tower box, STEP/STL, dimensioned drawing — [[Mechanical Hub]] |
| M2 | Blender shot list not rendered | Low | Film is demo/report only — dimensions must come from Mechanical Hub |

---

## Phase plan

Fine-grained roadmap: [[Phase 2 and Phase 3 plan]] (Phase 1 exit gate → Phase 2 workstreams A–G → Phase 3 outline).

---

## Explicitly out of scope (this build)

Planned later (architecture frozen): [[Data path future architecture]]

- Tower **secondary** LoRa backhaul (primary stays node RX)
- Tower **siren / alarm** + host SILENCE command
- Tower **solar + battery backup** (field power)
- Tower **self-telemetry** (battery/life/health) + host tower panel
- Host-side LoRa dongle / radio
- **Handheld** collector (LCD + LoRa + USB data mule to Qt)
- Field power for tower without permanent USB

Still out of scope unless reopened:

- Mesh hops between buried nodes
- ML false-alarm filtering
- Draw-wire extensometer / TDR / IPI (pads reserved only)
- Android phone as the primary field UI


## How to close an item

1. Change the **`.c` / schematic / CAD** first.  
2. Update the **canonical hub** ([[Firmware Hub]], [[Code Hub]], [[EasyEDA Hub]], …).  
3. Tick or delete the row on this page.  
4. Do not “fix” prose in [[Full Proposal]] without the code/CAD change.
