/*
 * master_debug.c – STEP 4 (real 12-byte project packet + INT0)
 */
#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define RADIO_CS_PORT PORTB
#define RADIO_CS_PIN PB2
#define DIO0_PIN PD2

#define FRAME_LEN 12

static volatile uint8_t packet_ready = 0;
static uint8_t rx[FRAME_LEN];

/* CRC-8/SMBUS */
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

static void uart_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);
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
static void uart_print_hex(uint8_t b) {
    const char hex[] = "0123456789ABCDEF";
    uart_tx(hex[b >> 4]);
    uart_tx(hex[b & 0x0F]);
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

static void spi_master_init(void) {
    DDRB |= (1 << PB3) | (1 << PB5) | (1 << RADIO_CS_PIN);
    DDRB &= ~(1 << PB4);
    RADIO_CS_PORT |= (1 << RADIO_CS_PIN);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0) | (1 << CPHA);
    SPSR = 0;
}
static uint8_t spi_transfer(uint8_t out) {
    SPDR = out;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

/* INT0 rising edge */
ISR(INT0_vect) {
    if (packet_ready)
        return;

    RADIO_CS_PORT &= ~(1 << RADIO_CS_PIN);
    _delay_us(200);

    for (uint8_t i = 0; i < FRAME_LEN; i++) {
        rx[i] = spi_transfer(0xFF);
        _delay_us(150);
    }

    RADIO_CS_PORT |= (1 << RADIO_CS_PIN);
    packet_ready = 1;
}

int main(void) {
    DDRD &= ~(1 << DIO0_PIN);

    EICRA = (1 << ISC01) | (1 << ISC00); /* rising edge */
    EIMSK = (1 << INT0);

    uart_init(8);
    spi_master_init();
    sei();

    uart_print("\r\n=== MASTER STEP4 – real binary packet ===\r\n");

    while (1) {
        if (packet_ready) {
            uint8_t calc = crc8(rx, 11);

            if (calc == rx[11]) {
                uint16_t id = (rx[0] << 8) | rx[1];
                uint16_t moist = (rx[2] << 8) | rx[3];
                int16_t tx = (rx[4] << 8) | rx[5];
                int16_t ty = (rx[6] << 8) | rx[7];
                int16_t tz = (rx[8] << 8) | rx[9];
                uint8_t stat = rx[10];

                uart_print("[OK] ID=");
                uart_print_u16(id);
                uart_print("  Moist=");
                uart_print_u16(moist / 100);
                uart_print(".");
                uart_print_u16(moist % 100);
                uart_print("%  X=");
                uart_print_u16(tx);
                uart_print("  Y=");
                uart_print_u16(ty);
                uart_print("  Z=");
                uart_print_u16(tz);
                uart_print("  STAT=0x");
                uart_print_hex(stat);
                uart_print("\r\n");
            } else {
                uart_print("[CRC FAIL]\r\n");
            }

            packet_ready = 0;
            while (PIND & (1 << DIO0_PIN))
                ;
        }
    }
}
