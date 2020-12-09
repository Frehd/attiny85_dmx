/*
 * File:   avr-main.c
 * Author: frederic
 *
 * Created on December 6, 2020, 12:46 AM
 */

#define F_CPU 8330000UL

#include <xc.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define DMX_CHANNEL 53

void setup(void) {
    DDRB |= 0x10 | 0x04; //set pin4, pin2 to out
    PCMSK = 0x08; //enable pin3 interrupt
    GIMSK |= 0x20; //enable pin change interrupts
    TCCR1 = 1 << CS10;
}

int main(void) {
    setup();

    /*while(1) {
        PINB = 0x04;
        _delay_us(4);
    }*/

    while (1) {
        cli(); //no interrupts

        while (PINB & 0x08) {
        } //wait until zero/packet start
        _delay_us(2); //offset half bit
        uint8_t success = 1;

        for (uint8_t i = 0; i < 22; i++) { //check for min low period
            _delay_us(4);
            if (PINB & 0x08) {
                //PINB = 0x04; //toggle pin2 for debugging purposes
                success = 0;
                break;
            }
        }
        while (!(PINB & 0x08)) {
        } //wait until packet commences/ low period ends
        if (success) {
            uint8_t dmx_read_val = 0;

            for (uint8_t i = 0; i < 2; i++) { //check for min high period
                _delay_us(4);
                if (!(PINB & 0x08)) {
                    //PINB = 0x04; //toggle pin2 for debugging purposes
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
                    _delay_us(1); //offset half a bit --hand tuned
                }
                _delay_us(2); //step over start bit into first bit --hand tuned


                /*for (uint8_t i = 0; i < 8; i++) { //we've only got 32 clock cycles in between bits
                    TCNT1 = 0;
                    dmx_read_val >>= 1; 
                    PINB = 0x04;
                    if(PINB & 0x08) {
                        dmx_read_val += 128;
                    }
                    PINB = 0x04;
                    
                    while(TCNT1 < 21){} //relies on the loop using 5 cycles
                }*/
                
                uint8_t counter = 0;
                
                asm("LDI R24, 0x04"); //todo: use counter
                asm("LDI R25, 0x08");   //1 cycle
                asm("loop: rjmp read_dmx_bit"); //2 cycles
                asm("dec R25"); //1 cycle
                asm("brne loop");   //2 cycles
                asm("rjmp next");

                asm("read_dmx_bit:");
                asm("    OUT 0x16, R24");
                asm("    LSR %0" : "=r"(dmx_read_val)); //shift to the right    1 cycle
                asm("    SBIC 0x16, 3"); //skip add if pin3 isn't high (0x16)   1 cycle if isn't skipped 2 if skipped
                asm("    SUBI %0, 0x80" : "=r"(dmx_read_val)); //sets highest bit   1 cycle
                asm("    OUT 0x16, R24");
                
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                asm("    nop");
                
                asm("next:");
                
                if (dmx_read_val > 100) {
                    PORTB = 0x10;
                } else if (dmx_read_val == 0) {
                    PORTB = 0;
                }
            }
        }
        GIFR = 0x20; //clear pin change interrupt
        sei();
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
    }
}
