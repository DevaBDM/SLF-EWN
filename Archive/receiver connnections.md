Got it! Using the ATmega328P on the receiver side is completely doable, even for storing data.

Because the ATmega328P's internal memory (EEPROM) is too small to store a database of logs from 5+ devices, you just need to add one extra through-hole component to your receiver PCB: a Through-Hole MicroSD Card Module or an external I2C EEPROM chip. This allows the ATmega328P to act as the master controller, log data 24/7 to the storage component, and dump it over USB when a computer connects. [1]

Here is the complete hardware and connection plan for your Through-Hole (THT) Receiver PCB in EasyEDA.

---

## 1. Core Components for the Receiver PCB (EasyEDA Layout)

|Component Function|Component Name|EasyEDA Library Search Keyword|Mounting Style|
|---|---|---|---|
|Main Controller|ATmega328P-PU (DIP-28 chip)|`ATMEGA328P-PU`|28-Pin DIP IC Socket|
|Radio Receiver|RFM95W Breakout Adapter Board|`RFM95W Breakout`|Through-Hole Header (2.54mm)|
|The Database / Storage|MicroSD Card SPI Module (THT)|`MicroSD Card Module`|Through-Hole Header (2.54mm)|
|PC USB Interface|FTDI FT232RL USB-to-Serial Board|`FT232RL Module` or `FTDI Breakout`|Through-Hole Header (2.54mm)|
|Antenna Port|Through-Hole SMA Female Connector|`SMA-TH` or `SMA-JW-P`|Through-Hole 5-Pin Jack|

_Design Note:_ Just like the transmitter, do not solder the ATmega328P or the modules directly. Solder standard female header strips (`HDR-F-2.54`) and a DIP-28 socket to the PCB so everything can be plugged in easily.

---

## 2. THT Schematic Pin Connections (EasyEDA Routing)

Since the ATmega328P, the RFM95W, and the MicroSD card module all use the SPI communication bus, they will share the exact same data lines. The ATmega328P will use separate "Chip Select" pins to talk to them individually. [2]

## A. Power Infrastructure

The FTDI USB module plugs into the PC and provides 5V power. You will route this to a 3.3V through-hole voltage regulator (like an `LM1117-3.3` in a TO-220 through-hole package) because the RFM95W and MicroSD card require 3.3V. [3, 4]

- VCC (3.3V Rail): Connect to ATmega328P Pin 7, RFM95W VCC, and MicroSD VCC.
- GND Rail: Connect to ATmega328P Pins 8 & 22, RFM95W GND, MicroSD GND, FTDI GND, and SMA Connector Ground legs.

## B. The Shared SPI Data Bus

- ATmega328P Pin 17 (MOSI) → Connect to both RFM95W MOSI and MicroSD MOSI.
- ATmega328P Pin 18 (MISO) → Connect to both RFM95W MISO and MicroSD MISO.
- ATmega328P Pin 19 (SCK) → Connect to both RFM95W SCK and MicroSD SCK.

## C. Dedicated Control & Interrupt Lines

- ATmega328P Pin 14 (Digital 8) → Connect to RFM95W NSS (Radio Chip Select).
- ATmega328P Pin 15 (Digital 9) → Connect to MicroSD CS (Storage Chip Select).
- ATmega328P Pin 4 (Digital 2 / INT0) → Connect to RFM95W DIO0. (Critical Interrupt). This pin forces the ATmega328P to stop what it is doing and instantly save the incoming LoRa packet to the SD card database.
- ATmega328P Pin 1 (RESET) → Connect a 10kΩ through-hole resistor from this pin to the 3.3V VCC rail to keep the MCU stable.

## D. The USB-to-PC Connection (Serial Link)

- ATmega328P Pin 2 (RXD) → Connect to FTDI Module TX.
- ATmega328P Pin 3 (TXD) → Connect to FTDI Module RX.

---

## 3. How the Receiver PCB Behaves in This Setup

1. Standalone Mode (PC Disconnected): The tower receiver runs continuously. When a sensor node sends data, the RFM95W wakes up the ATmega328P via Pin 4 (DIO0). The ATmega328P opens a text or CSV file on the MicroSD card (acting as your database) and appends a new line with the sensor data, timestamp, and node ID.
2. Dashboard Sync Mode (PC Connected): When you turn on your desktop/PC and connect the USB cable to the FTDI module, your desktop software sends a trigger command over the Serial connection. The ATmega328P reads the CSV database file line-by-line from the MicroSD card and dumps it straight through the USB cable to your dashboard software.


## 1. How the ATmega328P Knows Who is Talking

In your code, you will assign a unique Node ID number (like `1`, `2`, `3`, etc.) to each transmitter. When a transmitter sends data, it includes its ID at the beginning of the radio message. The ATmega328P reads that ID and saves it to the MicroSD card database alongside the sensor value. [1]

## 2. How it Handles Data Storage

The MicroSD card functions like a giant spreadsheet. Whenever any transmitter sends a message, the ATmega328P simply appends a new row to a `.csv` text file on the card:

```text
Timestamp, Node_ID, Sensor_Value
14:02:11,  1,       23.5
14:02:15,  4,       102.1
14:02:30,  2,       22.9
```

Because MicroSD cards have gigabytes of storage, this single receiver can store millions of readings from dozens of different transmitters for years.

## 3. The One Software Rule You Must Follow

Since the receiver only has one RFM95W radio module, it can only listen to one transmitter at a single millisecond. If two of your 5+ transmitters talk at the exact same instant, their signals will clash (collide), and the receiver won't hear either one.

To prevent this with an ATmega328P receiver, you must use Time-Division in your transmitter code.

- Do not have your transmitters send data continuously.
- Have them sleep, wake up quickly, send their data packet (which only takes a few milliseconds), and go back to sleep.
- Add a random small delay (jitter) of a few seconds to each transmitter's timer so they naturally stay out of each other's way and never transmit at the exact same moment.