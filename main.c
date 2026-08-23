/*
 * fake_lora_debug.c – STEP 4 (real 12-byte project packet)
 */
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DIO0_PORT PORTD
#define DIO0_PIN PD3

#define NSS_PIN PB2
#define MOSI_PIN PB3
#define MISO_PIN PB4
#define SCK_PIN PB5

#define FRAME_LEN 12

static uint8_t frame[FRAME_LEN];
static volatile uint8_t idx = 0;
static volatile uint8_t sending = 0;

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

static void build_packet(void) {
    /* Device ID = 0x0001 (NODE01) */
    frame[0] = 0x00;
    frame[1] = 0x01;

    /* Moisture = 4850 → 48.50 % */
    frame[2] = 0x12; /* 4850 = 0x12F2 */
    frame[3] = 0xF2;

    /* Tilt X = 0 */
    frame[4] = 0x00;
    frame[5] = 0x00;

    /* Tilt Y = 0 */
    frame[6] = 0x00;
    frame[7] = 0x00;

    /* Tilt Z = 1000 milli-g (approx 1 g) */
    frame[8] = 0x03;
    frame[9] = 0xE8;

    /* Status = 0 (normal) */
    frame[10] = 0x00;

    /* CRC over first 11 bytes */
    frame[11] = crc8(frame, 11);
}

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
static uint8_t uart_available(void) { return (UCSR0A & (1 << RXC0)) != 0; }

static void spi_slave_init(void) {
    DDRB &= ~((1 << NSS_PIN) | (1 << MOSI_PIN) | (1 << SCK_PIN));
    DDRB |= (1 << MISO_PIN);
    SPCR = (1 << SPE) | (1 << CPHA);
    SPDR = 0x00;
}

int main(void) {
    DDRD |= (1 << DIO0_PIN);
    DIO0_PORT &= ~(1 << DIO0_PIN);

    uart_init(51);
    spi_slave_init();
    build_packet();

    uart_print("\r\n=== FAKE LORA STEP4 – real 12-byte packet ===\r\n");

    while (1) {
        if (uart_available()) {
            uint8_t c = UDR0;
            if (c == '\r' || c == '\n') {
                uart_print(">>> sending binary frame\r\n");
                idx = 0;
                SPDR = frame[0];
                DIO0_PORT |= (1 << DIO0_PIN);
                sending = 1;
            }
        }

        if (SPSR & (1 << SPIF)) {
            (void)SPDR;
            if (sending) {
                idx++;
                if (idx < FRAME_LEN) {
                    SPDR = frame[idx];
                } else {
                    DIO0_PORT &= ~(1 << DIO0_PIN);
                    sending = 0;
                    SPDR = 0x00;
                    uart_print(">>> done\r\n");
                }
            }
        }
    }
}
