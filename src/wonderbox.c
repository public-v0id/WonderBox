#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nokia5110.h"
#include "wonderbox.h"

#define btnreg PIND
#define btnpin 0b00000100
#define ledreg PORTD
#define ledmask 0b11100000
#define buzzreg PORTD
#define buzzpin 0b00001000
#define objects 1

extern unsigned adcx;
extern unsigned adcy;
unsigned buzzval;

unsigned char litvin_idle[1][14] = {
	{0x38, 0x44, 0x44, 0x44, 0x38, 0x10, 0x38, 0x54, 0x54, 0x54, 0x28, 0x28, 0x28, 0x6c}
};

const int ledcols[] = {0, 0b10000000, 0b00100000, 0b01000000, 0b01100000, 0b11000000, 0b10100000, 0b11100000};

object_t obj[objects];

void timer_init(int div) {
	TCNT2 = 0;
	TIMSK2 |= (1 << OCIE2A);
	TCCR2A |= (1 << WGM21);
	TCCR2B |= (1 << CS21);
	OCR2A = div;
}

void led_init(void) {
	DDRD |= ledmask;
}

void btn_init(void) {
	DDRD &= ~btnpin;
	PORTD |= btnpin;
}

int btn_read(void) {
	return (btnreg & btnpin) ? 0 : 1;
}

void led_print(int col) {
	ledreg &= ~(ledmask);
	ledreg |= ledcols[col];
}

void buzzer_init(void) {
	DDRD |= buzzpin;
	buzzval = 0;
}

void buzz(void) {
	buzzreg &= ~(buzzpin);
	buzzreg |= (buzzpin & buzzval);
	buzzval = ~buzzval;
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

void litvin_update(object_t *lit) {
	if (adcx < 256) {
		lit->x--;
	}
	if (adcx > 768) {
		lit->x++;
	}
}

void init_obj(object_t *obj, unsigned char** idle, unsigned char** move,
	      unsigned int idle_frames, unsigned int move_frames,
	      unsigned char xsz, unsigned char ysz, unsigned char x,
	      unsigned char y, void (*upd)(object_t *)) {
	obj->idle = idle;
	obj->move = move;
	obj->cur_anim = idle;
	obj->idle_frames = idle_frames;
	obj->move_frames = move_frames;
	obj->cur_frame = 0;
	obj->xsz = xsz;
	obj->ysz = ysz;
	obj->x = x;
	obj->y = y;
	obj->update = upd;
}

void init(void) {
//	init_obj(obj+0, litvin_idle, litvin_idle, 1, 1, 8, 14, 0, 0, &litvin_update);
	nokia_lcd_init();
	nokia_lcd_clear();
	nokia_lcd_write_string("HELLO WORLD", 1);
	nokia_lcd_render();
}

void update() {
	nokia_lcd_clear();
	for (unsigned char i = 0; i < objects; ++i) {
		obj[i].update(&obj[i]);
		for (unsigned char j = 0; j < obj[i].ysz; ++j) {
			for (unsigned char k = 0; k < obj[i].xsz; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					nokia_lcd_set_pixel((k>>3)+bit+obj[i].x ,obj[i].y+j, litvin_idle[0][j] & mask);
					mask *= 2;
				}
			}
		}
	}
	nokia_lcd_render();
}
