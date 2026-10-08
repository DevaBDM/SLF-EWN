All chips work at 8 MHz with a 3.6 V battery, and use THT instead of SMD components[cite: 1].

the fake lora and fake accelerometers are to simulate the real lora and accelerometers, they will behave as they where the real device, so the master mcu doesn't know anything about those fakes, the firmware will be same whom every it connects. they both use atmega328p to simulate the real one

## Master MCU to fake LoRa Module Connection Table

| Master ATmega328P Pin Name | Label / Net Name | LoRa Module Pin        | Function / Description                                                                                      |
| :------------------------- | :--------------- | :--------------------- | :---------------------------------------------------------------------------------------------------------- |
| `PC6/RESET`                | `RESET`          | _N/A_ (To 10k Pull-Up) | Tie through a `10 kΩ` resistor to 3.6V VCC to keep the Master from resetting[cite: 1].                      |
| `VCC`                      | `VCC`            | _N/A_ (To 3.6V Rail)   | Connect directly to the global 3.6V Power Terminal[cite: 1].                                                |
| `GND`                      | `GND`            | _N/A_ (To Ground)      | Connect directly to the global Ground Terminal[cite: 1].                                                    |
| `PB1`                      | `NSS`            | NSS                    | Dedicated Active-Low Chip Select line driven by the Master to activate the radio[cite: 1].                  |
| `PB3/MOSI`                 | `MOSI`           | MOSI                   | SPI Master Out Slave In line. Sends commands and payloads to the radio[cite: 1].                            |
| `PB4/MISO`                 | `MISO`           | MISO                   | SPI Master In Slave Out line. _(Ensure the inner child sheet terminal is set to Output style)_[cite: 1].    |
| `PB5/SCK`                  | `SCK`            | SCK                    | SPI Serial Clock line. Synchronizes data bit shifts between the chips[cite: 1].                             |
| `PD3/INT1`                 | `INT1`           | DIO0                   | Handshake line. The LoRa module drives this HIGH to signal that a packet transmission is complete[cite: 1]. |
| `AVCC`                     | `AVCC`           | _N/A_ (To 3.6V Rail)   | Connect directly to the global 3.6V Power Terminal[cite: 1].                                                |
| `GND`                      | `GND`            | _N/A_ (To Ground)      | Connect directly to the global Ground Terminal[cite: 1].                                                    |

---

## Inside Fake LoRa Child Sheet Connection Table

| Internal Slave ATmega328P Pin Name | Connected Internal Net / Label | Outer Module Pin Name (On Parent Sheet) | Terminal Design Direction Type | Operational Purpose                                                                        |
| :--------------------------------- | :----------------------------- | :-------------------------------------- | :----------------------------- | :----------------------------------------------------------------------------------------- |
| `PB2/SS`                           | `NSS`                          | NSS                                     | INPUT                          | Slave Select. Master pulls this low to target the fake radio[cite: 1].                     |
| `PB3/MOSI`                         | `MOSI`                         | MOSI                                    | INPUT                          | Slave Data Input. Receives config bytes and payloads from the master[cite: 1].             |
| `PB4/MISO`                         | `MISO`                         | MISO                                    | BIDIR                          | Slave Data Output. Sends responses back to master.                                         |
| `PB5/SCK`                          | `SCK`                          | SCK                                     | INPUT                          | SPI Clock Input driven by the master to synchronize data bit shifts[cite: 1].              |
| `PD3/INT1`                         | `DIO0`                         | DIO0                                    | OUTPUT                         | Handshake Output. Slave drives this high to signal packet tx completion[cite: 1].          |
| `PD1/TXD`                          | _Direct Wire_                  | _N/A_ (To Virtual Terminal RXD)         | _Internal Trace Only_          | Serial line that pushes decoded text packets onto your visual terminal window[cite: 1].    |
| _No Pin (GND Rail)_                | `ANT`                          | ANT                                     | INPUT                          | Tied to ground through `R2 (10 kΩ)` pull-down resistor to prevent float warnings[cite: 1]. |

---

## Fake Accelerometer Inside Connection Table

| Internal ATmega328P (U3) Pin Name | Connected Component / Net | Outer Module Pin Name    | Terminal Type & Direction | Simulation Role                                                                                |
| :-------------------------------- | :------------------------ | :----------------------- | :------------------------ | :--------------------------------------------------------------------------------------------- |
| `PB2/SS`                          | Direct trace              | CS                       | INPUT (➔ Into Chip)       | Receives the Master's Chip Select trigger[cite: 1].                                            |
| `PB3/MOSI`                        | Direct trace              | SDA                      | INPUT (➔ Into Chip)       | Receives register address commands from the Master[cite: 1].                                   |
| `PB4/MISO`                        | Direct trace              | SD0                      | BIDIR                     | Pushes X, Y, or Z data bytes back to the Master                                                |
| `PB5/SCK`                         | Direct trace              | SCL                      | INPUT (➔ Into Chip)       | Synchronizes the data transfer clock timing[cite: 1].                                          |
| `PD2/INT0`                        | Direct trace              | INT1                     | OUTPUT (Chip ➔)           | Drives a logic pulse to wake the Master from sleep[cite: 1].                                   |
| `PC0/ADC0`                        | `RV1` Center Wiper Pin    | _N/A (Internal)_         | _Internal Net_            | Reads variable analog voltage for the X-Axis[cite: 1].                                         |
| `PC1/ADC1`                        | `RV2` Center Wiper Pin    | _N/A (Internal)_         | _Internal Net_            | Reads variable analog voltage for the Y-Axis[cite: 1].                                         |
| `PC2/ADC2`                        | `RV3` Center Wiper Pin    | _N/A (Internal)_         | _Internal Net_            | Reads variable analog voltage for the Z-Axis[cite: 1].                                         |
| `VCC`                             | Global Inner VCC          | VCC                      | POWER INPUT               | Connected to `VCC_SW` for power gating simulation[cite: 1].                                    |
| `AVCC`                            | `VCC`                     | Global 3.6V Battery Rail | Power Input               | Dedicated power supply input line for the internal Analog-to-Digital Converter block[cite: 1]. |
| `GND`                             | Global Inner Ground       | GND                      | POWER INPUT               | Common reference ground terminal[cite: 1].                                                     |

---

## Master MCU to Accelerometer Connection Table

| Master ATmega328P Pin Name | Label / Net Name    | Outside Accelerometer Block Pin | Terminal Direction          | Operational Description                                                                        |
| :------------------------- | :------------------ | :------------------------------ | :-------------------------- | :--------------------------------------------------------------------------------------------- |
| `PB2`                      | `CS_ACCEL`          | CS                              | Master Output ➔ Slave Input | Dedicated Chip Select. Master pulls this low to read G-force values[cite: 1].                  |
| `PB3/MOSI`                 | `MOSI`              | SDA                             | Master Output ➔ Slave Input | Shared SPI Master Out line. Sends register addresses to the sensor[cite: 1].                   |
| `PB4/MISO`                 | `MISO`              | SD0                             | Slave Output ➔ Master Input | Shared SPI Master In line. Sensor pushes X, Y, Z data back over this link[cite: 1].            |
| `PB5/SCK`                  | `SCK`               | SCL                             | Master Output ➔ Slave Input | Shared SPI Clock line. Synchronizes the communication timing[cite: 1].                         |
| `PD2/INT0`                 | `INT0`              | INT1                            | Slave Output ➔ Master Input | Dedicated wake-up interrupt. Sensor pulls this low/high during a sudden motion spike[cite: 1]. |
| Global VCC                 | Global inner VCC    | VCC                             | POWER INPUT                 | Switched 3.6V supply line[cite: 1].                                                            |
| Global GND                 | Global inner Ground | GND                             | POWER INPUT                 | Common reference ground for the circuit logic[cite: 1].                                        |

---

## Soil Moisture Sensor (POT-HG) Connection Table

| Component & Pin Name  | Connected Net Label | Master ATmega328P Pin Name | Terminal/Connection Type | Operational Description                                                                              |
| :-------------------- | :------------------ | :------------------------- | :----------------------- | :--------------------------------------------------------------------------------------------------- |
| `POT-HG` Top Leg      | `VCC_SW`            | _N/A (Via MOSFET Drain)_   | Power Input (Switched)   | Powered only when the Master drives the P-MOSFET gate low. Saves energy during sleep[cite: 1].       |
| `POT-HG` Center Wiper | `SOIL_RAW`          | `PC1/ADC1`                 | Analog Input             | Sends a variable 0–3.6V voltage level to the Master's internal Analog-to-Digital Converter[cite: 1]. |
| `POT-HG` Bottom Leg   | `GND`               | `GND`                      | Ground Circuit           | Common system reference ground path[cite: 1].                                                        |

---

### 💡 Simulation Mechanics (How to Read Wet vs. Dry Soil)

When coding your master firmware or running the simulation, the voltage from the potentiometer acts in reverse to real-world moisture due to how capacitive probes function[cite: 1]:

- **Dry Soil:** Slide the `POT-HG` slider all the way up to 100% [High Voltage, ~3.6V](cite: 1). Air has low capacitance, resulting in a higher voltage output[cite: 1].
- **Wet Soil:** Slide the `POT-HG` slider down to 0% [Low Voltage, ~1.0V to 1.5V](cite: 1). Water has high capacitance, which drops the analog voltage output on the signal line[cite: 1].

---

## Power-Gating MOSFET (Q1) Connection Table

| MOSFET Pin Name / Location | Connected Wire / Net Label | Destination Device & Pin Name             | Operational & Electrical Purpose                                                                                                               |
| :------------------------- | :------------------------- | :---------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------- |
| Gate (Top Pin)             | `PD4`                      | Master ATmega328P: `PD4`                  | Control Line: Master drives this pin LOW (0V) to turn on the power rail, and HIGH (3.6V) to shut it off completely during deep sleep[cite: 1]. |
| Source (Left Pin)          | `VCC`                      | Global 3.6V Battery Rail                  | Power Input: Connects directly to your constant, unswitched battery source[cite: 1].                                                           |
| Drain (Right Pin)          | `VCC_SW`                   | Soil Moisture Potentiometer Top Fixed Leg | Switched Power Output: This net provides 3.6V only when the master is awake and actively reading sensors[cite: 1].                             |

---

## 📦 Missing Support Components & Wiring Table

| Component Type                    | Proteus Library Keyword | Value / Rating               | Wiring Connections (Point-to-Point)                                                                            | Engineering Purpose                                                                                                                                                                                     |
| :-------------------------------- | :---------------------- | :--------------------------- | :------------------------------------------------------------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `PD4`.<br>Connect the other leg to Global `VCC` [3.6V](cite: 1).                            | Prevents Floating: Forces the MOSFET to stay completely OFF while the ATmega328P is booting up or in a deep reset state[cite: 1].                                                                       |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `NSS`.<br>Connect the other leg to Global `VCC` [3.6V](cite: 1).                            | Prevents Floating[cite: 1].                                                                                                                                                                             |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `CS_ACCEL`.<br>Connect the other leg to Global `VCC` [3.6V](cite: 1).                       | Prevents Floating[cite: 1].                                                                                                                                                                             |
| Pull-Up Resistor                  | `RES`                   | `100 kΩ`                     | Connect one leg to `INT0`.<br>Connect the other leg to Global `VCC` [3.6V](cite: 1).                           | Prevents Floating[cite: 1].                                                                                                                                                                             |
| Energy Reservoir (Supercapacitor) | `CAP-ELEC`              | `0.47 F`<br>_(Low ESR type)_ | Connect the positive (+) leg to Global `VCC` (3.6V).<br>Connect the negative (-) leg to Global `GND`[cite: 1]. | Voltage Buffer: Supplies the high pulse current (120mA) needed for LoRa bursts without dropping the battery voltage[cite: 1].                                                                           |
| Decoupling Capacitor              | `CAP`                   | `100 nF` _(0.1 µF)_          | Connect one leg to the switched `VCC_SW` (Drain) net.<br>Connect the other leg to Global `GND`[cite: 1].       | Noise Suppression: Filters out high-frequency switching noise when the MOSFET turns on the sensor rail[cite: 1].                                                                                        |
| Decoupling Capacitor              | `CAP`                   | `100 nF`                     | Connect one leg to `AREF`.<br>Connect the other leg to Global `GND`[cite: 1].                                  | Noise Suppression: While you can configure the ADC inside the firmware to use `AVCC` as the reference, it is standard practice to connect a 100 nF capacitor to suppress high-frequency noise[cite: 1]. |

---

### 📐 Schematic Placement Guide

1. **The Pull-Up Resistor (`R_GATE`):** Place it vertically directly above the line connecting Master `PD4` to the top Gate pin of the MOSFET[cite: 1].
2. **The Supercapacitor (`C_BULK`):** Place this large polarized capacitor right at your battery input terminal, _before_ the MOSFET source pin[cite: 1]. This ensures it stays constantly charged by the battery and is always ready to back up the radio[cite: 1].
3. **The Decoupling Capacitor (`C_BYPASS`):** Place this small ceramic capacitor directly between your `VCC_SW` node and Ground[cite: 1]. Position it as close as possible to the fake accelerometer's power input pin[cite: 1].

---

## Complete Master ATmega328P Connection Reference Table

| MCU Pin Name | Connected Net Name | Connected To Component & Pin                                | Wire Direction / Type | Engineering Function                                                                                   |
| :----------- | :----------------- | :---------------------------------------------------------- | :-------------------- | :----------------------------------------------------------------------------------------------------- |
| `PC6/RESET`  |                    | `10 kΩ` Resistor ➔ VCC                                      | Input (Pull-Up)       | Tie to power to prevent ambient static noise from causing random resets[cite: 1].                      |
| `PD2/INT0`   | `INT0`             | Accelerometer Module: `INT1` Pin                            | Input (Digital)       | Hardware wake-up trigger line. A sudden shock pulls this pin to wake the MCU from deep sleep[cite: 1]. |
| `PD3/INT1`   | `INT1`             | Fake LoRa Module: `DIO0` Pin                                | Input (Digital)       | Radio handshake line. The fake module pulls this high when a packet is sent[cite: 1].                  |
| `PD4`        | `PD4`              | `ZVP2106A` MOSFET (Q1): Gate Pin<br>`100 kΩ` Resistor ➔ VCC | Output (Digital)      | Power Gating: Drives `LOW (0V)` to turn on the sensor rail; holds `HIGH (3.6V)` in sleep[cite: 1].     |
| `VCC`        | `VCC`              | Global 3.6V Battery Rail<br>`0.47 F` Supercapacitor (+)     | Power Input           | Main constant power source. Connects to the (+) side of your supercapacitor buffer[cite: 1].           |
| `GND`        | `GND`              | Global Ground Terminal<br>`0.47 F` Supercapacitor (-)       | Power Input           | Common ground reference point for the circuit[cite: 1].                                                |
| `PB1`        | `NSS`              | Fake LoRa Module: `NSS` Pin                                 | Output (Digital)      | Active-Low SPI Chip Select dedicated to the radio module[cite: 1].                                     |
| `PB2`        | `CS_ACCEL`         | Accelerometer Module: `CS` Pin                              | Output (Digital)      | Active-Low SPI Chip Select dedicated to the motion tracker[cite: 1].                                   |
| `PB3/MOSI`   | `MOSI`             | Accelerometer: `SDA` Pin<br>Fake LoRa Module: `MOSI` Pin    | Output (Shared SPI)   | Master Out Slave In line. Shared communication path to send data out to sensors[cite: 1].              |
| `PB4/MISO`   | `MISO`             | Accelerometer: `SD0` Pin<br>Fake LoRa Module: `MISO` Pin    | Input (Shared SPI)    | Master In Slave Out line. Shared communication path to read bytes back into the MCU[cite: 1].          |
| `PB5/SCK`    | `SCK`              | Accelerometer: `SCL` Pin<br>Fake LoRa Module: `SCK` Pin     | Output (Shared SPI)   | SPI Serial Clock line. Generated by the master to synchronize data bit transfers[cite: 1].             |
| `AVCC`       | `VCC`              | Global 3.6V Battery Rail                                    | Power Input           | Dedicated power supply input line for the internal Analog-to-Digital Converter block[cite: 1].         |
| `GND`        | `GND`              | Global Ground Terminal                                      | Power Input           | Analog section ground reference point[cite: 1].                                                        |
| `PC1/ADC1`   | `SOIL_RAW`         | `POT-HG` Potentiometer: Middle Wiper                        | Input (Analog)        | Reads 0V–3.6V analog signals from your capacitive soil moisture probe simulator[cite: 1].              |
| `PD1/TXD`    | --                 | Virtual terminal `RXD`                                      | Output                | Display for debugging purposes[cite: 1].                                                               |

---

# Design Decisions

- **Supercapacitor (`C_BULK`):** A 0.47 F THT supercapacitor will work perfectly to handle the ~120 mA LoRa transmission spikes[cite: 1]. Ensure its voltage rating is **5.0V or 5.5V**[cite: 1]. Avoid the common 3.6V rated memory-backup supercapacitors, as running them right at their maximum operating ceiling (3.6V battery) drastically reduces their operational lifespan[cite: 1].
- **Decoupling Capacitors:** Ensure your 100 nF capacitors are ceramic disc or multilayer ceramic (MLCC) leaded radial types, placed as physically close to Pins `VCC`/`GND` and `AVCC`/`GND` of the microcontrollers as possible to keep the 8 MHz clock lines clean[cite: 1].
- **The Brown-Out Detection (BOD) Trap:** Because you are running a 3.6V battery that will drain over time, you must change the ATmega328P extended fuse bits[cite: 1]. The default Arduino factory setting assumes a 5V rail with a BOD threshold of 4.3V or 2.7V[cite: 1]. If your battery drops beneath 2.7V, the chip will lock up[cite: 1]. Ensure you flash the fuses to set the **BOD to 1.8V** or disable it completely to squeeze out the full capacity of your battery[cite: 1].
- Since the accelerometer and LoRa share MISO, the inactive slave cannot continuously drive MISO. The fake LoRa firmware must configure its MISO output appropriately so that it releases the line when NSS = HIGH. Otherwise you can get SPI bus contention. Same issue for the fake accelerometer.
