#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "wonderbox.h"

#define btnreg PINB
#define btnpin 0b00000001
#define ledreg PORTB
#define ledmask 0b00011100
#define buzzreg PINB
#define buzzpin 0b00000010


const int ledcols[] = {0, 0b00000100, 0b00001000, 0b00010000, 0b00001100, 0b00011000, 0b00010100, 0b00011100};

void timer_init(int div) {
	TCCR1A = 0;
	TCCR1B = 0;
	TIMSK1 = (1 << TOIE1);
	TCCR1B |= 0b1100;
	OCR1A = div;
	TIMSK1 |= 0b0010;
}

void led_init(void) {
	DDRB |= 0b00011100;
}

void btn_init(void) {
	DDRB |= 0b00000010;
	PORTB |= 0b1;
}

int btn_read(void) {
	return (btnreg & btnpin) ? 0 : 1;
}

void led_print(int col) {
	ledreg &= ~(ledmask);
	ledreg |= ledcols[col];
}

void buzz(void) {
	buzzreg &= ~(buzzpin);
	buzzreg |= buzzpin;
}

void adc_init(void) {
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADMUX |= (0 << REFS1) | (1 << REFS0);
}

unsigned int adc_read(int pin) {
	ADMUX &= (0b11110000);
	ADMUX |= pin;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return (unsigned)ADC;
}
