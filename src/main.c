#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int led = 0;

void main() {
	cli();
	TCCR1A = 0;
	TCCR1B = 0;
	TIMSK1 = (1 << TOIE1);
	TCCR1B |= 0b1100;
	OCR1A = 63;
	TIMSK1 |= 0b0010;
	DDRB |= 0b00011110;
//	int cur = 0b00000100;
	sei();
	while (1) {
/*		PORTB = cur;
		cur <<= 1;
		if (cur > 0b00011100) {
			cur = 0b00000100;
		}
		_delay_ms(2000); */
	}
}

ISR(TIMER1_COMPA_vect)
{
	PORTB = 0;
	PORTB |= led & 0b10;
	led = ~led;
}
