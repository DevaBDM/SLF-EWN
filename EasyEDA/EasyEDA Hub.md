---
tags:
  - slf-ewn
  - easyeda
  - pcb
  - hub
aliases:
  - PCB
  - EasyEDA
parent: "[[SLF-EWN Hub]]"
status: active
---

# EasyEDA

**1-layer, 100% THT.** Socketed ATmega328P. Modules on 2.54 mm **female** headers — do not solder Ra-02. Crossed traces: jumper or top-side solder, not a second layer.

← [[SLF-EWN Hub]] · Firmware: [[Firmware Hub]] · Code: [[Code Hub]] · Sim: [[Transmitter Simulation]] · [[Receiver Simulation]] · Tube: [[Mechanical Hub]]

Boards: [[Transmitter PCB]] · [[Receiver PCB]]

![[system-block-diagram.png]]

**Link (intent):** LoRa 433 MHz · **1 m** through soil · **200–500 m** after breakout.

---

## Circuit decisions (TX vs RX) — final

| Item | Buried TX | Tower RX |
|:-----|:----------|:---------|
| **47 mF / 5.5 V supercap** | **Yes** — with **1.2 kΩ** series R from cell | **No** |
| **1.2 kΩ series R** | **Yes** — limits inrush to 3.0 mA | **No** |
| **τ = 56.4 s** | RC time constant | **No** |
| **5τ = 282 s (4.7 min)** | Full recharge window | **No** |
| **ΔV = 127 mV** | Voltage dip @ 120 mA × 50 ms | **No** |
| **I_max = 3.0 mA** | Inrush-limited to Li-SOCl₂ safe | **No** |
| **ZVP2106A / VCC_SW** | **Yes** — **moisture + Ra-02 only** | **No** |
| **ADXL345 power** | **Always-on 3.6 V** (INT in sleep) | Not fitted |
| **Primary cell** | Li-SOCl₂ AA | No (USB) |
| **Clock** | 8 MHz RC | 16 MHz crystal + 2×22 pF |
| **Storage** | None | W25Q64 |
| **USB / FT232 / LM1117** | No | Yes |

---

## Transmitter — definitive connections

### Power tree

```
Li-SOCl₂ (+) ── 2-pin jumper ── 1.2 kΩ ──┬── 47 mF supercap (+)
                                         ├── MCU VCC, AVCC
                                         ├── ADXL345 VS + VDD_IO
                                         ├── 100 kΩ CS pull-ups, RESET pull-up
                                         └── ZVP2106A SOURCE
Li-SOCl₂ (−) ──────────────────────────── GND (entire board)

ZVP2106A DRAIN = VCC_SW
  ├── Moisture probe VCC
  ├── Ra-02 3.3V pin
  ├── 0.1 µF + 10–100 µF at Ra-02 VCC
  └── 0.1 µF at moisture VCC

ZVP2106A GATE ← PD4, with 100 kΩ to always-on 3.6 V
  HIGH (default) = rail OFF · LOW = rail ON
```

### MCU pin map (must match `transmitter.c`)

| ATmega328P | Net | Destination |
|:-----------|:----|:------------|
| PB5 (SCK) | SCK | Ra-02 SCK, ADXL345 SCLK |
| PB3 (MOSI) | MOSI | Ra-02 MOSI, ADXL345 SDI |
| PB4 (MISO) | MISO | Ra-02 MISO, ADXL345 SDO |
| PB1 | LORA_NSS | Ra-02 NSS · 100 kΩ to 3.6 V |
| PB2 | ACCEL_CS | ADXL345 CS · 100 kΩ to 3.6 V |
| PD4 | GATE | ZVP2106A gate · 100 kΩ to 3.6 V |
| PD2 (INT0) | MOTION | ADXL345 **INT1** (activity) |
| PD3 (INT1) | DIO0 | Ra-02 DIO0 (TxDone) |
| PC1 (ADC1) | SOIL | Moisture SIG |
| PD1 (TXD) | DEBUG | optional UART 9600 |
| PC6 (RESET) | RESET | 10 kΩ to 3.6 V · no ICSP header |
| VCC / AVCC | 3.6 V | always-on · 0.1 µF each to GND |
| GND | GND | common |

### Module wiring (TX)

| Module | Must connect | Do not |
|:-------|:-------------|:-------|
| **ADXL345** | VS+VDD_IO → **always-on 3.6 V**; INT1 → PD2; SPI; CS → PB2 | Do **not** power from VCC_SW |
| **Ra-02** | 3.3V → **VCC_SW**; NSS PB1; DIO0 PD3; SPI; ANT→SMA; RESET 10 kΩ to VCC_SW | — |
| **Moisture** | VCC → **VCC_SW**; SIG → PC1; GND | — |
| **Supercap 47 mF** | + on always-on rail after **1.2 kΩ**; − to GND | Not on RX |
| **Series R 1.2 kΩ** | Between cell/jumper and always-on rail | Limits inrush to 3 mA |
| **SMA** | Center → Ra-02 ANT; shield → GND | Long skinny ANT track |

### Passives (TX)

100 kΩ: GATE, LORA_NSS, ACCEL_CS · 10 kΩ: MCU RESET · **1.2 kΩ series** cell→rail · **47 mF / 5.5 V** on always-on rail · 0.1 µF at MCU VCC/AVCC, ADXL, Ra-02, moisture · 10–100 µF at Ra-02 on VCC_SW · optional Schottky on cell.

**Not on TX:** crystal, USB, FT232, LM1117, W25Q64.

---

## Receiver — definitive connections

### Power tree

```
USB 5 V (FT232) ── 10 µF ── LM1117-3.3 IN
LM1117 OUT = 3.3 V ── 10 µF + 0.1 µF ── MCU, Ra-02, W25Q64, pull-ups
GND common
```

### MCU pin map (must match `receiver.c` / `receiver_radio.c` / `receiver_storage.c`)

| ATmega328P | Net | Destination |
|:-----------|:----|:------------|
| PB5 / PB3 / PB4 | SPI | Ra-02 **and** W25Q64 |
| PB2 | RADIO_CS | Ra-02 NSS · 100 kΩ → 3.3 V |
| PB1 | FLASH_CS | W25Q64 /CS · 100 kΩ → 3.3 V |
| PD2 (INT0) | DIO0 | Ra-02 DIO0 (RxDone) |
| PD1 (TXD) | UART | FT232 RXD |
| PD0 (RXD) | UART | FT232 TXD |
| PB6 / PB7 | XTAL | 16 MHz + 22 pF each → GND |
| PC6 (RESET) | RESET | 10 kΩ → 3.3 V |
| VCC / AVCC | 3.3 V | 0.1 µF each |
| GND | GND | common |

### Module wiring (RX)

| Module | Must connect | Do not |
|:-------|:-------------|:-------|
| **Ra-02** | 3.3 V; NSS PB2; DIO0 PD2; SPI; ANT→SMA | No FET, no supercap |
| **W25Q64** | VCC 3.3 V; /CS PB1; SPI; **/WP and /HOLD → 10 kΩ → 3.3 V** | No MicroSD |
| **FT232RL** | USB 5 V; TXD/RXD cross to MCU | — |
| **LM1117-3.3** | IN 5 V, OUT 3.3 V, caps | — |

### Passives (RX)

16 MHz + 2×22 pF · 10 kΩ RESET · 100 kΩ both CS · 10 kΩ WP/HOLD · regulator 10 µF in/out + 0.1 µF · 0.1 µF MCU · 0.1 µF + 10–47 µF Ra-02 · 0.1 µF flash · optional LEDs, TVS on SMA, USB polyfuse.

**Not on RX:** supercap, ZVP2106A, ADXL345, moisture, Li-SOCl₂ as primary supply.

---

## Layout rules (both)

### Units: mil (use these in EasyEDA)

**1 mil = 0.001 inch ≈ 0.0254 mm.** EasyEDA trace width and clearance dialogs commonly use **mil**. Set design rules / track properties to the values below when drawing TX and RX boards.

| Class | Width | Where |
|:------|:------|:------|
| **Power** | **30–40 mil** | Cell/jumper → 1.2 kΩ → always-on rail; FET drain → Ra-02 VCC; supercap leads |
| **Signal** | **~10 mil** | SPI, CS, INT, ADC, UART |
| **ANT** | Short, as wide as practical to SMA centre | Minimise inductance; keep away from noisy SPI if possible |

Also:

- 0.1 µF within **5 mm** of each IC VCC  
- Moisture ADC away from RF/digital (TX)  
- 1-layer crossings = jumper only  
- DIP-28 socket; female headers for modules  

Full netlists: sections above.

---

## Assembly order

**TX:** socket + passives + FET → headers/SMA/moisture/jumper → pre-flashed MCU → cell + supercap → Ra-02 + ADXL + probe → POST → remove jumper for storage.

**RX:** socket + crystal + LM1117 + passives → headers/SMA/FT232/W25Q64 → MCU → USB. **No supercap, no FET.**

**Status:** schematics done · layouts in review · before fab verify nets match tables above (**H6**).

---

## Search keywords (EasyEDA)

| Want | Try |
|:-----|:----|
| MCU | `ATMEGA328P-PU` |
| Radio | `Ra-02` |
| Headers | `HDR-F-2.54` |
| SMA | `SMA-TH` / `SMA-JW-P` |
| FET | `ZVP2106A` |
| Flash | `W25Q64` |
| USB | `FT232RL Module` |
| 3.3 V | `LM1117-3.3` |
