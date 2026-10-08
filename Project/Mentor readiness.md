---
tags:
  - slf-ewn
  - mentor
  - feasibility
  - standards
aliases:
  - Mentor FAQ
  - Feasibility pack
  - Compliance readiness
parent: "[[Project Overview]]"
status: active
---

# Mentor readiness — feasibility and compliance pack

← [[SLF-EWN Hub]] · Decisions: [[Project Overview]] · IEC detail: [[Project Overview#IEC standards and feasibility]] · Gaps: [[Status and open issues]] · Challenges: [[Challenges and Solutions]]

**Purpose.** One place for questions a mentor typically asks after “is this feasible?” — standards, radio rules, safety, cost, power, RF, ethics, and what is still unproven.  
**Last structured:** 2026-10-06.

---

## 1. Overall feasibility (one page)

| Question | Answer |
|:---------|:-------|
| Is the architecture feasible? | **Yes** — buried moisture + tilt, 433 MHz LoRa, primary cell, surface tower, Qt host is a known class of environmental IoT |
| Blocked by IEC? | **No** — see §2 and [[Project Overview#IEC standards and feasibility]] |
| Blocked by radio law? | **No in principle** — 433 MHz ISM/SRD is a standard student/industrial path; **local duty-cycle and power limits** must be respected (§3) |
| Can it be built with the team constraints? | **Yes** — 1-layer THT, shop parts, AVR-GCC, Qt; cost pair **&lt; $75** |
| Is multi-year life proven? | **Designed, not measured** — needs sleep-current and seal tests before burial claims |
| Is through-soil range proven? | **Designed from literature, not measured** — soil-barrel / pit test still required |
| Is this a certified life-safety system? | **No** — research / early-warning **prototype** and decision-support tool, not IEC 61508 SIL equipment |

**Quote for reviews**

> SLF-EWN is a feasible engineering prototype: the sensing, radio, power, and host approach align with established IEC equipment domains and common sub-GHz IoT practice. Critical performance claims (µA sleep, soil RF, multi-year seal) remain subject to measurement; the system is not claimed as certified functional-safety equipment.

---

## 2. Standards (IEC / ISO)

Full tables: [[Project Overview#IEC standards and feasibility]] · [[Full Proposal#7.3 IEC standards and feasibility]].

| Domain | Reference | Feasible? |
|:-------|:----------|:---------:|
| Ingress (IP) | IEC 60529 | Yes — design toward IP67/68-class capsule |
| Environment tests | IEC 60068 | Yes — qualification path exists |
| Impact | IEC 62262 | Yes — tower boxes |
| EMC | IEC 61000-6-x, 61000-4-x | Yes — design toward; lab later if required |
| ICT safety | IEC 62368-1 | Yes — tower / USB / host |
| Primary cell | IEC 60086 | Yes — Li-SOCl₂ class |
| Landslide EWS process | ISO 22327 / 22328 | Yes — process guidance, not a PCB ban |
| Functional safety SIL | IEC 61508 | **Not claimed** for this prototype |

**Honest limit:** referencing standards ≠ holding test certificates.

---

## 3. Radio spectrum and EMC

| Topic | Position |
|:------|:---------|
| Band | **433 MHz** licence-exempt / ISM-style short-range device (region-dependent) |
| Typical EU technical refs | **ETSI EN 300 220** (SRD), **ETSI EN 301 489** (radio EMC) |
| Design mitigations | Short 12-byte frames, low duty cycle (~15 min), modest TX power class, matched SF/BW |
| National duty | Confirm **local** power and duty-cycle limits before field TX (mentor may ask “did you check the regulator?”) |
| EMC of the *equipment* | IEC 61000 family; PCB layout (short RF, decoupling, ADC separation) is the practical response |

**Feasibility:** using 433 MHz LoRa for sparse telemetry is standard. Feasibility is **not** the same as “already type-approved.”

---

## 4. Electrical and battery safety

| Topic | Position |
|:------|:---------|
| Buried node voltage | **3.6 V** primary Li-SOCl₂ — low shock hazard |
| No 5 V ISP on buried board | Pre-flash in lab — avoids stressing a 3.6 V design with a 5 V programmer |
| Storage | 2-pin jumper isolates cell |
| Tower | USB 5 V → 3.3 V regulation; treat as ICT equipment (**IEC 62368-1** thinking) |
| Lithium transport | Shipping cells may need **UN 38.3** rules (logistics, not design feasibility) |
| Supercap | 5.5 V rated parallel buffer for PA peaks — ordinary practice |

---

## 5. Environmental and mechanical feasibility

| Topic | Position |
|:------|:---------|
| Burial ~1 m | Matches both sensing intent and soil RF segment |
| Seal | Design intent IP67/68-class; **must** be verified (potting, gaskets, cable exits) |
| Capsule | Cylinder preferred (soil load, auger install); SolidWorks deliverables still open |
| Tower | Raised vertical whip 1.5–2 m AGL; outdoor box IP65+ intent |
| Climate | Li-SOCl₂ and industrial parts chosen for wide temperature; still site-specific |

---

## 6. Power and lifetime feasibility

| Claim | Basis | Status |
|:------|:------|:------:|
| Cell energy ~8.64 Wh | 3.6 V × ~2.4 Ah | Design calc |
| 7-year average ceiling ~39 µA | Calendar arithmetic | Design calc |
| Target &lt; 10 µA average | Margin for cold/leakage | **Unmeasured** on HW |
| Duty cycle active share ~1.7 µA | 50 ms / 900 s at 30 mA | Design calc |
| PA peak | 1 F supercap + P-MOS load switch | Designed |
| `SIM_FAST_WAKE` | Must be off in production | Open **F1** |

**Mentor line:** lifetime is **feasible on paper**; burial claims wait on measured sleep current and seal integrity.

Detail: [[Challenges and Solutions]] sections E1–E5 · [[Status and open issues]] H3, F1.

---

## 7. Sensing and warning feasibility

| Topic | Position |
|:------|:---------|
| Moisture + tilt at depth | Established early indicators; **not** a full slope-stability FEM |
| Thresholds | Host defaults + per-node overrides; status `0x80` forces CRITICAL |
| ISO EWS role | Aligns with monitoring + warning service elements of **ISO 22327** |
| False alarms / missed events | Residual risk; ML filtering out of scope |
| After landslide | ID continuous; map pin may need survey update; no GNSS on node |

**Mentor line:** scientifically and operationally **plausible** as a prototype instrument network; not a substitute for geotechnical design of a slope.

---

## 8. Cost and schedule feasibility

| Item | Figure |
|:-----|:-----|
| Node BOM | ~$27 |
| Tower BOM | ~$15.50 |
| Pair | **&lt; $75** |
| Working timeline | ~12 days design→demo (aggressive; slips if fab/soil tests delay) |
| Team | A PCB/firmware · B mechanical · C dashboard · D film |

Cost is **feasible** for a student multi-node demo. Field campaigns and certification are extra budget.

---

## 9. Safety, ethics, and claims (important for mentors)

| Principle | Our position |
|:----------|:-------------|
| Role of the system | **Decision support / research prototype** — alerts operators; humans decide evacuation |
| Not claimed | Certified life-safety, SIL, or sole warning channel for a community |
| Data privacy | Air frames currently cleartext; **encryption planned** (F7) — IDs and moisture are sensitive in some sites |
| Responsible language | Prefer “early indicator” and “prototype EWS” over “guarantees lives saved” |
| Dual use | Slope monitoring for civil protection; same tech can serve mining safety |

If the mentor asks “can people rely on this tomorrow?”, answer: **not as a sole certified system** — as a **validated prototype after measurement**, yes for research and pilot support.

---

## 10. Technical risk register (short)

| Risk | Severity | Mitigation / status |
|:-----|:---------|:--------------------|
| Sleep current too high | High | Measure H3; clear F1 |
| Soil RF fails link | High | Barrel/pit test H4; antenna air pocket |
| Moisture polarity wrong | Medium | F3 invert/calibrate |
| Multi-node collisions | Medium | F2 jitter |
| Seal fails underground | High | Mechanical + IP-oriented potting |
| PCB never fabbed | High | H1 |
| Over-claiming safety | Medium | §9 language discipline |

Live list: [[Status and open issues]].

---

## 11. Academic / engineering contribution (if asked “what’s new?”)

Not “first landslide sensor in history.” Contributions of **this** build:

- Subsurface + through-soil **433 MHz** path with fixed **12-byte** protocol  
- **Pre-flash / no ICSP** on buried 3.6 V board  
- **P-MOS** gating + motion-INT dual wake architecture  
- Tower **NOR** log without FAT/MicroSD  
- End-to-end **Proteus fake-SPI** path sharing application hex  
- Qt host with **Places / per-node thresholds**  

Challenges narrative: [[Challenges and Solutions]].

---

## 12. Quick FAQ (mentor-style)

**Q. Is this IEC compliant?**  
A. It is **aligned and feasible** under the listed IEC families. It is **not** lab-certified in this phase.

**Q. Is 433 MHz legal here?**  
A. It is a standard SRD/ISM-class choice; we must still observe **local** power and duty-cycle rules before permanent field TX.

**Q. Will it last seven years?**  
A. **Designed** for that class of life; **prove** with µA measurement and seal validation first.

**Q. Will it work through one metre of soil?**  
A. **Literature-supported** design target; **prove** with soil-barrel / pit test.

**Q. Can this replace a geotechnical consultant?**  
A. **No.** It is an instrument and warning aid.

**Q. Why not Wi-Fi / 2.4 GHz / BLE?**  
A. Soil attenuation and energy budget — see challenges section C.

**Q. Why not GPS on every node?**  
A. Power, cost, and burial; ID + host map is the prototype split.

**Q. What if the mentor only wants one sentence?**  
A. *Feasible prototype under IEC equipment practice and common sub-GHz IoT methods; performance claims pending measurement; not a certified sole life-safety system.*

---

## 13. Document map for mentor sessions

| If they ask… | Open… |
|:-------------|:------|
| Decisions / BOM | [[Project Overview]] |
| Formal write-up | [[Full Proposal]] |
| IEC tables | [[Project Overview#IEC standards and feasibility]] |
| Open bugs / tests | [[Status and open issues]] |
| Deep technical challenges | [[Challenges and Solutions]] |
| Plain-language slide notes | [[Challenges Presentation Script]] |
| Protocol bytes | [[Firmware Hub]] |
| This pack | **This page** |
'''
