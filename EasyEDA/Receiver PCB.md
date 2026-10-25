---
tags:
  - slf-ewn
  - easyeda
  - receiver
  - pcb
parent: "[[EasyEDA Hub]]"
---

# Receiver PCB

← [[EasyEDA Hub]] · **Full netlist:** [[EasyEDA Hub#Receiver — definitive connections]] · Sim: [[Receiver Simulation]] · Firmware: [[Firmware Hub]]

| Spec | Value |
|:-----|:------|
| Size | **95 × 70 mm**, 1-layer THT |
| MCU | ATmega328P-PU socket, **16 MHz** crystal |
| Power | USB 5 V (FT232RL) → LM1117-3.3 |
| Log | W25Q64 8 MB SPI NOR |

Older drafts used MicroSD or two Li-SOCl₂ cells. **USB 5 V** is the build; optional battery is extra. Storage is **W25Q64**, not a card.

## What does **not** go on the receiver

| Part | Why absent |
|:-----|:-----------|
| **47 mF + 1.2 kΩ (TX RC)** | USB can source radio/MCU current; RC buffer is for **buried primary-cell TX peaks only** |
| **ZVP2106A / VCC_SW** | Tower stays powered for continuous RX; no multi-year µA sleep requirement |
| **ADXL345 / moisture** | Sensing is on the node |
| **Li-SOCl₂ as sole supply** | Prototype tower is USB-hosted |

**Schematic** · **PCB** · **3D**

![[Schematic_landslideReciever_2026-08-26.png]]
*Figure R1 — receiver schematic export, Rev 1.0. Pre-fab; the image still shows the dual-cell / supercap / EEPROM revision (H7).*

![[PCB_PCB_landslideReciever_2026-08-26.png]]
*Figure R2 — receiver 95 × 70 mm single-layer layout, Rev 1.0.*

![[Attachments/Receiver 3D PCB.png]]
*Figure R3 — receiver 3D top view, Rev 1.0.*

![[Attachments/Receiver 3D PCB back.png]]
*Figure R4 — receiver 3D bottom view, Rev 1.0.*

> [!warning] Images are not as-built (H7)
> The exported revision shows two Li-SOCl₂ holders, a supercap, and an EEPROM footprint. The build text and `Code/` specify **USB-only power** and a **W25Q64** log. Re-export both boards after the schematic is reconciled — see **H7** on [[Status and open issues]].

## Parts on this board

| Function  | Part               | EasyEDA          | Mount                 |
| :-------- | :----------------- | :--------------- | :-------------------- |
| MCU       | ATmega328P-PU      | `ATMEGA328P-PU`  | DIP-28 socket         |
| Radio     | **Ra-02**          | `Ra-02`          | Female 2.54 mm        |
| Flash     | W25Q64             | `W25Q64`         | DIP-8 or SOIC adapter |
| USB       | FT232RL module     | `FT232RL Module` | Headers               |
| Regulator | LM1117-3.3 TO-220  | `LM1117-3.3`     | THT                   |
| Antenna   | SMA + outdoor whip | `SMA-TH`         | Edge                  |
| Clock     | 16 MHz + 2× 22 pF  | —                | THT                   |

**3.3 V rail:** MCU pin 7, Ra-02 VCC, W25Q64 VCC. **GND:** MCU 8 & 22, radio, flash, FTDI, SMA legs.

Must match `receiver.c`:

| MCU pin | Net | Goes to |
|:--------|:----|:--------|
| 19 / 17 / 18 | SCK / MOSI / MISO | Ra-02 and W25Q64 |
| 16 PB2 | `RADIO_CS` | Ra-02 NSS |
| 15 PB1 | `FLASH_CS` | W25Q64 CS |
| 4 PD2 | INT0 | Ra-02 DIO0 |
| 3 PD1 TXD | UART | FTDI RX |
| 2 PD0 RXD | UART | FTDI TX |
| 9 / 10 | XTAL | 16 MHz + 22 pF |
| 1 RESET | — | 10 kΩ to 3.3 V |

UART 115200 host setting; `receiver.c` currently generates 111.1 kbaud (H9). Host protocol: [[Firmware Hub#Data formats]].

## Recommended passives (RX)

| Part | Value | Where |
|:-----|:------|:------|
| Crystal | **16 MHz** + **2× 22 pF** | XTAL1 / XTAL2 |
| RESET pull-up | **10 kΩ** | RESET → 3.3 V |
| CS pull-ups | **100 kΩ** | RADIO_CS, FLASH_CS → 3.3 V |
| Regulator input | **10 µF** | USB 5 V near LM1117 in |
| Regulator output | **10 µF + 0.1 µF** | 3.3 V near LM1117 out |
| MCU decouple | **0.1 µF** | VCC and AVCC within 5 mm |
| Radio decouple | **0.1 µF + 10–47 µF** | at Ra-02 VCC (RX current quieter than TX PA, still good practice) |
| Flash decouple | **0.1 µF** | at W25Q64 VCC |

**Optional (nice for outdoor tower)**

| Part | Why |
|:-----|:----|
| Power LED + 1 kΩ on 3.3 V | Visual “alive” |
| Activity LED on DIO0 or software GPIO | Packet indication |
| Low-capacitance TVS at SMA | Antenna ESD / surge assist (**IEC 61000-4-5** thinking) |
| Polyfuse on USB 5 V | Cable fault protection |
| Extra bulk 100 µF on 5 V | Long USB cable sag |

**Behaviour**

| Mode | What happens |
|:-----|:-------------|
| PC unplugged | DIO0 → CRC-ok frame → 16-byte append (`record_id` + 12 B) |
| PC plugged | Same; dashboard **polls** `'S'`/`'D'`/`'N'` |
| Host `'D'` / `'N'` | `0xAA` header + records (not a raw 12-byte stream) |

Do **not** put a FAT filesystem on the AVR. W25Q64 is a raw page log: [[Firmware Hub#Surface tower (`receiver.c`)]].

**~$15.50.** Antenna **1.5–2 m** up: [[Project Overview#Radio]].
