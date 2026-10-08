Unlike the buried transmitter node, the tower receiver is not battery-constrained — it runs continuously from USB power, so there is no need to economize on clock speed or voltage headroom the way the transmitter does. The master ATmega328P runs on a **16 MHz external crystal** (rather than the transmitter's 8 MHz internal RC oscillator) for faster SPI and UART timing margin, and the whole board is powered directly from the **5 V USB rail** with no step-down regulation required, since none of the simulated parts (fake LoRa RX slave, 25AA1024, COMPIM) need a lower voltage to operate correctly in Proteus.

The fake LoRa RX module is simulated using a secondary ATmega328P chip acting as an SPI slave, so that the master tower ATmega328P behaves exactly as it would with the real RFM95W. The flash database and the USB-UART bridge, however, use **native Proteus parts** instead of custom emulated slaves: a **25AA1024 SPI EEPROM** stands in for the W25Q64 flash, and a **COMPIM** (bound to a real COM port) stands in for the FT232RL. This means the master firmware never knows it is talking to a simulation the same compiled firmware is used for both the Proteus test bench and the real PCB and it also lets the actual Qt 6 / QML dashboard talk to the simulation over a real serial link.

Because the tower must aggregate telemetry from **five or more buried nodes**, the "multiple receivers" behavior is emulated on the transmit side, not by adding multiple radios to the tower: several independent fake-transmitter child sheets (see `Proteus_transmitter.md`) are wired to the **same shared fake-LoRa antenna net**, each with a unique simulated Device ID and a randomized jitter delay before it "transmits." The single fake LoRa RX slave on the tower side simply decodes whichever packet arrives on that shared net, exactly like a real RFM95W can only demodulate one over-the-air packet at a time.

---

## Master MCU (Tower) to Fake LoRa RX Module Connection Table

| Master ATmega328P Pin Name | Label / Net Name | LoRa Module Pin          | Function / Description                                                                                           |
| :------------------------- | :--------------- | :----------------------- | :--------------------------------------------------------------------------------------------------------------- |
| `PC6/RESET`                | `RESET`          | _N/A_ (To 10 kΩ Pull-Up) | Tie through a `10 kΩ` resistor to 5V VCC to keep the Master from resetting.                                      |
| `VCC`                      | `VCC`            | _N/A_ (To 5V USB Rail)   | Connect directly to the 5V USB Power Terminal.                                                                   |
| `GND`                      | `GND`            | _N/A_ (To Ground)        | Connect directly to the global Ground Terminal.                                                                  |
| `PB2`                      | `RADIO_CS`       | NSS                      | Dedicated Active-Low Chip Select line driven by the Master to activate the fake radio (Digital Pin 8 equiv.).    |
| `PB3/MOSI`                 | `MOSI`           | MOSI                     | Shared SPI Master Out Slave In line — also wired to the fake flash chip.                                         |
| `PB4/MISO`                 | `MISO`           | MISO                     | Shared SPI Master In Slave Out line. Only the currently selected slave (radio or flash) may drive this net.      |
| `PB5/SCK`                  | `SCK`            | SCK                      | Shared SPI Serial Clock line, synchronizes bit shifts for both the fake radio and fake flash.                    |
| `PD2/INT0`                 | `DIO0_INT`       | DIO0                     | Handshake line. Fake radio drives this HIGH the instant a simulated over-the-air packet has been fully received. |
| `AVCC`                     | `AVCC`           | _N/A_ (To 5V USB Rail)   | Connect directly to the 5V USB Power Terminal.                                                                   |
| `GND`                      | `GND`            | _N/A_ (To Ground)        | Connect directly to the global Ground Terminal.                                                                  |

---

## Inside Fake LoRa RX Child Sheet Connection Table

| Internal Slave ATmega328P Pin Name | Connected Internal Net / Label | Outer Module Pin Name (On Parent Sheet) | Terminal Design Direction Type | Operational Purpose                                                                                                 |
| :--------------------------------- | :----------------------------- | :-------------------------------------- | :----------------------------- | :------------------------------------------------------------------------------------------------------------------ |
| `PB2/SS`                           | `NSS`                          | NSS                                     | INPUT                          | Slave Select. Master pulls this low to read out the buffered packet from the fake radio                             |
| `PB3/MOSI`                         | `MOSI`                         | MOSI                                    | INPUT                          | Slave Data Input. Receives register/command bytes from the master                                                   |
| `PB4/MISO`                         | `MISO`                         | MISO                                    | BIDIR                          | Slave Data Output. Releases the line (tri-states) whenever `NSS` = HIGH to avoid bus contention with the fake flash |
| `PB5/SCK`                          | `SCK`                          | SCK                                     | INPUT                          | SPI Clock Input driven by the master to synchronize data bit shifts                                                 |
| `PD3/INT1`                         | `DIO0`                         | DIO0                                    | OUTPUT                         | Handshake Output. Slave drives this high once it has assembled a complete simulated packet in its RX FIFO           |
| `PD0/RXD`                          | _Direct Wire_                  | _N/A_ (From shared ANT/virtual bus net) | _Internal Trace Only_          | Listens on the shared "antenna" net for incoming ASCII/byte packets from any of the fake transmitter child sheets   |
| _No Pin (GND Rail)_                | `ANT`                          | ANT                                     | INPUT                          | Tied to ground through `R1 (10 kΩ)` pull-down to prevent float warnings when no transmitter is active               |

---

## Master MCU (Tower) to 25AA1024 SPI EEPROM (W25Q64 Stand-In) Connection Table

Proteus does not carry a native W25Q64 model, so the tower's flash database is simulated with a **Microchip 25AA1024** (128 KB / 1 Mbit SPI EEPROM), which Proteus does model natively.

|Master ATmega328P Pin Name|Label / Net Name|25AA1024 Pin|Function / Description|
|:--|:--|:--|:--|
|`PB1`|`FLASH_CS`|`CS` (Pin 1)|Dedicated Active-Low Chip Select for the flash log database (Digital Pin 9 equiv.)[cite: 1].|
|`PB3/MOSI`|`MOSI`|`SI` (Pin 5)|Shared SPI line. Sends opcode + 24-bit address + data bytes for page-program and read commands[cite: 1].|
|`PB4/MISO`|`MISO`|`SO` (Pin 2)|Shared SPI line. 25AA1024 returns stored bytes back to the master during a read/dump command[cite: 1].|
|`PB5/SCK`|`SCK`|`SCK` (Pin 6)|Shared SPI Clock line[cite: 1].|
|`VCC`|`VCC`|`VCC` (Pin 8)|Connect directly to the 5V USB Power Terminal[cite: 1].|
|`GND`|`GND`|`VSS` (Pin 4)|Connect directly to the global Ground Terminal[cite: 1].|
|_N/A (tie high)_|`WP_N`|`WP` (Pin 3)|Through a `10 kΩ` pull-up to `VCC`. Tied high to disable hardware write-protection; floating this pin risks blocking writes[cite: 1].|
|_N/A (tie high)_|`HOLD_N`|`HOLD` (Pin 7)|Through a `10 kΩ` pull-up to `VCC`. Tied high to keep the device out of hold state; floating this pin can freeze the SPI transfer mid-byte[cite: 1].|

### 25AA1024 vs. W25Q64 — Driver-Level Differences to Account For

|Aspect|W25Q64 (real, on final PCB)|25AA1024 (Proteus simulation)|
|:--|:--|:--|
|Capacity|8 MB|128 KB — plenty for a simulation run, but the circular-buffer wraparound point in firmware must use a compile-time size constant rather than a hardcoded W25Q64 address range[cite: 1].|
|Erase before write|Requires `0x20` sector-erase before `0x02` page-program|**Not required** — EEPROM cells can be rewritten directly, so the erase opcode is simply skipped or ignored in simulation[cite: 1].|
|Write-enable|`0x06` WREN before any write op|Same `0x06` WREN before any write op — unchanged[cite: 1].|
|Page size|256 B|256 B — matches, no driver change needed[cite: 1].|
|Status/busy polling|`0x05` RDSR, bit 0 = BUSY|`0x05` RDSR, bit 0 = WIP — same polling logic works unmodified[cite: 1].|

The firmware's flash driver guards the sector-erase call behind a `#ifdef SIM_EEPROM` compile flag: the real-hardware build still erases before every page-program as W25Q64 requires, while the Proteus build skips straight to page-program, since the 25AA1024 does not gate writes on a prior erase and there is no BUSY flag to wait on for a command that was never issued[cite: 1].

---

## Master MCU (Tower) to COMPIM (FT232RL USB-UART Stand-In) Connection Table

Proteus does not model the FT232RL chip itself, so the USB-UART link is simulated with a **COMPIM** (COM Port Interface Model) part, bound to a real or virtual COM port on the host PC. This lets the **actual Qt 6 / QML dashboard application** connect to that COM port and talk to the Proteus simulation exactly as it would talk to a real tower over USB a true end-to-end test.

|Master ATmega328P Pin Name|Label / Net Name|COMPIM Pin|Function / Description|
|:--|:--|:--|:--|
|`PD1/TXD`|`UART_TX`|`RXD`|Forwards every decoded packet (with the appended timestamp) out to the bound COM port at 115200 baud, 8N1[cite: 1].|
|`PD0/RXD`|`UART_RX`|`TXD`|Receives the `0x44` ("Dump") command byte sent by the dashboard over the bound COM port to trigger a flash history dump[cite: 1].|
|_N/A_|`DTR_LINE`|`DTR`|Wired so the dashboard's serial library sees DTR asserted on connect, matching the "DTR and RTS asserted" requirement noted for the real FT232RL link[cite: 1]. If left unconnected, some serial libraries will stall waiting for a handshake that never arrives[cite: 1].|
|_N/A_|`RTS_LINE`|`RTS`|Wired the same way as `DTR`, for libraries that gate transmission on RTS rather than DTR[cite: 1].|
|`VCC`|`VCC`|_N/A_|COMPIM does not require a simulated supply pin — it is a virtual port model, not a powered part[cite: 1].|

### Proteus ↔ Host PC Bridging Setup

1. Install a loopback COM port pair utility on the host PC, creating two linked virtual ports.
2. In Proteus, set the COMPIM part's Physical Port property to one side of the pair.
3. Point the Qt 6 / QML dashboard's serial connection at the other side of the pair using the same 115200 baud / 8N1 settings the real FT232RL link uses.
4. Run the Proteus simulation and the dashboard simultaneously decoded packets streamed from the master over `UART_TX` should now appear live in the dashboard's node cards, and a dashboard-issued dump request should trigger the master's flash-read routine exactly as it would over a physical USB cable.

---

## Direct USB(Batter) 5V Power Section

Because the tower is USB(full power battery) powered and not power-constrained, the master, fake LoRa RX slave, and 25AA1024 are all wired directly to the 5V rail supplied by the COMPIM's simulated USB or a batter source source, with bulk capacitors for noise stability. If a real 3.3V-only part (such as an actual RFM95W breakout) is later swapped into the simulation in place of the fake LoRa RX slave, reintroduce an `LM1117-3.3` (or similar) regulator ahead of that part only — the master, 25AA1024, and COMPIM link can remain on 5V regardless

---

### 💡 Simulation Mechanics (How Multi-Node Reception Is Emulated)

Since Proteus cannot model true RF propagation or true radio contention, the following conventions are used to faithfully emulate the tower's real-world behavior

- Each fake transmitter child sheet (from `Proteus_transmitter.md`) drives the shared `ANT` net only when its own internal randomized jitter delay expires, mimicking the real 15-minute-plus-jitter schedule[cite: 1].
- The fake LoRa RX slave treats the **first character it sees rise on the shared net** as the start of a new packet and ignores further edges until that packet's terminator is received, approximating the "one demodulation at a time" limitation of a single physical RFM95W[cite: 1].
- If two fake transmitters assert the shared net inside the same simulation tick, the fake LoRa RX slave discards the corrupted frame (via a simulated CRC-8 mismatch) exactly as the real CRC-8/SMBUS check would reject a collided over-the-air frame[cite: 1].
- Successfully decoded packets are annotated with a running 32-bit tick counter inside the fake LoRa RX slave before being handed to the master, standing in for the timestamp the master would normally attach after reading the RFM95W FIFO[cite: 1].

---

## 📦 Missing Support Components & Wiring Table

|Component Type|Proteus Library Keyword|Value / Rating|Wiring Connections (Point-to-Point)|Engineering Purpose|
|:--|:--|:--|:--|:--|
|Pull-Up Resistor|`RES`|`10 kΩ`|Connect one leg to `PC6/RESET`.<br>Connect the other leg to Global `VCC` [5V](cite: 1).|Prevents Floating: keeps the Master from resetting on ambient noise[cite: 1].|
|Crystal Oscillator|`CRYSTAL`|`16 MHz`|Connect across `XTAL1` (Pin 9) and `XTAL2` (Pin 10) of the Master ATmega328P[cite: 1].|Provides the external clock reference — used instead of the transmitter's internal 8 MHz RC oscillator, since the tower isn't power-constrained and benefits from the tighter timing tolerance[cite: 1].|
|Load Capacitor|`CAP`|`22 pF`|Connect one leg to `XTAL1`.<br>Connect the other leg to Global `GND`[cite: 1].|Standard crystal load capacitor, paired with a matching one on `XTAL2`[cite: 1].|
|Load Capacitor|`CAP`|`22 pF`|Connect one leg to `XTAL2`.<br>Connect the other leg to Global `GND`[cite: 1].|Standard crystal load capacitor, paired with a matching one on `XTAL1`[cite: 1].|
|Pull-Up Resistor|`RES`|`100 kΩ`|Connect one leg to `RADIO_CS`.<br>Connect the other leg to Global `VCC` [5V](cite: 1).|Prevents Floating on the radio chip-select line during boot[cite: 1].|
|Pull-Up Resistor|`RES`|`100 kΩ`|Connect one leg to `FLASH_CS`.<br>Connect the other leg to Global `VCC` [5V](cite: 1).|Prevents Floating on the flash chip-select line during boot[cite: 1].|
|Pull-Up Resistor|`RES`|`10 kΩ`|Connect one leg to 25AA1024 `WP` (Pin 3).<br>Connect the other leg to Global `VCC` [5V](cite: 1).|Disables hardware write-protect on the flash EEPROM; floating this pin risks blocking writes[cite: 1].|
|Pull-Up Resistor|`RES`|`10 kΩ`|Connect one leg to 25AA1024 `HOLD` (Pin 7).<br>Connect the other leg to Global `VCC` [5V](cite: 1).|Keeps the flash EEPROM out of hold state; floating this pin can freeze an SPI transfer mid-byte[cite: 1].|
|Pull-Down Resistor|`RES`|`10 kΩ`|Connect one leg to shared `ANT` net.<br>Connect the other leg to `GND`[cite: 1].|Prevents float warnings on the shared antenna bus when no transmitter is currently active[cite: 1].|
|Decoupling Capacitor|`CAP`|`100 nF` _(0.1 µF)_|Connect one leg to `VCC`/`AVCC`.<br>Connect the other leg to Global `GND`[cite: 1].|Noise Suppression at both the master and each SPI slave to keep the 16 MHz clock lines clean[cite: 1].|
|Input Bulk Capacitor|`CAP-ELEC`|`10 µF`|Connect (+) to `USB_5V` (incoming from COMPIM's simulated USB source).<br>Connect (-) to `GND`[cite: 1].|Stabilizes the 5V rail against USB current fluctuations before it feeds the master and simulated peripherals[cite: 1].|
|Output Bulk Capacitor|`CAP-ELEC`|`10 µF`|Connect (+) to `VCC` (5V rail, near the master).<br>Connect (-) to `GND`[cite: 1].|Local decoupling for the master and SPI slaves, standing in for what would sit at the regulator output on real hardware[cite: 1].|
|Output Ceramic Capacitor|`CAP`|`100 nF`|Connect one leg to `AREF`.<br>Connect the other leg to `GND`[cite: 1].|Standard ADC reference noise suppression, matching the transmitter node's practice even though the tower does not use analog sensing[cite: 1].|

---

## Complete Master ATmega328P (Tower) Connection Reference Table

|MCU Pin Name|Connected Net Name|Connected To Component & Pin|Wire Direction / Type|Engineering Function|
|:--|:--|:--|:--|:--|
|`PC6/RESET`|`RESET`|`10 kΩ` Resistor ➔ VCC|Input (Pull-Up)|Tie to power to prevent ambient static noise from causing random resets[cite: 1].|
|`XTAL1` (Pin 9)|`XTAL1`|16 MHz Crystal Leg 1<br>`22 pF` Load Capacitor ➔ GND|Clock Input/Output|External clock reference, used instead of the transmitter's internal 8 MHz RC oscillator since the tower has no power budget to conserve[cite: 1].|
|`XTAL2` (Pin 10)|`XTAL2`|16 MHz Crystal Leg 2<br>`22 pF` Load Capacitor ➔ GND|Clock Input/Output|Second crystal leg, paired with `XTAL1`[cite: 1].|
|`PD2/INT0`|`DIO0_INT`|Fake LoRa RX Module: `DIO0` Pin|Input (Digital)|Packet-ready interrupt. Fires whenever the fake radio finishes assembling a decoded frame[cite: 1].|
|`PB1`|`FLASH_CS`|25AA1024: `CS` Pin|Output (Digital)|Active-Low SPI Chip Select dedicated to the flash log database[cite: 1].|
|`PB2`|`RADIO_CS`|Fake LoRa RX Module: `NSS` Pin|Output (Digital)|Active-Low SPI Chip Select dedicated to the radio receiver[cite: 1].|
|`PB3/MOSI`|`MOSI`|Fake LoRa RX: `MOSI` Pin<br>25AA1024: `SI` Pin|Output (Shared SPI)|Master Out Slave In line. Shared communication path to both simulated peripherals[cite: 1].|
|`PB4/MISO`|`MISO`|Fake LoRa RX: `MISO` Pin<br>25AA1024: `SO` Pin|Input (Shared SPI)|Master In Slave Out line. Only the currently-selected slave drives this net at any given time[cite: 1].|
|`PB5/SCK`|`SCK`|Fake LoRa RX: `SCK` Pin<br>25AA1024: `SCK` Pin|Output (Shared SPI)|SPI Serial Clock line generated by the master to synchronize both simulated peripherals[cite: 1].|
|`PD1/TXD`|`UART_TX`|COMPIM `RXD`|Output|Streams every decoded, timestamped packet out to the bound COM port at 115200 baud, readable by the real dashboard[cite: 1].|
|`PD0/RXD`|`UART_RX`|COMPIM `TXD`|Input|Receives the `0x44` dump-request command byte sent by the dashboard over the bound COM port[cite: 1].|
|`VCC`|`VCC`|5V USB Rail<br>`10 µF` Bulk Capacitor|Power Input|Main constant power source — the tower is never sleep- or power-gated, unlike the buried node[cite: 1].|
|`GND`|`GND`|Global Ground Terminal|Power Input|Common ground reference point for the circuit[cite: 1].|
|`AVCC`|`VCC`|5V USB Rail|Power Input|Dedicated power supply input line for the internal Analog-to-Digital Converter block (unused on the tower but tied per datasheet)[cite: 1].|

---

# Design Decisions

- **No Power Gating on the Tower:** Unlike the buried transmitter node, the tower receiver runs in `RX_CONTINUOUS` mode 24/7 and is mains/USB/battery-powered when connected, or battery-buffered without deep sleep when standalone, so no P-MOSFET power-gating circuit is required here.
- **Shared SPI Bus Contention:** Because the fake LoRa RX slave and the 25AA1024 share `MOSI`/`MISO`/`SCK`, the LoRa slave firmware must release (tri-state) `MISO` whenever its chip-select line is HIGH. The 25AA1024 already handles this correctly as a native Proteus SPI slave, so the only remaining risk is on the fake-radio side.
- **No Step-Down Regulation Needed:** Unlike the buried transmitter, the tower has no reason to conserve power, so the simulation skips the 3.3V regulation stage entirely and runs the master, fake LoRa RX slave, and 25AA1024 directly off the 5V USB rail. A regulator would only need to be reintroduced if a genuinely 3.3V-only part (e.g. a real RFM95W footprint) were substituted in later.
- **16 MHz Clock Instead of 8 MHz:** The transmitter runs its internal RC oscillator at 8 MHz specifically to save power during its brief active windows between long sleeps. The tower has no such constraint — it is always awake — so the master runs on a 16 MHz external crystal, giving more SPI/UART timing margin and matching the clock speed typically used on non-battery-constrained AVR designs.
- **25AA1024 in Place of the W25Q64:** Proteus has no native W25Q64 model, so a Microchip 25AA1024 SPI EEPROM stands in for it. The trade-offs are a much smaller address space (128 KB vs. 8 MB — irrelevant for a short simulation run) and the fact that EEPROM cells don't require a sector-erase before rewriting; the firmware's sector-erase call is compiled out under a `SIM_EEPROM` flag for the Proteus build only, leaving the real-hardware W25Q64 driver untouched.
- **COMPIM** Using a COMPIM part bound to a host COM port lets the actual Qt 6 / QML dashboard connect to the simulation over a real serial link, exercising the full protocol stack framing, CRC alignment, and the `0x44` dump command .`DTR`/`RTS` must be wired on the COMPIM part, since some serial libraries wait indefinitely for that handshake before they will read incoming bytes.
- **Timestamp Injection Point:** Timestamping is performed inside the fake LoRa RX slave (rather than by the master, as in the real design) purely to keep the simulation's tick counter synchronized with packet arrival order across multiple simulated transmitters; the real hardware firmware still timestamps in the master's main loop exactly as described in the project proposal.