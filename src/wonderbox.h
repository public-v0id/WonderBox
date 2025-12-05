#ifndef WONDERBOX_H
#define WONDERBOX_H 

typedef struct object {
	unsigned char **idle;
	unsigned char **move;
	unsigned char **cur_anim;
	unsigned int idle_frames;
	unsigned int move_frames;
	unsigned int cur_frame;
	unsigned char xsz;
	unsigned char ysz;
	double x;
	double y;
	double vx;
	double vy;
	void (*update)(struct object *);
} object_t;

typedef struct {
	unsigned char x;
	unsigned char y;
} snake_t;

typedef struct {
	void (*init)(void);
	void (*update)(void);
	void (*deinit)(void);
} game_t;

void timer_init(int div);
void buzzer_init(void);
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
