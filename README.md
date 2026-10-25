# SLF-EWN documentation

Obsidian-style vault for the **Subterranean Landslide and Flood Early-Warning Network**.

## Start here

| Page | Purpose |
|:-----|:--------|
| **[[SLF-EWN Hub]]** | Map of content, data flow, pin glance, team |
| **[[Project Overview]]** | Decisions table (wins on conflicts), BOM, radio, team |
| **[[Status and open issues]]** | Phase status and tracked gaps |
| **[[Code Hub]]** | As-built AVR sources — wins over older prose |
| **[[Firmware Hub]]** | 12-byte / 16-byte formats, status bits, WDT |
| **[[Full Proposal]]** | Formal write-up |
| **[[Challenges and Solutions]]** | Presentation: problems, math, fixes, open work |

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
├── Project/   (Overview, Full Proposal, Challenges, Status)
├── SOLIDWORKS/Mechanical Hub.md   (capsule + film)
├── Blender/   (stub → Mechanical Hub)
└── Archive/   (old drafts only)
```

**As-built stack:** ADXL345 + capacitive moisture → 12-byte LoRa frame (Ra-02) → tower W25Q64 (16-byte records) → Qt polls USB @ 115200.
