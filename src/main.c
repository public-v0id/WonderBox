#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "wonderbox.h"

int led[] = {0b00000100, 0b00001000, 0b00010000};
int ocr1a[] = {63, 91, 127};
int curled = 0;
int curfreq = 0;
int buzzerstate = 0;
unsigned adcx = 0;
unsigned adcy = 0;

void main() {
	cli();
	timer_init(ocr1a[curfreq]);
	led_init();
	btn_init();
	adc_init();
	led_print(0);
	init();
	sei();
	while (1) {
		update();
	}
}

ISR(TIMER1_COMPA_vect)
{
	int btn = btn_read();
	if (btn) {
		buzz();
	}
	unsigned adc_val = adc_read(0);	
	adcx = adc_val;
	adc_val = adc_read(1);
	adcy = adc_val;
}

