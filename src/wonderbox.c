#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nokia5110.h"
#include "wonderbox.h"
#include "st7735.h"

#define btnreg PIND
#define btnpin 0b00000100
#define ledreg PORTD
#define ledmask 0b11100000
#define buzzreg PORTD
#define buzzpin 0b00001000
#define objects 2
#define games_n 3

extern unsigned adcx;
extern unsigned adcy;
extern int btn;
extern unsigned prev_adcx;
extern unsigned prev_adcy;
extern int prev_btn;
extern long long ticks;
struct signal cs = { .ddr = &DDRB, .port = &PORTB, .pin = 2 };
  // Back Light
struct signal bl = { .ddr = &DDRB, .port = &PORTB, .pin = 0 };
  // Data / Command
struct signal dc = { .ddr = &DDRB, .port = &PORTB, .pin = 1 };
  // Reset
struct signal rs = { .ddr = &DDRB, .port = &PORTB, .pin = 4 };
  // LCD struct
struct st7735 lcd = { .cs = &cs, .bl = &bl, .dc = &dc, .rs = &rs };
unsigned buzzval;

unsigned char litvin_idle_0[14] = {0x38, 0x44, 0x44, 0x44, 0x38, 0x10, 0x38, 0x54, 0x54, 0x54, 0x28, 0x28, 0x28, 0x6c};

unsigned char *litvin_idle[1] = { &litvin_idle_0 };

unsigned char bird_idle_0[20] = {0x1, 0xf0, 0x6, 0x28, 0x8, 0x44, 0x1c,  0x4a, 0x22, 0x22, 0x21, 0x1f, 0x11, 0x21, 0xe, 0x2f, 0x8, 0x21, 0x7, 0xfe};

unsigned char *bird_idle[1] = {&bird_idle_0};

unsigned char sad_face[8] = {0x3c, 0x42, 0xa5, 0xa5, 0x99, 0xa5, 0x42, 0x3c};

unsigned char neutral_face[8] = {0x3c, 0x42, 0xa5, 0xa5, 0x81, 0xbe, 0x42, 0x3c};

unsigned char happy_face[8] = {0x3c, 0x42, 0xa5, 0xa5, 0x81, 0xa5, 0x5a, 0x3c};

unsigned char *faces[3] = {&sad_face, &neutral_face, &happy_face};

unsigned char apple[8] = {0x8, 0x76, 0x55, 0x81, 0x81, 0x81, 0x42, 0x3c};

unsigned char burger[8] = {0x3c, 0x42, 0x81, 0xff, 0x42, 0xff, 0x81, 0x7e};

unsigned char *food[2] = {&apple, &burger};

unsigned char pipe_idle_0[255];

unsigned char *pipe_idle[1] = {&pipe_idle_0};

const int ledcols[] = {0, 0b10000000, 0b00100000, 0b01000000, 0b01100000, 0b11000000, 0b10100000, 0b11100000};

object_t obj[objects];

enum state {
	MENU = 0, BIRD = 1, SNAKE = 2
};

game_t games[games_n];

char *game_names[games_n] = {"", "FLAPPY BIRD", "TAMAGOCHI"};

enum state game_state;

unsigned char curgamesel;

unsigned char field[32][32];

int snake_len;

enum snakedir {
	UP, DOWN, LEFT, RIGHT
};

enum snakedir snake_dir;

long long snake_ticks;

long long snake_ticks_cur;

unsigned char fruit_x;

unsigned char fruit_y;

enum tamagochi_mood {
	SAD = 0, NEUTRAL = 1, HAPPY = 2
};

enum tamagochi_mood cur_mood;

unsigned char cur_food;

void timer_init(int div) {
	TCCR2A = 0;
	TCCR2B = 0;
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
}

void bird_update(object_t *bird) {
	if (bird->vy > -1.5) {
		bird->vy -= 0.04;
	}
	if ((btn && !prev_btn) || ((adcy >= 768) && (prev_adcy < 768))) {
		bird->vy = 20;
	}
	bird->y += bird->vy;
}

void pipe_update(object_t *pipe) {
	if (pipe->x < -10) {
		pipe->x = 130;
	}
	pipe->x -= 3;
}

void objs_update() {
	for (unsigned char i = 0; i < objects; ++i) {
		for (unsigned char j = 0; j < obj[i].ysz; ++j) {
			for (unsigned char k = 0; k < obj[i].xsz; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					if ((obj[i].idle[0][j*obj[i].xsz+k] & mask) && (k*8+bit+(int)obj[i].x >= 0) && (k*8+bit+(int)obj[i].x < 128) &&
							((int)obj[i].y+j < 128) && ((int)obj[i].y+j >= 0)) {
						ST7735_DrawPixel(&lcd, k*8+bit+(int)obj[i].x, (int)obj[i].y+j, 0);
					}
					mask *= 2;
				}
			}
		}
	}
	for (unsigned char i = 0; i < objects; ++i) {
		obj[i].update(&obj[i]);
		for (unsigned char j = 0; j < obj[i].ysz; ++j) {
			for (unsigned char k = 0; k < obj[i].xsz; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					if ((obj[i].idle[0][j*obj[i].xsz+k] & mask) && (k*8+bit+(int)obj[i].x >= 0) && (k*8+bit+(int)obj[i].x < 128) &&
							((int)obj[i].y+j < 128) && ((int)obj[i].y+j >= 0)) {
						ST7735_DrawPixel(&lcd, k*8+bit+(int)obj[i].x, (int)obj[i].y+j, 0xFFFF);
					}
					mask *= 2;
				}
			}
		}
	}
}

void bird_deinit() {
	for (int i = 0; i < objects; ++i) {
		obj[i].x = -100;
		obj[i].y = -100;
	}
	ST7735_ClearScreen (&lcd, BLACK);
}

void birdgame_update() {
	objs_update();
	if (((int)obj[1].x > 6 && (int)obj[1].x < 27 && ((int)obj[0].y <= 104 || (int)obj[0].y >= 24))
		       || (obj[0].y <= 5))	{
//		) {
		games[game_state].deinit();
		game_state = MENU;
		games[game_state].init();
	}
}

void init_obj(object_t *obj, unsigned char** idle, unsigned char** move,
	      unsigned int idle_frames, unsigned int move_frames,
	      unsigned char xsz, unsigned char ysz, double x,
	      double y, void (*upd)(object_t *)) {
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

void init_pipe_sprite(void) {
	for (int i = 0; i < 88; ++i) {
		pipe_idle_0[i] = 0x7e;
		pipe_idle_0[255-i] = 0x7e;
	}
}

void init_game(game_t *game, void (*init)(void), void (*update)(void), void (*deinit)(void)) {
	game->init = init;
	game->update = update;
	game->deinit = deinit;
}

void menu_init(void) {
	int pos = 10;
	curgamesel = 1;
	for (int i = 1; i < games_n; ++i) {
		ST7735_SetPosition(10, pos);
		ST7735_DrawString(&lcd, game_names[i], i == curgamesel ? RED : WHITE, X2);
		pos += 20;
	}
}

void menu_deinit() {	
	ST7735_ClearScreen (&lcd, BLACK);
}

void menu_update(void) {
	unsigned char cgs_prev = curgamesel;
	if (adcy >= 768 && prev_adcy < 768 && curgamesel < games_n-1) {
		curgamesel++;
	}
	else if (adcy <= 256 && prev_adcy > 256 && curgamesel > 1) {
		curgamesel--;
	} 
	if (curgamesel != cgs_prev) {
		ST7735_ClearScreen (&lcd, BLACK);
		int pos = 10;
		for (int i = 1; i < games_n; ++i) {
			ST7735_SetPosition(10, pos);
			ST7735_DrawString(&lcd, game_names[i], i == curgamesel ? RED : WHITE, X2);
			pos += 20;
		}
	}
	if (btn && !prev_btn) {
		menu_deinit();
		game_state = curgamesel;
		games[game_state].init();
	}
}

void bird_init() {
	init_obj(obj+0, bird_idle, bird_idle, 1, 1, 2, 10, 20, 20, &bird_update);
	init_obj(obj+1, pipe_idle, pipe_idle, 1, 1, 1, 255, -10, -64, &pipe_update);
	ST7735_ClearScreen (&lcd, BLACK);
}

void snake_init() {
	for (unsigned char i = 0; i < 32; ++i) {
		for (unsigned char j = 0; j < 32; ++j) {
			field[i][j] = 0;
		}
	}
	snake_ticks = 1024;
	snake_ticks_cur = 1024;
	snake_len = 1;
	snake_dir = RIGHT;
	fruit_x = 255;
	fruit_y = 255;
	ST7735_ClearScreen (&lcd, BLACK);	
}

void snake_update() {
/*	snake_ticks_cur--;
	if (adcy >= 768 && prev_adcy < 768) {
		snake_dir = UP;
	}
	else if (adcy <= 256 && prev_adcy > 256) {
		snake_dir = DOWN;
	}
	else if (adcx >= 768 && prev_adcx < 768) {
		snake_dir = RIGHT;
	}
	else if (adcx <= 256 && prev_adcx > 256) {
		snake_dir = LEFT;
	}
	if (snake_ticks_cur == 0) {
		if (fruit_x == 255 || fruit_y == 255) {
			int check;
			fruit_x = ticks & 63;
			fruit_y = (ticks & 504) >> 3;
			for (int i = 0; i < snake_len; ++i) {
				if (fruit_x == snake[i].x && snake[i].y == fruit_y) {
					check = 1;
					break;
				}
			}
			while (check) {
				check = 0;
				fruit_x = ticks & 63;
				fruit_y = (ticks & 504) >> 3;
				for (int i = 0; i < snake_len; ++i) {
					if (fruit_x == snake[i].x && snake[i].y == fruit_y) {
						check = 1;
						break;
					}
				}
			}
			ST7735_DrawRectangle(&lcd, fruit_x*2, fruit_x*2+1, fruit_y*2, fruit_y*2+1, WHITE);
		}
		if (snake[0].x == fruit_x && snake[0].y == fruit_y) {
			fruit_x = 255;
			fruit_y = 255;
			snake_ticks = snake_ticks*3/4;
		}
		ST7735_DrawRectangle(&lcd, snake[snake_len-1].x*2, snake[snake_len-1].x*2+1, snake[snake_len-1].y*2, snake[snake_len-1].y*2+1, BLACK);
		for (int i = snake_len-1; i > 0; i--) {
			if (snake[i].x == snake[0].x && snake[i].y == snake[0].y) {
				games[game_state].deinit();
				//game_state = 0;
				games[game_state].init();
			}
			snake[i].x = snake[i-1].x;
			snake[i].y = snake[i-1].y;
		}	
		switch (snake_dir) {
			case LEFT:
				snake[0].x++;
				break;
			case RIGHT:
				snake[0].x--;
				break;
			case UP:
				snake[0].y--;
				break;
			case DOWN:
				snake[0].y++;
				break;
		}
		ST7735_DrawRectangle(&lcd, snake[0].x*2, snake[0].x*2+1, snake[0].y*2, snake[0].y*2+1, WHITE);
		snake_ticks_cur = snake_ticks;
	} */
}

void tamagochi_init() {
	ST7735_ClearScreen (&lcd, BLACK);
	init_obj(obj+0, litvin_idle, litvin_idle, 1, 1, 1, 14, 100, 20, &litvin_update);
	cur_food = 0;
	ST7735_DrawLine(&lcd, 0, 128, 108, 108, WHITE);
	for (unsigned char j = 0; j < 8; ++j) {
		for (unsigned char k = 0; k < 1; ++k) {
			unsigned char mask = 1;
			for (unsigned char bit = 0; bit < 8; ++bit) {
				if (food[cur_food][j] & mask) {
					ST7735_DrawPixel(&lcd, 60+bit, 110+j, WHITE);
				}
				mask *= 2;
			}
		}
	}
	for (unsigned char j = 0; j < 8; ++j) {
		for (unsigned char k = 0; k < 1; ++k) {
			unsigned char mask = 1;
			for (unsigned char bit = 0; bit < 8; ++bit) {
				if (faces[cur_mood][j] & mask) {
					ST7735_DrawPixel(&lcd, 10+bit, 10+j, WHITE);
				}
				mask *= 2;
			}
		}
	}
}

void tamagochi_update() {
	int prev_food = cur_food;
	if (adcx >= 768 && prev_adcx < 768) {
		cur_food = (cur_food + 1) & 1;
	}
	else if (adcx <= 256 && prev_adcx > 256) {
		cur_food = (cur_food - 1) & 1;
	}
	if (prev_food != cur_food) {
		for (unsigned char j = 0; j < 8; ++j) {
			for (unsigned char k = 0; k < 1; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					if (food[prev_food][j] & mask) {
						ST7735_DrawPixel(&lcd, 60+bit, 110+j, BLACK);
					}
					mask *= 2;
				}
			}
		}
		for (unsigned char j = 0; j < 8; ++j) {
			for (unsigned char k = 0; k < 1; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					if (food[cur_food][j] & mask) {
						ST7735_DrawPixel(&lcd, 60+bit, 110+j, WHITE);
					}
					mask *= 2;
				}
			}
		}
	}
	enum tamagochi_mood prev_mood = cur_mood;
	if (btn && (cur_food == 1) && (cur_mood > 0)) {
		cur_mood--;
	}
	else if (btn && (cur_food == 0) && (cur_mood < 2)) {
		cur_mood++;
	}
	if (prev_mood != cur_mood) {
		for (unsigned char j = 0; j < 8; ++j) {
			for (unsigned char k = 0; k < 1; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					if (faces[prev_mood][j] & mask) {
						ST7735_DrawPixel(&lcd, 10+bit, 10+j, BLACK);
					}
					mask *= 2;
				}
			}
		}
		for (unsigned char j = 0; j < 8; ++j) {
			for (unsigned char k = 0; k < 1; ++k) {
				unsigned char mask = 1;
				for (unsigned char bit = 0; bit < 8; ++bit) {
					if (faces[cur_mood][j] & mask) {
						ST7735_DrawPixel(&lcd, 10+bit, 10+j, WHITE);
					}
					mask *= 2;
				}
			}
		}
	}
	objs_update();
}

void init(void) {
//	init_obj(obj+0, litvin_idle, litvin_idle, 1, 1, 1, 14, 20, 20, &bird_update);
	init_pipe_sprite(); 
	game_state = MENU;
	init_game(games+0, &menu_init, &menu_update, &menu_deinit);
	init_game(games+1, &bird_init, &birdgame_update, &bird_deinit);
	init_game(games+2, &tamagochi_init, &tamagochi_update, &bird_deinit);
//	init_obj(obj+1, pipe_idle, pipe_idle, 1, 1, 1, 256, -10, 0, &pipe_update);
//	init_obj(obj+0, &bird_idle, &bird_idle, 1, 1, 2, 12, 20, 105, &bird_update);
/*	nokia_lcd_init();
	nokia_lcd_clear();
	nokia_lcd_write_string("HELLO WORLD", 1);
	nokia_lcd_render(); */
	ST7735_Init(&lcd);
	ST7735_ClearScreen (&lcd, BLACK);
	games[game_state].init();
}

void update(void) {
	games[game_state].update();
}
//[j*obj[i].xsz+k]
