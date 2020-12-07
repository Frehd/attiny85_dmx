/*
 * File:   avr-main.c
 * Author: frederic
 *
 * Created on December 6, 2020, 12:46 AM
 */

#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>


#define DMX_CHANNEL 510

void setup(void) {
    DDRB |= 0x10; //set pin4 to out
}

int main(void) {
    setup();
    while (1) {
        while (PINB & 0x08) {
        } //wait until zero/packet start
        _delay_us(2); //offset half bit
        uint8_t success = 1;

        for (uint8_t i = 0; i < 22; i++) { //check for min low period
            _delay_us(4);
            if (PINB & 0x08) {
                success = 0;
                break;
            }
        }
        while (! (PINB & 0x08) ) {
        } //wait until packet commences/ low period ends
        if (success) {
            uint8_t dmx_read_val = 0;

            for (uint8_t i = 0; i < 2; i++) { //check for min high period
                _delay_us(4);
                if (! (PINB & 0x08)) {
                    success = 0;
                    break;
                }
            }
            while (PINB & 0x08) {
            } //wait until initial high period ends

            if (success) {
                _delay_us(4);
                for (uint16_t i = 0; i < DMX_CHANNEL; i++) {
                    _delay_us(4 + 4 * 8);
                    while (PINB & 0x08) {
                    } //wait until end of the 2 end bits
                    _delay_us(2); //offset half a bit
                }
                _delay_us(4); //step over start bit into first bit

                for (uint8_t i = 0; i < 8; i++) {
                    dmx_read_val |= ((PINB & 0x08)>>3) << i;
                    _delay_us(4);
                }

                if (dmx_read_val > 100) {
                    PORTB = 0x10;
                } else {
                    PORTB = 0;
                }


            }
        }
    }
}
