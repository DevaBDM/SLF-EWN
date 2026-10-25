> **Archived draft.** This is the earlier MicroSD / RFM95W version of the receiver. The built tower uses W25Q64 and Ra-02 — see [[Receiver PCB]] and [[Firmware Hub]].

# Receiver connections — archived THT plan

The ATmega328P is enough to run the receiver and log data. Its internal EEPROM is too small for a multi-node history, so this draft added one extra through-hole part to the receiver board: a THT MicroSD card module or an external I2C EEPROM. The ATmega328P stays the master, logs continuously, and dumps over USB when a computer connects.

## 1. Core parts for the receiver PCB

| Function | Part | EasyEDA search | Mount |
|---|---|---|---|
| Main controller | ATmega328P-PU (DIP-28) | `ATMEGA328P-PU` | 28-pin DIP socket |
| Radio receiver | RFM95W breakout | `RFM95W Breakout` | 2.54 mm through-hole header |
| Storage | MicroSD card SPI module (THT) | `MicroSD Card Module` | 2.54 mm through-hole header |
| USB interface | FTDI FT232RL USB-to-serial board | `FT232RL Module` or `FTDI Breakout` | 2.54 mm through-hole header |
| Antenna | Through-hole SMA female | `SMA-TH` or `SMA-JW-P` | 5-pin THT jack |

As on the transmitter, the ATmega328P and the modules are not soldered directly. Solder female header strips (`HDR-F-2.54`) and a DIP-28 socket so everything can be plugged in.

## 2. THT connections

The ATmega328P, RFM95W, and MicroSD module share the SPI bus. The MCU uses separate chip-select pins to address them one at a time.

### A. Power

The FTDI module takes 5 V from the PC. That feeds an LM1117-3.3 in a TO-220 through-hole package, because the RFM95W and MicroSD card need 3.3 V.

- 3.3 V rail: ATmega328P pin 7, RFM95W VCC, MicroSD VCC.
- Ground: ATmega328P pins 8 and 22, RFM95W GND, MicroSD GND, FTDI GND, SMA ground legs.

### B. Shared SPI bus

- ATmega328P pin 17 (MOSI) → RFM95W MOSI and MicroSD MOSI.
- ATmega328P pin 18 (MISO) → RFM95W MISO and MicroSD MISO.
- ATmega328P pin 19 (SCK) → RFM95W SCK and MicroSD SCK.

### C. Control and interrupt lines

- ATmega328P pin 14 (digital 8) → RFM95W NSS (radio chip select).
- ATmega328P pin 15 (digital 9) → MicroSD CS (storage chip select).
- ATmega328P pin 4 (digital 2 / INT0) → RFM95W DIO0. The radio pulls this to make the MCU stop and save the incoming packet.
- ATmega328P pin 1 (RESET) → 10 kΩ to 3.3 V.

### D. USB serial link

- ATmega328P pin 2 (RXD) → FTDI TX.
- ATmega328P pin 3 (TXD) → FTDI RX.

## 3. Behaviour

1. **PC disconnected.** The receiver runs continuously. When a node transmits, the RFM95W raises DIO0 on pin 4. The ATmega328P appends a line to a text/CSV file on the MicroSD card with the sensor value, timestamp, and node ID.
2. **PC connected.** The dashboard sends a trigger over serial. The ATmega328P reads the CSV file line by line from the MicroSD card and sends it over USB.

## 4. Node identity and storage

Each transmitter gets a unique node ID (`1`, `2`, `3`, …) in firmware. The ID sits at the start of every radio message, so the receiver can store each reading under the right node.

The MicroSD card behaves like a large table. Every message appends a row:

```text
Timestamp, Node_ID, Sensor_Value
14:02:11,  1,       23.5
14:02:15,  4,       102.1
14:02:30,  2,       22.9
```

A card of a few gigabytes holds millions of readings from dozens of transmitters.

## 5. Collision rule

The receiver has one radio, so it demodulates one packet at a time. If two nodes transmit at the same instant, both frames collide and neither is heard.

The transmitters therefore use time division:

- Do not transmit continuously.
- Sleep, wake quickly, send the packet, and go back to sleep.
- Add a small jitter delay based on the node ID so wakes drift apart instead of lining up.

The final design keeps this rule but replaces the MicroSD card with W25Q64 and the RFM95W with Ra-02.
