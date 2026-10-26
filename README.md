# SLF-EWN documentation

Obsidian-style vault for the **Subterranean Landslide and Flood Early-Warning Network**.

**Submission package:** INSA 2026 — 5th Round Summer Camp · Group 1 · Embedded Department. Start with [[Submission Checklist]].

## Start here

| Page | Purpose |
|:-----|:--------|
| **[[Submission Checklist]]** | Package contents, reading order, pre-submission checks, claims to avoid |
| **[[Full Proposal]]** | Formal technical report (submission draft) |
| **[[Challenges and Solutions]]** | Presentation: problems, math, fixes, open work |
| **[[SLF-EWN Hub]]** | Map of content, data flow, pin glance, team |
| **[[Project Overview]]** | Decisions table (wins on conflicts), BOM, radio, team |
| **[[Status and open issues]]** | Phase status and tracked gaps |
| **[[Code Hub]]** | As-built AVR sources — wins over older prose |
| **[[Firmware Hub]]** | 12-byte / 16-byte formats, status bits, WDT |

## Video

<a href="https://www.youtube.com/watch?v=RLF6Dt3RYG0">
  <img src="https://img.youtube.com/vi/RLF6Dt3RYG0/maxresdefault.jpg" alt="Watch the SLF-EWN project video" width="720">
</a>

▶ Click the preview to play the project walkthrough on YouTube.

## Rule of truth

1. **`Code/`** and **[[Project Overview#What we decided]]** win when numbers disagree.
2. `Archive/` is stale drafts — do not copy part numbers or byte layouts from there.
3. Change a pin or a byte only in [[Firmware Hub]] / the matching `.c`, then update the sim or PCB note.
4. Close gaps on [[Status and open issues]] when code/CAD lands — do not “fix” prose alone.

## Vault layout

```
artifacts/
├── SLF-EWN Hub.md · README.md
├── Code/ · Firmware/ · Proteus/ · EasyEDA/ · Dashboard/
├── Project/   (Overview, Full Proposal, Challenges, Status, Submission Checklist)
├── SOLIDWORKS/Mechanical Hub.md   (capsule + film)
├── Blender/   (stub → Mechanical Hub)
└── Archive/   (old drafts only)
```

**As-built stack:** ADXL345 + capacitive moisture → 12-byte LoRa frame (Ra-02, 433 MHz SF7) → tower W25Q64 (16-byte records) → Qt polls USB.

**Status in one line:** simulation-validated; hardware not fabricated; range, sleep current, and through-soil loss are still open. Do not describe the build as field-proven.
