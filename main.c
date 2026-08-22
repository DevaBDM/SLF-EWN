/*
 * master_debug.c
 * Receives bytes from the fake LoRa and prints them on TXD.
 * Mode 1 + /128 clock + longer CS settle (Proteus fix)
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define RADIO_CS_PORT PORTB
#define RADIO_CS_PIN PB2

#define DIO0_PIN_REG PIND
#define DIO0_PIN PD2

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

static void spi_master_init(void) {
    DDRB |= (1 << PB3) | (1 << PB5) | (1 << RADIO_CS_PIN);
    DDRB &= ~(1 << PB4);
    RADIO_CS_PORT |= (1 << RADIO_CS_PIN);

    /* Mode 1 + /128 – Proteus timing fix */
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0) | (1 << CPHA);
    SPSR = 0;
}

static uint8_t spi_transfer(uint8_t out) {
    SPDR = out;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

int main(void) {
    DDRD &= ~(1 << DIO0_PIN);

    uart_init(8); /* 115200 @ 16 MHz */
    spi_master_init();

    uart_print("\r\n=== MASTER – waiting for LoRa data ===\r\n");

    while (1) {
        if (DIO0_PIN_REG & (1 << DIO0_PIN)) {
            RADIO_CS_PORT &= ~(1 << RADIO_CS_PIN);
            _delay_us(50); /* longer settle */

            uint8_t b = spi_transfer(0xFF);

            RADIO_CS_PORT |= (1 << RADIO_CS_PIN);

            uart_print("[RX] 0x");
            uart_print_hex(b);
            uart_print("  '");
            if (b >= 32 && b < 127)
                uart_tx(b);
            else
                uart_tx('?');
            uart_print("'\r\n");
        }
        _delay_ms(50);
    }
}
