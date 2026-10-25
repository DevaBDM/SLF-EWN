> **As-built:** ADXL345. The LIS3DH pinout is kept for reference only (future drop-in).

# Device pinouts

Pin references taken from the datasheets while drawing the schematics. Signal names are the datasheet names, not the net labels used on the board.

## ADXL345 (14-lead)

| Pin | Name | Function |
|:---:|:-----|:---------|
| 1 | VDD I/O | Digital interface supply |
| 2 | GND | Ground |
| 3 | Reserved | Connect to $V_S$ or leave open |
| 4 | GND | Ground |
| 5 | GND | Ground |
| 6 | VS | Supply voltage |
| 7 | CS | Chip select |
| 8 | INT1 | Interrupt 1 output |
| 9 | INT2 | Interrupt 2 output |
| 10 | GND | Ground |
| 11 | Reserved | Connect to GND or leave open |
| 12 | SDO / ALT ADDRESS | Serial data out; I²C alternate address select |
| 13 | SDA / SDI / SDIO | I²C data; SPI data in (4-wire); SPI data in/out (3-wire) |
| 14 | SCL / SCLK | Serial communications clock |

## LIS3DH (16-lead LGA) — reference only

| Pin | Name | Function |
|:---:|:-----|:---------|
| 1 | Vdd_IO | Supply for I/O pins |
| 2 | NC | Not connected |
| 3 | NC | Not connected |
| 4 | SCL / SPC | I²C clock / SPI clock |
| 5 | GND | 0 V supply |
| 6 | SDA / SDI / SDO | I²C data / SPI data in / 3-wire data out |
| 7 | SDO / SA0 | SPI data out / I²C address LSB |
| 8 | CS | SPI enable (low = SPI); I²C/SPI mode select |
| 9 | INT2 | Inertial interrupt 2 |
| 10 | RES | Connect to GND |
| 11 | INT1 | Inertial interrupt 1 |
| 12 | GND | 0 V supply |
| 13 | ADC3 | ADC input 3 |
| 14 | Vdd | Supply |
| 15 | ADC2 | ADC input 2 |
| 16 | ADC1 | ADC input 1 |

## W25Q64 (8-pin SOIC / PDIP / WSON)

| Pin | Name | Function |
|:---:|:-----|:---------|
| 1 | /CS | Chip select input |
| 2 | DO / IO1 | Data output (dual/quad SPI IO1) |
| 3 | /WP / IO2 | Write protect (dual/quad SPI IO2) |
| 4 | GND | Ground |
| 5 | DI / IO0 | Data input (dual/quad SPI IO0) |
| 6 | CLK | Serial clock |
| 7 | /HOLD / IO3 | Hold (dual/quad SPI IO3) |
| 8 | VCC | Supply |

### W25Q64 (16-pin SOIC)

| Pin | Name | Function |
|:---:|:-----|:---------|
| 1 | /HOLD / IO3 | Hold (dual/quad SPI IO3) |
| 2 | VCC | Supply |
| 3 | N/C | No connect |
| 4 | N/C | No connect |
| 5 | N/C | No connect |
| 6 | N/C | No connect |
| 7 | /CS | Chip select input |
| 8 | DO / IO1 | Data output (dual/quad SPI IO1) |
| 9 | /WP / IO2 | Write protect (dual/quad SPI IO2) |
| 10 | GND | Ground |
| 11 | N/C | No connect |
| 12 | N/C | No connect |
| 13 | N/C | No connect |
| 14 | N/C | No connect |
| 15 | DI / IO0 | Data input (dual/quad SPI IO0) |
| 16 | CLK | Serial clock |

## Ra-01 and Ra-02 (Ai-Thinker)

The two modules share the same pin order. They differ on pins 1 and 2 because Ra-01 uses a spring/wire antenna pad and Ra-02 uses an IPEX connector.

| Pin | Ra-01 | Ra-02 | Function |
|:---:|:------|:------|:---------|
| 1 | ANT | GND | Antenna pad (Ra-01) / ground (Ra-02) |
| 2 | GND | GND | Ground |
| 3 | 3.3V | 3.3V | 3.3 V supply |
| 4 | RESET | RESET | Device reset |
| 5 | DIO0 | DIO0 | Digital I/O 0 |
| 6 | DIO1 | DIO1 | Digital I/O 1 |
| 7 | DIO2 | DIO2 | Digital I/O 2 |
| 8 | DIO3 | DIO3 | Digital I/O 3 |
| 9 | GND | GND | Ground |
| 10 | DIO4 | DIO4 | Digital I/O 4 |
| 11 | DIO5 | DIO5 | Digital I/O 5 |
| 12 | SCK | SCK | SPI clock |
| 13 | MISO | MISO | SPI data out |
| 14 | MOSI | MOSI | SPI data in |
| 15 | NSS | NSS | SPI chip select |
| 16 | GND | GND | Ground |
