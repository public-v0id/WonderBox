#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "wonderbox.h"

int led[] = {0b00000100, 0b00001000, 0b00010000};
int ocr1a[] = {63, 91, 127};
int curled = 0;
int curfreq = 0;
int buzzerstate = 0;
unsigned prevadcx = 0;
unsigned prevadcy = 0;

void main() {
	cli();
	timer_init(ocr1a[curfreq]);
	led_init();
	btn_init();
	adc_init();
	led_print(0);
	sei();
	while (1) {
	}
}

ISR(TIMER1_COMPA_vect)
{
	int btn = btn_read();
	if (btn) {
		buzz();
	}
	unsigned adc_val = adc_read(0);
	if (adc_val > 768 && prevadcx <= 768) {
		curled = (curled+1)%8;
		led_print(curled);
	}
	else if (adc_val < 256 && prevadcx >= 256) {
		curled = (curled+7)%8;
		led_print(curled);
	}
	prevadcx = adc_val;
	adc_val = adc_read(1);
	if (adc_val > 768 && prevadcy <= 768) {
		curfreq = (curfreq+1)%3;
		OCR1A = ocr1a[curfreq];
	}
	else if (adc_val < 256 && prevadcy >= 256) {
		curfreq = (curfreq+2)%3;
		OCR1A = ocr1a[curfreq];
	}
	prevadcy = adc_val;
}
