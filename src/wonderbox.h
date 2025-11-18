#ifndef WONDERBOX_H
#define WONDERBOX_H 

typedef struct {
	unsigned char **idle;
	unsigned char **move;
	unsigned int idle_frames;
	unsigned int move_frames;
	unsigned int cur_frames;
	unsigned char xsz;
	unsigned char ysz;
	unsigned char x;
	unsigned char y;
	void (*update)(void);
} object_t;

void timer_init(int div);
void led_init(void);
void btn_init(void);
int btn_read(void);
void led_print(int col);
void buzz(void);
void adc_init(void);
unsigned int adc_read(int pin);
void init(void);
void update(void);

#endif
