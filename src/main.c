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
unsigned prev_adcx = 0;
unsigned prev_adcy = 0;
int btn = 0;
int prev_btn = 0;
long long ticks = 0;

int main() {
	cli();
	timer_init(ocr2a[curfreq]);
	buzzer_init();
	btn_init();
	adc_init();
	init();
//	led_init();
	sei();
	while (1) {
		unsigned adc_val = adc_read(0);	
		adcx = adc_val;
		adc_val = adc_read(1);
		adcy = adc_val;
		btn = btn_read();
		update();
		prev_adcx = adcx;
		prev_adcy = adcy;
		prev_btn = btn;
		++ticks;
//		buzz();
	}
}

ISR(TIMER2_COMPA_vect)
{
/*	int btn = btn_read();
	if (btn) {
		buzz();
	} */

/*	unsigned adc_val = adc_read(0);	
	adcx = adc_val;
	adc_val = adc_read(1);
	adcy = adc_val;
	if (adcy > 768) {
		curled++;
		led_print(curled);
	}
	if (curled > 8) {
		curled = 0;
	} */
}
