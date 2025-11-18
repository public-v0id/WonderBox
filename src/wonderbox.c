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
#define objects 1

unsigned char scr[] = { 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char litvin[] = {
	0x38, 0x44, 0x44, 0x44, 0x38, 0x10, 0x38, 0x54, 0x54, 0x54, 0x28, 0x28, 0x28, 0x6c
};

const int ledcols[] = {0, 0b00000100, 0b00001000, 0b00010000, 0b00001100, 0b00011000, 0b00010100, 0b00011100};

object_t obj[objects];

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

void litvin_update() {
	return;
}

void init_obj(object_t *obj, unsigned char** idle, unsigned char** move,
	      unsigned int idle_frames, unsigned int move_frames,
	      unsigned char xsz, unsigned char ysz, unsigned char x,
	      unsigned char y, void (*upd)(void)) {
	obj->idle = idle;
	obj->move = move;
	obj->idle_frames = idle_frames;
	obj->move_frames = move_frames;
	obj->cur_frames = 0;
	obj->xsz = xsz;
	obj->ysz = ysz;
	obj->x = x;
	obj->y = y;
	obj->update = upd;
}

void init(void) {
	init_obj(obj+0, &litvin, &litvin, 1, 1, 8, 14, 0, 0, litvin_update);
}

void update(void) {
	for (unsigned char i = 0; i < objects; ++i) {
		obj[i].update();
	}
}
