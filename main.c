#include <avr/io.h>
#include <util/delay.h>

/* LIS3DH Register Map Configuration */
#define WHO_AM_I 0x0F
#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D
#define WHO_AM_I_VAL 0x33

/* Full 64-byte Register Workspace (0x00 to 0x3F) */
uint8_t registers[64];
int16_t x_axis = 0, y_axis = 0, z_axis = 0;

void spi_slave_init(void) {
    DDRB |= (1 << DDB4); // Set MISO as Output, all other SPI lines are Inputs
    /* Enable SPI, Slave Mode, SPI Mode 3 (CPOL=1, CPHA=1) */
    SPCR = (1 << SPE) | (1 << CPOL) | (1 << CPHA);
}

void update_fake_data(void) {
    /* X-Axis Navigation Buttons (PD4 +, PD5 -) */
    if (!(PIND & (1 << PD4))) {
        x_axis += 10;
        _delay_ms(150);
    }
    if (!(PIND & (1 << PD5))) {
        x_axis -= 10;
        _delay_ms(150);
    }

    /* Y-Axis Navigation Buttons (PD6 +, PD7 -) */
    if (!(PIND & (1 << PD6))) {
        y_axis += 10;
        _delay_ms(150);
    }
    if (!(PIND & (1 << PD7))) {
        y_axis -= 10;
        _delay_ms(150);
    }

    /* Z-Axis Navigation Buttons (PB0 +, PC3 -) */
    if (!(PINB & (1 << PB0))) {
        z_axis += 10;
        _delay_ms(150);
    }
    if (!(PINC & (1 << PC3))) {
        z_axis -= 10;
        _delay_ms(150);
    }

    /* Parse 16-bit integers into low and high byte blocks */
    registers[OUT_X_L] = (uint8_t)(x_axis & 0xFF);
    registers[OUT_X_H] = (uint8_t)(x_axis >> 8);
    registers[OUT_Y_L] = (uint8_t)(y_axis & 0xFF);
    registers[OUT_Y_H] = (uint8_t)(y_axis >> 8);
    registers[OUT_Z_L] = (uint8_t)(z_axis & 0xFF);
    registers[OUT_Z_H] = (uint8_t)(z_axis >> 8);
}

int main(void) {
    /* Set button IO configurations with internal pull-up resistor vectors */
    DDRD &= ~((1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));
    PORTD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);
    DDRC &= ~(1 << PC3);
    PORTC |= (1 << PC3);

    registers[WHO_AM_I] = WHO_AM_I_VAL;
    spi_slave_init();

    uint8_t current_addr = 0;

    while (1) {
        update_fake_data();

        /* Non-blocking SPI State Machine to completely eliminate overrun
         * warnings */
        if (SPSR & (1 << SPIF)) {
            uint8_t received_byte =
                SPDR; // Accessing SPDR instantly resets the SPIF bit

            /* Verify Chip Select (PB2) is held LOW by the Master MCU */
            if (!(PINB & (1 << PB2))) {

                if (received_byte & 0x80) { // If bit 7 is high, Master is
                                            // executing a READ operation
                    current_addr =
                        received_byte & 0x3F; // Bound check address array

                    /* Preload next transmission data instantly while master
                     * handles bytes */
                    SPDR = registers[current_addr];
                } else {
                    current_addr =
                        received_byte & 0x3F; // Handle a write block if needed
                }
            }
        }
    }
    return 0;
}
