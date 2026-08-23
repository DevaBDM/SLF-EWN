/*
 * master_integrated_v2.c
 * Receiver: LoRa multi-node + sequence + EEPROM log + incremental dump
 *
 * Record format in EEPROM (16 bytes):
 *   [0..3]  uint32_t Record ID (big-endian, unique & increasing)
 *   [4..15] original 12-byte LoRa frame
 *
 * Commands (UART / Virtual Terminal):
 *   D or 0x44     → full dump
 *   N             → full dump
 *   N5            → dump only records with RID > 5
 *   N123          → dump only records with RID > 123
 *   C             → clear all records
 */

#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

/* ---------- Pins ---------- */
#define RADIO_CS_PORT PORTB
#define RADIO_CS_PIN PB2
#define FLASH_CS_PORT PORTB
#define FLASH_CS_PIN PB1
#define DIO0_PIN PD2

#define FRAME_LEN 12
#define RECORD_SIZE 16 /* 4-byte ID + 12-byte frame */

/* EEPROM layout (25AA1024 = 128 KB) */
#define EE_HEADER_ADDR 0x000000UL
#define EE_DATA_START 0x000010UL
#define EE_MAX_RECORDS ((131072UL - EE_DATA_START) / RECORD_SIZE)

/* ---------- Globals ---------- */
static volatile uint8_t packet_ready = 0;
static uint8_t rx[FRAME_LEN];
static uint32_t write_count = 0;

/* ---------- CRC-8/SMBUS ---------- */
static uint8_t crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* ---------- UART ---------- */
static void uart_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_tx(uint8_t b) {
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = b;
}

static void uart_print(const char *s) {
    while (*s)
        uart_tx((uint8_t)*s++);
}

static void uart_print_u16(uint16_t v) {
    char tmp[6];
    uint8_t i = 5;
    tmp[i] = 0;
    do {
        tmp[--i] = '0' + (v % 10);
        v /= 10;
    } while (v && i);
    uart_print(&tmp[i]);
}

static void uart_print_u32(uint32_t v) {
    char tmp[11];
    uint8_t i = 10;
    tmp[i] = 0;
    do {
        tmp[--i] = '0' + (v % 10);
        v /= 10;
    } while (v && i);
    uart_print(&tmp[i]);
}

static uint8_t uart_available(void) { return (UCSR0A & (1 << RXC0)) != 0; }

static uint8_t uart_rx(void) {
    while (!(UCSR0A & (1 << RXC0)))
        ;
    return UDR0;
}

/* ---------- SPI helpers ---------- */
static void spi_set_mode0(void) {
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
    SPSR = 0;
}

static void spi_set_mode1(void) {
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0) | (1 << CPHA);
    SPSR = 0;
}

static uint8_t spi_transfer(uint8_t out) {
    SPDR = out;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

/* ---------- Flash (25AA1024) ---------- */
static void flash_select(void) {
    FLASH_CS_PORT &= ~(1 << FLASH_CS_PIN);
    _delay_us(20);
}

static void flash_deselect(void) {
    FLASH_CS_PORT |= (1 << FLASH_CS_PIN);
    _delay_us(20);
}

static void flash_wait_ready(void) {
    uint8_t status;
    do {
        flash_select();
        spi_transfer(0x05);
        status = spi_transfer(0xFF);
        flash_deselect();
    } while (status & 0x01);
}

static void flash_wren(void) {
    flash_select();
    spi_transfer(0x06);
    flash_deselect();
    _delay_us(50);
}

static void flash_write_bytes(uint32_t addr, const uint8_t *data, uint8_t len) {
    spi_set_mode0();
    flash_wren();
    flash_select();
    spi_transfer(0x02);
    spi_transfer((addr >> 16) & 0xFF);
    spi_transfer((addr >> 8) & 0xFF);
    spi_transfer(addr & 0xFF);
    for (uint8_t i = 0; i < len; i++)
        spi_transfer(data[i]);
    flash_deselect();
    flash_wait_ready();
}

static void flash_read_bytes(uint32_t addr, uint8_t *buf, uint8_t len) {
    spi_set_mode0();
    flash_select();
    spi_transfer(0x03);
    spi_transfer((addr >> 16) & 0xFF);
    spi_transfer((addr >> 8) & 0xFF);
    spi_transfer(addr & 0xFF);
    for (uint8_t i = 0; i < len; i++)
        buf[i] = spi_transfer(0xFF);
    flash_deselect();
}

/* ---------- EEPROM helpers ---------- */
static void ee_load_count(void) {
    uint8_t tmp[4];
    flash_read_bytes(EE_HEADER_ADDR, tmp, 4);
    write_count = ((uint32_t)tmp[0] << 24) | ((uint32_t)tmp[1] << 16) |
                  ((uint32_t)tmp[2] << 8) | (uint32_t)tmp[3];
    if (write_count > EE_MAX_RECORDS)
        write_count = 0;
}

static void ee_save_count(void) {
    uint8_t tmp[4] = {(write_count >> 24) & 0xFF, (write_count >> 16) & 0xFF,
                      (write_count >> 8) & 0xFF, write_count & 0xFF};
    flash_write_bytes(EE_HEADER_ADDR, tmp, 4);
}

static void ee_append_frame(const uint8_t *frame) {
    if (write_count >= EE_MAX_RECORDS)
        return;

    uint8_t rec[RECORD_SIZE];
    rec[0] = (write_count >> 24) & 0xFF;
    rec[1] = (write_count >> 16) & 0xFF;
    rec[2] = (write_count >> 8) & 0xFF;
    rec[3] = write_count & 0xFF;
    for (uint8_t i = 0; i < FRAME_LEN; i++)
        rec[4 + i] = frame[i];

    uint32_t addr = EE_DATA_START + (write_count * RECORD_SIZE);
    flash_write_bytes(addr, rec, RECORD_SIZE);
    write_count++;
    ee_save_count();
}

static void print_record(uint32_t rid, const uint8_t *frame) {
    uint16_t id = (frame[0] << 8) | frame[1];
    uint16_t moist = (frame[2] << 8) | frame[3];
    int16_t tx = (int16_t)((frame[4] << 8) | frame[5]);
    int16_t ty = (int16_t)((frame[6] << 8) | frame[7]);
    int16_t tz = (int16_t)((frame[8] << 8) | frame[9]);
    uint8_t stat = frame[10];
    uint8_t seq = stat & 0x7F;
    uint8_t emerg = stat & 0x80;

    uart_print("RID=");
    uart_print_u32(rid);
    uart_print(" ID=");
    uart_print_u16(id);
    uart_print(" SEQ=");
    uart_print_u16(seq);
    uart_print(" Moist=");
    uart_print_u16(moist / 100);
    uart_print(".");
    if ((moist % 100) < 10)
        uart_tx('0');
    uart_print_u16(moist % 100);
    uart_print("% X=");
    if (tx < 0) {
        uart_tx('-');
        tx = -tx;
    }
    uart_print_u16(tx);
    uart_print(" Y=");
    if (ty < 0) {
        uart_tx('-');
        ty = -ty;
    }
    uart_print_u16(ty);
    uart_print(" Z=");
    if (tz < 0) {
        uart_tx('-');
        tz = -tz;
    }
    uart_print_u16(tz);
    if (emerg)
        uart_print(" **EMERGENCY**");
    uart_print("\r\n");
}

static void ee_dump_from(uint32_t after_id) {
    uint32_t start = (after_id == 0xFFFFFFFFUL) ? 0 : (after_id + 1);
    if (start > write_count)
        start = write_count;

    uart_print("\r\n=== DUMP from RID ");
    uart_print_u32(start);
    uart_print(" (");
    uart_print_u32(write_count - start);
    uart_print(" new) ===\r\n");

    uint8_t buf[RECORD_SIZE];
    for (uint32_t i = start; i < write_count; i++) {
        uint32_t addr = EE_DATA_START + (i * RECORD_SIZE);
        flash_read_bytes(addr, buf, RECORD_SIZE);

        uint32_t rid = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
                       ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
        print_record(rid, &buf[4]);
    }
    uart_print("=== END DUMP ===\r\n");
}

/* ---------- LoRa RX ISR ---------- */
ISR(INT0_vect) {
    if (packet_ready)
        return;

    spi_set_mode1();
    RADIO_CS_PORT &= ~(1 << RADIO_CS_PIN);
    _delay_us(200);
    for (uint8_t i = 0; i < FRAME_LEN; i++) {
        rx[i] = spi_transfer(0xFF);
        _delay_us(150);
    }
    RADIO_CS_PORT |= (1 << RADIO_CS_PIN);
    packet_ready = 1;
}

/* ---------- Main ---------- */
int main(void) {
    DDRB |= (1 << PB3) | (1 << PB5) | (1 << RADIO_CS_PIN) | (1 << FLASH_CS_PIN);
    DDRB &= ~(1 << PB4);
    RADIO_CS_PORT |= (1 << RADIO_CS_PIN);
    FLASH_CS_PORT |= (1 << FLASH_CS_PIN);

    DDRD &= ~(1 << DIO0_PIN);

    EICRA = (1 << ISC01) | (1 << ISC00);
    EIMSK = (1 << INT0);

    uart_init(8);
    spi_set_mode0();
    sei();

    ee_load_count();

    uart_print("\r\n=== MASTER v2 (incremental dump) ===\r\n");
    uart_print("Records stored: ");
    uart_print_u32(write_count);
    uart_print("\r\nCommands: D=full  N5=new after 5  C=clear\r\n");

    while (1) {
        /* ---- UART commands ---- */
        if (uart_available()) {
            uint8_t c = uart_rx();

            if (c == 'D' || c == 'd' || c == 0x44) {
                ee_dump_from(0xFFFFFFFFUL);
            } else if (c == 'N' || c == 'n') {
                /* Wait patiently for a number (works with Proteus Virtual
                 * Terminal) */
                uint32_t last_id = 0;
                uint8_t got_digit = 0;

                /* Give the user time to type the number after pressing N */
                for (uint8_t wait = 0; wait < 150;
                     wait++) { // ~1.5 seconds total
                    if (uart_available()) {
                        uint8_t ch = uart_rx();

                        if (ch >= '0' && ch <= '9') {
                            last_id = last_id * 10 + (ch - '0');
                            got_digit = 1;
                            wait =
                                0; // reset timeout every time a digit arrives
                        } else if (ch == '\r' || ch == '\n') {
                            break; // Enter key finishes the number
                        } else {
                            break; // any other character ends it
                        }
                    }
                    _delay_ms(10);
                }

                if (got_digit) {
                    uart_print("→ requesting RID > ");
                    uart_print_u32(last_id);
                    uart_print("\r\n");
                    ee_dump_from(last_id);
                } else {
                    uart_print("→ full dump (no number given)\r\n");
                    ee_dump_from(0xFFFFFFFFUL);
                }
            } else if (c == 'C' || c == 'c') {
                write_count = 0;
                ee_save_count();
                uart_print("[CLEARED]\r\n");
            }
        }

        /* ---- LoRa packet ---- */
        if (packet_ready) {
            uint8_t calc = crc8(rx, 11);
            if (calc == rx[11]) {
                uart_print("[OK] ");
                print_record(write_count, rx);
                ee_append_frame(rx);
            } else {
                uart_print("[CRC FAIL]\r\n");
            }

            packet_ready = 0;
            while (PIND & (1 << DIO0_PIN))
                ;
        }
    }
}
