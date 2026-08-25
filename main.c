/* =====================================================================
 * SLF-EWN  --  Proteus "Fake LoRa" TX Child-Sheet Simulator
 * Target: ATmega328P, 8 MHz internal RC, 3.6 V
 *
 * After receiving a valid 12-byte packet from the master it:
 *   1. Prints the human-readable line (debug)
 *   2. Broadcasts the raw 12-byte frame on UART TXD
 *      (wire this TXD to the shared ANT net / Receiver Fake LoRa RXD)
 *   3. Pulses DIO0 to acknowledge the master
 * ===================================================================== */

#define F_CPU 8000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define LORA_CMD_FIFO_WRITE 0x8Du
#define PACKET_LEN 12u
#define STATUS_EMERGENCY_BIT 0x80u

#define NSS_PIN PB2
#define MOSI_PIN PB3
#define MISO_PIN PB4
#define SCK_PIN PB5
#define DIO0_PIN PD3

/* ---------------- UART --------------------------------------------------- */

static void uart_init(void) {
    uint16_t ubrr = (F_CPU / (16UL * 9600UL)) - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = c;
}

static void uart_puts(const char *s) {
    while (*s)
        uart_putc(*s++);
}

static void uart_put_u16(uint16_t v) {
    char buf[6];
    uint8_t i = 0;
    if (v == 0) {
        uart_putc('0');
        return;
    }
    while (v > 0 && i < sizeof(buf)) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }
    while (i > 0)
        uart_putc(buf[--i]);
}

static void uart_put_i16(int16_t v) {
    if (v < 0) {
        uart_putc('-');
        uart_put_u16((uint16_t)(-(int32_t)v));
    } else {
        uart_put_u16((uint16_t)v);
    }
}

/* Send the raw 12-byte binary frame (for the shared ANT net) */
static void uart_send_raw(const uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        uart_putc(buf[i]);
        _delay_us(200); /* keep it slow & reliable */
    }
}

/* ---------------- CRC ---------------------------------------------------- */

static uint8_t crc8_smbus(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07)
                               : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

/* ---------------- SPI slave ---------------------------------------------- */

static volatile uint8_t rx_buf[PACKET_LEN + 1];
static volatile uint8_t rx_index = 0;
static volatile uint8_t frame_ready = 0;

ISR(SPI_STC_vect) {
    uint8_t received = SPDR;
    if (rx_index < sizeof(rx_buf)) {
        rx_buf[rx_index++] = received;
    }
    SPDR = 0x00;
}

ISR(PCINT0_vect) {
    if (PINB & (1 << NSS_PIN)) {
        DDRB &= ~(1 << MISO_PIN);
        if (rx_index >= (PACKET_LEN + 1) && rx_buf[0] == LORA_CMD_FIFO_WRITE) {
            frame_ready = 1;
        }
        rx_index = 0;
    } else {
        DDRB |= (1 << MISO_PIN);
        rx_index = 0;
        frame_ready = 0;
    }
}

static void spi_slave_init(void) {
    DDRB &=
        ~((1 << NSS_PIN) | (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << MISO_PIN));
    SPCR = (1 << SPE) | (1 << SPIE);
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT2);
}

/* ---------------- DIO0 --------------------------------------------------- */

static void dio0_pulse(void) {
    PORTD |= (1 << DIO0_PIN);
    _delay_ms(2);
    PORTD &= ~(1 << DIO0_PIN);
}

/* ---------------- Process frame ------------------------------------------ */

static void process_frame(void) {
    uint8_t local[PACKET_LEN];
    for (uint8_t i = 0; i < PACKET_LEN; i++) {
        local[i] = rx_buf[i + 1];
    }

    uint16_t device_id = ((uint16_t)local[0] << 8) | local[1];
    uint16_t moisture = ((uint16_t)local[2] << 8) | local[3];
    int16_t tilt_x = (int16_t)(((uint16_t)local[4] << 8) | local[5]);
    int16_t tilt_y = (int16_t)(((uint16_t)local[6] << 8) | local[7]);
    int16_t tilt_z = (int16_t)(((uint16_t)local[8] << 8) | local[9]);
    uint8_t status = local[10];
    uint8_t rx_crc = local[11];
    uint8_t calc_crc = crc8_smbus(local, 11);

    if (rx_crc != calc_crc) {
        uart_puts("[RADIO] CRC FAIL - discarded\r\n");
        return;
    }

    /* Human-readable debug */
    uart_puts("[RADIO TX] NODE");
    uart_put_u16(device_id);
    uart_puts(" X:");
    uart_put_i16(tilt_x);
    uart_puts(" Y:");
    uart_put_i16(tilt_y);
    uart_puts(" Z:");
    uart_put_i16(tilt_z);
    uart_puts(" Moist:");
    uart_put_u16(moisture / 100);
    uart_puts("%");
    if (status & STATUS_EMERGENCY_BIT)
        uart_puts(" EMERGENCY");
    uart_puts("\r\n");

    /* === Broadcast the raw 12-byte frame onto the shared ANT net === */
    uart_send_raw(local, PACKET_LEN);
}

/* ---------------- main --------------------------------------------------- */

int main(void) {
    DDRD |= (1 << DIO0_PIN);
    PORTD &= ~(1 << DIO0_PIN);

    uart_init();
    spi_slave_init();
    sei();

    uart_puts("=== FAKE LORA TX READY ===\r\n");

    for (;;) {
        if (frame_ready) {
            frame_ready = 0;
            process_frame();
            dio0_pulse();
        }
    }
}
