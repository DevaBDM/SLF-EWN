# PCB Design Documentation: Transmitter & Receiver

## 1. Project Overview
* **Project Name:** **SLF_EWN**
* **Date:** 2026-08-26
* **Status:** **In Review**
* **Designers:**  Dawit Berhan
* **System Goal:** The transmitter shares the status of the land to the receiver and the receiver delivers it to the dashboard.

## 2. System Architecture
* **Block Diagram:** `![[system-block-diagram.png]]`
* **Communication Matrix:**
  * **Protocol:**  LoRa
  * **Target Range:** 1 meters underground , 100 to 200 meters on surface
  * **Data Rate:** ~290 bps, with ToA of SF10, this takes arround 330 milliseconds.

---

## 3. Transmitter PCB (`TX`)
### Overview
* **Dimensions:** `50 mm x 50 mm`
* **Layer Count:** 1-Layer
* **Primary MCU:** ATmega328
* **Power Domain:**  3.6 V Li-SOCl2 (Lithium Thionyl Chloride)
### Schematics & Layout
* **Schematic View:** 
![[Schematic transmitter.png]]
* **PCB view:
![[PCB_PCB_SingleLayer_SQUARE_2026-08-26.png]]
- 3D view:
![[Attachments/transmitter 3D PCB.png]]
![[Attachments/transmitter 3D PCB back.png]]

---

## 4. Receiver PCB (`RX`)
### Overview
* **Dimensions:** `95 mm x 70 mm`
* **Layer Count:** 1-Layer
* **Primary MCU:** ATmega328p
* **Power Domain:**  2x 3.6 V Li-SOCl2 (Lithium Thionyl Chloride)

### Schematics & Layout
* **Schematic View:** `
![[Schematic_landslideReciever_2026-08-26.png]]
* **PCB View:
![[PCB_PCB_landslideReciever_2026-08-26.png]]
- 3D view:
![[Attachments/Receiver 3D PCB.png]]
![[Attachments/Receiver 3D PCB back.png]]

---

## 5. Bill of Materials (BOM)
* **Transmitter BOM:** 
	* ATmega328P-PN
	* PMOS (ZVP2106A)
	* 4x resistors
	* 2x capacitors (100nf
	* 1x electro-capacitor
	* LIS3DH module
	* Lora Module Ra-02
	* Antenna 17cm
	* Capacitive Soil Moisture Sensor v1.2
	* 3.6 V Li-SOCl2 (Lithium Thionyl Chloride)
* **Receiver BOM:** `[[RX_BOM.csv]]`
	* ATmega328P-PN
	* 3x resistors
	* 2x capacitors (100nf)
	* 2x capacitors (22pf)
	- Crystal Oscillator  
	* 1x electro-capacitor
	* Lora Module Ra-02
	* Antenna 17cm
	* 2x 3.6 V Li-SOCl2 (Lithium Thionyl Chloride)
	- Switch
	- SPI supported EEPROM Flash memory
	- FT232RL serial to USB

## 6. Testing & Debugging Logs

### Known Issues & Hardware Errata
