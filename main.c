#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/* Pin and Register Definitions */
#define CS_PIN PB2
#define MOSI_PIN PB3
#define MISO_PIN PB4
#define SCK_PIN PB5
#define MOSFET_GATE PB0

#define REG_WHO_AM_I 0x0F
#define REG_OUT_X_L 0x28
#define REG_OUT_Y_L 0x2A
#define REG_OUT_Z_L 0x2C

/* --- UART Driver for Proteus Virtual Terminal --- */
void uart_init(uint32_t baud) {
    uint16_t ubrr = F_CPU / 16 / baud - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_print(const char *s) {
    while (*s) {
        while (!(UCSR0A & (1 << UDRE0)))
            ;
        UDR0 = *s++;
    }
}

/* --- SPI Master Initialization and Controls --- */
void spi_init_master(void) {
    DDRB |= (1 << CS_PIN) | (1 << MOSI_PIN) | (1 << SCK_PIN);
    DDRB &= ~(1 << MISO_PIN);
    PORTB |= (1 << CS_PIN); // Pull CS HIGH (Deselect)

    /* SPI Mode 3 (CPOL=1, CPHA=1), Clock speed fosc/16 */
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << CPOL) | (1 << CPHA);

    /* Clear any stale hardware flags cleanly */
    (void)SPSR;
    (void)SPDR;
}

uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
        ; // Block until bit-shifting completes
    return SPDR;
}

uint8_t read_register(uint8_t addr) {
    uint8_t result;

    PORTB &= ~(1 << CS_PIN); // Drop CS LOW
    _delay_us(5);            // Setup delay for Proteus physics

    spi_transfer(addr |
                 0x80); // Send address with LIS3DH READ bit set (Bit 7 = 1)
    _delay_us(15);      // Crucial settling window for slave processing

    result = spi_transfer(0x00); // Clock a dummy byte to receive the data
    _delay_us(5);

    PORTB |= (1 << CS_PIN); // Pull CS HIGH
    return result;
}

int main(void) {
    char buffer[64]; // Fixed size array allocation to fix compiler pointers

    uart_init(9600);
    _delay_ms(50);
    uart_print("\r\n--- LANDSLIDE NODE DEBUG BOOT ---\r\n");

    /* Power up the P-Channel MOSFET Power Rail */
    uart_print("Powering on MOSFET rail... ");
    DDRB |= (1 << MOSFET_GATE);
    PORTB &= ~(1 << MOSFET_GATE); // Pull low to open the ZVP2106A gate
    _delay_ms(200);               // Wait for power lines to stabilize
    uart_print("ON\r\n");

    spi_init_master();
    uart_print("SPI Bus Master Status... READY\r\n");

    /* Identity Verification Step */
    uart_print("Pinging Accelerometer WHO_AM_I... ");
    uint8_t id = read_register(REG_WHO_AM_I);

    if (id == 0x33) {
        uart_print("SUCCESS (ID: 0x33 Verified)\r\n");
    } else {
        sprintf(buffer, "FAILED (Returned: 0x%02X)\r\n", id);
        uart_print(buffer);
    }

    /* Configure Interrupt Line Input Pin */
    DDRD &= ~(1 << DDD2); // PD2 as input
    PORTD |= (1 << PD2);  // Enable internal pull-up resistor

    while (1) {
        /* Read sequential Low bytes from the sensor array */
        int8_t x = (int8_t)read_register(REG_OUT_X_L);
        int8_t y = (int8_t)read_register(REG_OUT_Y_L);
        int8_t z = (int8_t)read_register(REG_OUT_Z_L);

        sprintf(buffer, "X: %4d | Y: %4d | Z: %4d\r", x, y, z);
        uart_print(buffer);

        /* Read physical INT1 pin status to monitor emergency tilt signals */
        if (!(PIND & (1 << PD2))) {
            uart_print(
                "\r\n[ALERT] Hardware Motion Interrupt Triggered on INT1!\r\n");
            _delay_ms(200);
        }

        _delay_ms(300);
    }
    return 0;
}
