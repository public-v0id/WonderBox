#ifndef WONDERBOX_H
#define WONDERBOX_H

void timer_init(int div);
void led_init(void);
void btn_init(void);
int btn_read(void);
void led_print(int col);
void buzz(void);
void adc_init(void);
unsigned int adc_read(int pin);

#endif
