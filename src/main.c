#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "wonderbox.h"

int ocr2a[] = {63, 91, 127};
int curled = 0;
int curfreq = 0;
int buzzerstate = 0;
unsigned adcx = 0;
unsigned adcy = 0;

void main() {
	cli();
	timer_init(ocr2a[curfreq]);
	buzzer_init();
	led_init();
	btn_init();
	adc_init();
	sei();
	init();
	while (1) {
		update();
	}
}

ISR(TIMER2_COMPA_vect)
{
	int btn = btn_read();
	if (btn) {
		buzz();
	}
	unsigned adc_val = adc_read(0);	
	adcx = adc_val;
	adc_val = adc_read(1);
	adcy = adc_val;
	if (adcy > 768) {
		curled++;
		led_print(curled);
	}
	if (curled > 8) {
		curled = 0;
	}
	
}
