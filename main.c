/*
 * fake_lora_debug.c
 * Multi-node simulation + sequence number
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

static uint8_t seq = 0;      /* global sequence number */
static uint8_t node_idx = 0; /* which virtual node to send next */

/* Three simulated nodes */
typedef struct {
    uint16_t id;
    uint16_t moisture; /* 0–10000 → xx.xx % */
    int16_t x, y, z;   /* milli-g */
    uint8_t status;
} node_t;

static const node_t nodes[] = {
    {0x0001, 4850, 0, 0, 1000, 0x00},      /* NODE01 – normal */
    {0x0002, 7200, 150, -80, 980, 0x00},   /* NODE02 – wetter */
    {0x0003, 3100, -200, 120, 1050, 0x80}, /* NODE03 – emergency */
};
#define NUM_NODES (sizeof(nodes) / sizeof(nodes[0]))

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
    const node_t *n = &nodes[node_idx];

    /* Device ID */
    frame[0] = (n->id >> 8) & 0xFF;
    frame[1] = n->id & 0xFF;

    /* Moisture */
    frame[2] = (n->moisture >> 8) & 0xFF;
    frame[3] = n->moisture & 0xFF;

    /* Tilt X */
    frame[4] = (n->x >> 8) & 0xFF;
    frame[5] = n->x & 0xFF;

    /* Tilt Y */
    frame[6] = (n->y >> 8) & 0xFF;
    frame[7] = n->y & 0xFF;

    /* Tilt Z */
    frame[8] = (n->z >> 8) & 0xFF;
    frame[9] = n->z & 0xFF;

    /* Status – we put the sequence number in the low 7 bits,
       emergency flag stays in bit 7 if the node has it */
    frame[10] = (n->status & 0x80) | (seq & 0x7F);

    /* CRC over first 11 bytes */
    frame[11] = crc8(frame, 11);

    /* advance for next time */
    seq++;
    node_idx++;
    if (node_idx >= NUM_NODES)
        node_idx = 0;
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
static void uart_print_hex(uint8_t b) {
    const char hex[] = "0123456789ABCDEF";
    uart_tx(hex[b >> 4]);
    uart_tx(hex[b & 0x0F]);
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

    uart_print("\r\n=== FAKE LORA multi-node + seq ===\r\n");

    while (1) {
        if (uart_available()) {
            uint8_t c = UDR0;
            if (c == '\r' || c == '\n') {
                build_packet(); /* new values + seq */
                uart_print(">>> node ");
                uart_print_hex(frame[1]);
                uart_print(" seq=");
                uart_print_hex(frame[10] & 0x7F);
                uart_print("\r\n");

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
