/*
 * fake_lora_debug.c
 * Raises DIO0 and pushes the payload.
 * Mode 1 to match the master.
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

static const uint8_t payload[] = "NODE01_DATA:X_0:Y_0:Z_0:MOIST_48%:SEQ_2";
#define PAYLOAD_LEN (sizeof(payload) - 1)

static volatile uint8_t idx = 0;
static volatile uint16_t hold_ms = 0;

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

    /* Mode 1 – match master */
    SPCR = (1 << SPE) | (1 << CPHA);
    SPDR = payload[0];
    idx = 0;
}

int main(void) {
    DDRD |= (1 << DIO0_PIN);
    DIO0_PORT &= ~(1 << DIO0_PIN);

    uart_init(51); /* 9600 @ 8 MHz */
    spi_slave_init();

    uart_print("\r\n=== FAKE LORA – type Enter to send payload ===\r\n");

    while (1) {
        if (uart_available()) {
            uint8_t c = UDR0;
            if (c == '\r' || c == '\n') {
                uart_print(">>> sending payload, DIO0 HIGH\r\n");
                DIO0_PORT |= (1 << DIO0_PIN);
                hold_ms = 2000;
                idx = 0;
                SPDR = payload[0];
            }
        }

        if (SPSR & (1 << SPIF)) {
            uint8_t dummy = SPDR;
            (void)dummy;

            uart_print("[SPI] sent '");
            uart_tx(payload[idx]);
            uart_print("'\r\n");

            idx++;
            if (idx >= PAYLOAD_LEN)
                idx = 0;
            SPDR = payload[idx];
        }

        if (hold_ms > 0) {
            _delay_ms(1);
            hold_ms--;
            if (hold_ms == 0) {
                DIO0_PORT &= ~(1 << DIO0_PIN);
                uart_print(">>> DIO0 LOW\r\n");
            }
        }
    }
}
