> **Archived draft.** Early Proteus wiring notes; the current setup is in [[Proteus Hub]] and [[Transmitter Simulation]].

All chips run at 8 MHz from a 3.6 V battery and use THT parts instead of SMD.

The fake LoRa and fake accelerometer subcircuits stand in for the real devices. Each is an ATmega328P that answers the same SPI registers, so the master firmware is identical whether it talks to the fake or the real part. The master has no “simulation” branch.

## Master MCU to fake LoRa Module Connection Table

| Master ATmega328P Pin Name | Label / Net Name | LoRa Module Pin        | Function / Description                                                                                      |
| :------------------------- | :--------------- | :--------------------- | :---------------------------------------------------------------------------------------------------------- |
| `PC6/RESET`                | `RESET`          | _N/A_ (To 10k Pull-Up) | Tie through a `10 kΩ` resistor to 3.6V VCC to keep the Master from resetting.                      |
| `VCC`                      | `VCC`            | _N/A_ (To 3.6V Rail)   | Connect directly to the global 3.6V Power Terminal.                                                |
| `GND`                      | `GND`            | _N/A_ (To Ground)      | Connect directly to the global Ground Terminal.                                                    |
| `PB1`                      | `NSS`            | NSS                    | Dedicated Active-Low Chip Select line driven by the Master to activate the radio.                  |
| `PB3/MOSI`                 | `MOSI`           | MOSI                   | SPI Master Out Slave In line. Sends commands and payloads to the radio.                            |
| `PB4/MISO`                 | `MISO`           | MISO                   | SPI Master In Slave Out line. _(The inner child-sheet terminal is set to Output style.)_    |
| `PB5/SCK`                  | `SCK`            | SCK                    | SPI Serial Clock line. Synchronizes data bit shifts between the chips.                             |
| `PD3/INT1`                 | `INT1`           | DIO0                   | Handshake line. The LoRa module drives this HIGH to signal that a packet transmission is complete. |
| `AVCC`                     | `AVCC`           | _N/A_ (To 3.6V Rail)   | Connect directly to the global 3.6V Power Terminal.                                                |
| `GND`                      | `GND`            | _N/A_ (To Ground)      | Connect directly to the global Ground Terminal.                                                    |

---

## Inside Fake LoRa Child Sheet Connection Table

| Internal Slave ATmega328P Pin Name | Connected Internal Net / Label | Outer Module Pin Name (On Parent Sheet) | Terminal Design Direction Type | Operational Purpose                                                                        |
| :--------------------------------- | :----------------------------- | :-------------------------------------- | :----------------------------- | :----------------------------------------------------------------------------------------- |
| `PB2/SS`                           | `NSS`                          | NSS                                     | INPUT                          | Slave Select. Master pulls this low to target the fake radio.                     |
| `PB3/MOSI`                         | `MOSI`                         | MOSI                                    | INPUT                          | Slave Data Input. Receives config bytes and payloads from the master.             |
| `PB4/MISO`                         | `MISO`                         | MISO                                    | BIDIR                          | Slave Data Output. Sends responses back to master.                                         |
| `PB5/SCK`                          | `SCK`                          | SCK                                     | INPUT                          | SPI Clock Input driven by the master to synchronize data bit shifts.              |
| `PD3/INT1`                         | `DIO0`                         | DIO0                                    | OUTPUT                         | Handshake Output. Slave drives this high to signal packet tx completion.          |
| `PD1/TXD`                          | _Direct Wire_                  | _N/A_ (To Virtual Terminal RXD)         | _Internal Trace Only_          | Serial line that pushes decoded text packets onto the visual terminal window.    |
| _No Pin (GND Rail)_                | `ANT`                          | ANT                                     | INPUT                          | Tied to ground through `R2 (10 kΩ)` pull-down resistor to prevent float warnings. |

---

## Fake Accelerometer Inside Connection Table

| Internal ATmega328P (U3) Pin Name | Connected Component / Net | Outer Module Pin Name    | Terminal Type & Direction | Simulation Role                                                                                |
| :-------------------------------- | :------------------------ | :----------------------- | :------------------------ | :--------------------------------------------------------------------------------------------- |
| `PB2/SS`                          | Direct trace              | CS                       | INPUT (➔ Into Chip)       | Receives the Master's Chip Select trigger.                                            |
| `PB3/MOSI`                        | Direct trace              | SDA                      | INPUT (➔ Into Chip)       | Receives register address commands from the Master.                                   |
| `PB4/MISO`                        | Direct trace              | SD0                      | BIDIR                     | Pushes X, Y, or Z data bytes back to the Master                                                |
| `PB5/SCK`                         | Direct trace              | SCL                      | INPUT (➔ Into Chip)       | Synchronizes the data transfer clock timing.                                          |
| `PD2/INT0`                        | Direct trace              | INT1                     | OUTPUT (Chip ➔)           | Drives a logic pulse to wake the Master from sleep.                                   |
| `PC0/ADC0`                        | `RV1` Center Wiper Pin    | _N/A (Internal)_         | _Internal Net_            | Reads variable analog voltage for the X-Axis.                                         |
| `PC1/ADC1`                        | `RV2` Center Wiper Pin    | _N/A (Internal)_         | _Internal Net_            | Reads variable analog voltage for the Y-Axis.                                         |
| `PC2/ADC2`                        | `RV3` Center Wiper Pin    | _N/A (Internal)_         | _Internal Net_            | Reads variable analog voltage for the Z-Axis.                                         |
| `VCC`                             | Global Inner VCC          | VCC                      | POWER INPUT               | Connected to `VCC_SW` for power gating simulation.                                    |
| `AVCC`                            | `VCC`                     | Global 3.6V Battery Rail | Power Input               | Dedicated power supply input line for the internal Analog-to-Digital Converter block. |
| `GND`                             | Global Inner Ground       | GND                      | POWER INPUT               | Common reference ground terminal.                                                     |

---

## Master MCU to Accelerometer Connection Table

| Master ATmega328P Pin Name | Label / Net Name    | Outside Accelerometer Block Pin | Terminal Direction          | Operational Description                                                                        |
| :------------------------- | :------------------ | :------------------------------ | :-------------------------- | :--------------------------------------------------------------------------------------------- |
| `PB2`                      | `CS_ACCEL`          | CS                              | Master Output ➔ Slave Input | Dedicated Chip Select. Master pulls this low to read G-force values.                  |
| `PB3/MOSI`                 | `MOSI`              | SDA                             | Master Output ➔ Slave Input | Shared SPI Master Out line. Sends register addresses to the sensor.                   |
| `PB4/MISO`                 | `MISO`              | SD0                             | Slave Output ➔ Master Input | Shared SPI Master In line. Sensor pushes X, Y, Z data back over this link.            |
| `PB5/SCK`                  | `SCK`               | SCL                             | Master Output ➔ Slave Input | Shared SPI Clock line. Synchronizes the communication timing.                         |
| `PD2/INT0`                 | `INT0`              | INT1                            | Slave Output ➔ Master Input | Dedicated wake-up interrupt. Sensor pulls this low/high during a sudden motion spike. |
| Global VCC                 | Global inner VCC    | VCC                             | POWER INPUT                 | Switched 3.6V supply line.                                                            |
| Global GND                 | Global inner Ground | GND                             | POWER INPUT                 | Common reference ground for the circuit logic.                                        |

---

## Soil Moisture Sensor (POT-HG) Connection Table

| Component & Pin Name  | Connected Net Label | Master ATmega328P Pin Name | Terminal/Connection Type | Operational Description                                                                              |
| :-------------------- | :------------------ | :------------------------- | :----------------------- | :--------------------------------------------------------------------------------------------------- |
| `POT-HG` Top Leg      | `VCC_SW`            | _N/A (Via MOSFET Drain)_   | Power Input (Switched)   | Powered only when the Master drives the P-MOSFET gate low. Saves energy during sleep.       |
| `POT-HG` Center Wiper | `SOIL_RAW`          | `PC1/ADC1`                 | Analog Input             | Sends a variable 0–3.6V voltage level to the Master's internal Analog-to-Digital Converter. |
| `POT-HG` Bottom Leg   | `GND`               | `GND`                      | Ground Circuit           | Common system reference ground path.                                                        |

---

### Reading wet vs. dry soil in the simulation

When coding the master firmware or running the simulation, the voltage from the potentiometer acts in reverse to real-world moisture due to how capacitive probes function:

- **Dry Soil:** Slide the `POT-HG` slider all the way up to 100% [High Voltage, ~3.6V]. Air has low capacitance, resulting in a higher voltage output.
- **Wet Soil:** Slide the `POT-HG` slider down to 0% [Low Voltage, ~1.0V to 1.5V]. Water has high capacitance, which drops the analog voltage output on the signal line.

---

## Power-Gating MOSFET (Q1) Connection Table

| MOSFET Pin Name / Location | Connected Wire / Net Label | Destination Device & Pin Name             | Operational & Electrical Purpose                                                                                                               |
| :------------------------- | :------------------------- | :---------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------- |
| Gate (Top Pin)             | `PD4`                      | Master ATmega328P: `PD4`                  | Control Line: Master drives this pin LOW (0V) to turn on the power rail, and HIGH (3.6V) to shut it off completely during deep sleep. |
| Source (Left Pin)          | `VCC`                      | Global 3.6V Battery Rail                  | Power Input: Connects directly to the constant, unswitched battery source.                                                           |
| Drain (Right Pin)          | `VCC_SW`                   | Soil Moisture Potentiometer Top Fixed Leg | Switched Power Output: This net provides 3.6V only when the master is awake and actively reading sensors.                             |

---

## Missing Support Components & Wiring Table

| Component Type                    | Proteus Library Keyword | Value / Rating               | Wiring Connections (Point-to-Point)                                                                            | Engineering Purpose                                                                                                                                                                                     |
| :-------------------------------- | :---------------------- | :--------------------------- | :------------------------------------------------------------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `PD4`.<br>Connect the other leg to Global `VCC` [3.6V].                            | Prevents Floating: Forces the MOSFET to stay completely OFF while the ATmega328P is booting up or in a deep reset state.                                                                       |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `NSS`.<br>Connect the other leg to Global `VCC` [3.6V].                            | Prevents Floating.                                                                                                                                                                             |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `CS_ACCEL`.<br>Connect the other leg to Global `VCC` [3.6V].                       | Prevents Floating.                                                                                                                                                                             |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `INT0`.<br>Connect the other leg to Global `VCC` [3.6V].                           | Prevents Floating.                                                                                                                                                                             |
| Energy Reservoir (Supercapacitor) | `CAP-ELEC`              | `0.47 F`<br>_(Low ESR type)_ | Connect the positive (+) leg to Global `VCC` (3.6V).<br>Connect the negative (-) leg to Global `GND`. | Voltage Buffer: Supplies the high pulse current (120mA) needed for LoRa bursts without dropping the battery voltage.                                                                           |
| Decoupling Capacitor              | `CAP`                   | `100 nF` _(0.1 µF)_          | Connect one leg to the switched `VCC_SW` (Drain) net.<br>Connect the other leg to Global `GND`.       | Noise Suppression: Filters out high-frequency switching noise when the MOSFET turns on the sensor rail.                                                                                        |
| Decoupling Capacitor              | `CAP`                   | `100 nF`                     | Connect one leg to `AREF`.<br>Connect the other leg to Global `GND`.                                  | Noise Suppression: The ADC is configured to use `AVCC` as the reference, and it is standard practice to connect a 100 nF capacitor to suppress high-frequency noise. |

---

### Schematic Placement Guide

1. **The Pull-Up Resistor (`R_GATE`):** Place it vertically directly above the line connecting Master `PD4` to the top Gate pin of the MOSFET.
2. **The Supercapacitor (`C_BULK`):** Place this large polarized capacitor right at the battery input terminal, _before_ the MOSFET source pin. This ensures it stays constantly charged by the battery and is always ready to back up the radio.
3. **The Decoupling Capacitor (`C_BYPASS`):** Place this small ceramic capacitor directly between the `VCC_SW` node and Ground. Position it as close as possible to the fake accelerometer's power input pin.

---

## Complete Master ATmega328P Connection Reference Table

| MCU Pin Name | Connected Net Name | Connected To Component & Pin                                | Wire Direction / Type | Engineering Function                                                                                   |
| :----------- | :----------------- | :---------------------------------------------------------- | :-------------------- | :----------------------------------------------------------------------------------------------------- |
| `PC6/RESET`  |                    | `10 kΩ` Resistor ➔ VCC                                      | Input (Pull-Up)       | Tie to power to prevent ambient static noise from causing random resets.                      |
| `PD2/INT0`   | `INT0`             | Accelerometer Module: `INT1` Pin                            | Input (Digital)       | Hardware wake-up trigger line. A sudden shock pulls this pin to wake the MCU from deep sleep. |
| `PD3/INT1`   | `INT1`             | Fake LoRa Module: `DIO0` Pin                                | Input (Digital)       | Radio handshake line. The fake module pulls this high when a packet is sent.                  |
| `PD4`        | `PD4`              | `ZVP2106A` MOSFET (Q1): Gate Pin<br>`100 kΩ` Resistor ➔ VCC | Output (Digital)      | Power Gating: Drives `LOW (0V)` to turn on the sensor rail; holds `HIGH (3.6V)` in sleep.     |
| `VCC`        | `VCC`              | Global 3.6V Battery Rail<br>`0.47 F` Supercapacitor (+)     | Power Input           | Main constant power source. Connects to the (+) side of the supercapacitor buffer.           |
| `GND`        | `GND`              | Global Ground Terminal<br>`0.47 F` Supercapacitor (-)       | Power Input           | Common ground reference point for the circuit.                                                |
| `PB1`        | `NSS`              | Fake LoRa Module: `NSS` Pin                                 | Output (Digital)      | Active-Low SPI Chip Select dedicated to the radio module.                                     |
| `PB2`        | `CS_ACCEL`         | Accelerometer Module: `CS` Pin                              | Output (Digital)      | Active-Low SPI Chip Select dedicated to the motion tracker.                                   |
| `PB3/MOSI`   | `MOSI`             | Accelerometer: `SDA` Pin<br>Fake LoRa Module: `MOSI` Pin    | Output (Shared SPI)   | Master Out Slave In line. Shared communication path to send data out to sensors.              |
| `PB4/MISO`   | `MISO`             | Accelerometer: `SD0` Pin<br>Fake LoRa Module: `MISO` Pin    | Input (Shared SPI)    | Master In Slave Out line. Shared communication path to read bytes back into the MCU.          |
| `PB5/SCK`    | `SCK`              | Accelerometer: `SCL` Pin<br>Fake LoRa Module: `SCK` Pin     | Output (Shared SPI)   | SPI Serial Clock line. Generated by the master to synchronize data bit transfers.             |
| `AVCC`       | `VCC`              | Global 3.6V Battery Rail                                    | Power Input           | Dedicated power supply input line for the internal Analog-to-Digital Converter block.         |
| `GND`        | `GND`              | Global Ground Terminal                                      | Power Input           | Analog section ground reference point.                                                        |
| `PC1/ADC1`   | `SOIL_RAW`         | `POT-HG` Potentiometer: Middle Wiper                        | Input (Analog)        | Reads 0V–3.6V analog signals from the capacitive soil moisture probe simulator.              |
| `PD1/TXD`    | --                 | Virtual terminal `RXD`                                      | Output                | Display for debugging purposes.                                                               |

---

# Design Decisions

- **Supercapacitor (`C_BULK`):** A **47 mF THT supercapacitor** (1 F for simulation) at **5.5V** works perfectly to handle the ~120 mA LoRa transmission spikes. Its voltage rating must be **≥5.0 V or 5.5 V**. Avoid the common 3.6V rated memory-backup supercapacitors, as running them right at their maximum operating ceiling (3.6V battery) drastically reduces their operational lifespan.

**RC Network Design:** The 47 mF capacitor is paired with a **1.2 kΩ series resistor** to limit inrush current to **3.0 mA** ($I_{\text{max}} = 3.6\,\text{V}/1.2\,\text{k}\Omega$). The time constant is $\tau = 56.4\,\text{s}$, giving a full recharge time of **4.7 minutes** ($5\tau$) within our 15-minute wake cycle. During a 50 ms, 120 mA burst, the voltage dip is **ΔV = 127 mV** ($\Delta V = 0.006\,\text{C}/0.047\,\text{F}$), keeping the rail at ~3.40 V — well above the 2.7 V BOD threshold.
- **Decoupling Capacitors:** The 100 nF capacitors are ceramic disc or multilayer ceramic (MLCC) leaded radial types, placed as physically close to Pins `VCC`/`GND` and `AVCC`/`GND` of the microcontrollers as possible to keep the 8 MHz clock lines clean.
- **The brown-out detection (BOD) trap:** the 3.6 V cell drains over time, so the ATmega328P extended fuse bits are set for this design. The Arduino factory setting assumes a 5 V rail with BOD at 4.3 V or 2.7 V; once the cell falls below that, the chip locks up. The fuses are flashed with **BOD at 1.8 V**, or BOD disabled, to use the full cell capacity.
- Since the accelerometer and LoRa share MISO, the inactive slave cannot continuously drive MISO. The fake LoRa firmware must configure its MISO output appropriately so that it releases the line when NSS = HIGH. Otherwise the bus has contention. Same issue for the fake accelerometer.
