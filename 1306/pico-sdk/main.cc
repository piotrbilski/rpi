#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

extern "C" const uint8_t ssd1306_font6x8[];

//# MicroPython SSD1306 OLED driver, I2C and SPI interfaces

//from micropython import const
//import framebuf


#define SET_CONTRAST 0x81
#define SET_ENTIRE_ON 0xA4
#define SET_NORM_INV 0xA6
#define SET_DISP 0xAE
#define SET_MEM_ADDR 0x20
#define SET_COL_ADDR 0x21
#define SET_PAGE_ADDR 0x22
#define SET_DISP_START_LINE 0x40
#define SET_SEG_REMAP 0xA0
#define SET_MUX_RATIO 0xA8
#define SET_COM_OUT_DIR 0xC0
#define SET_DISP_OFFSET 0xD3
#define SET_COM_PIN_CFG 0xDA
#define SET_DISP_CLK_DIV 0xD5
#define SET_PRECHARGE 0xD9
#define SET_VCOM_DESEL 0xDB
#define SET_CHARGE_PUMP 0x8D

typedef uint8_t u8;

#define I2C_PORT i2c0
#define SID 0x3C // I2C 1306 slave ID ... for 64 height display. 32 height is 0x3D

const u8 height = 64;
const u8 width = 128;
const int pages = height / 8;
const bool external_vcc = false;


u8 scr[pages*width+1]; // extra byte holds data send instruction

void write_cmd(u8 cmd);

void fill_scr(u8 v)
{
	memset(scr, v, sizeof(scr));
}


void send2(u8 v1, u8 v2)
{
	u8 buf[2];
	buf[0] = v1;
	buf[1] = v2;
	i2c_write_blocking(I2C_PORT, SID, buf, 2, false);
}

/* Original Python
   def show(self):
   x0 = 0
   x1 = self.width - 1
   if self.width == 64:
# displays with width of 64 pixels are shifted by 32
x0 += 32
x1 += 32
self.write_cmd(SET_COL_ADDR)
self.write_cmd(x0)
self.write_cmd(x1)
self.write_cmd(SET_PAGE_ADDR)
self.write_cmd(0)
self.write_cmd(self.pages - 1)
self.write_data(self.buffer)
*/

void show_scr()
{
#if 0
	int x0 = 0;
	int x1 = width -1;
	if(width == 64) { x0 += 32; x1 += 32; } // displays of width 64 are shifted by 32
	write_cmd(SET_COL_ADDR); // 0x21
	write_cmd(x0);
	write_cmd(x1);
#endif

	write_cmd(SET_MEM_ADDR); // 0x20
	write_cmd(0b01); // vertical addressing mode

	write_cmd(SET_COL_ADDR); // 0x21
	write_cmd(0);
	write_cmd(127);

	write_cmd(SET_PAGE_ADDR); // 0x22
	write_cmd(0);
	write_cmd(7);


	scr[0] = 0x40; // the data instruction	
	i2c_write_blocking(I2C_PORT, SID, scr, sizeof(scr), false);
}



void write_cmd(u8 cmd) 
{ 
	send2(0x80, cmd);
#if 0
	u8 buf[2];
	buf[0] = 0x80;
	buf[1] = cmd;
	i2c_write_blocking(I2C_PORT, SID, buf, 2, false);
	//send2(0x80, cmd); 
#endif
}


#if 0
/* Original Python code:
   def write_cmd(self, cmd):
   self.temp[0] = 0x80  # Co=1, D/C#=0
   self.temp[1] = cmd
   self.i2c.writeto(self.addr, self.temp)
   */
void write_cmds(u8* cmds, int n)
{
	for(int i=0; i<n; i++)
		write_cmd(cmds[i]);
}
#endif

void poweroff() { write_cmd(SET_DISP | 0x00); }

void poweron() { write_cmd(SET_DISP | 0x01); }

void contrast(u8 contrast) { write_cmd(SET_CONTRAST); write_cmd(contrast); }

void invert(u8 invert) { write_cmd(SET_NORM_INV | (invert & 1)); }


void init_display()
{
	u8 cmds[] = {
		SET_DISP | 0x00,  // display off 0x0E | 0x00

		SET_MEM_ADDR, // 0x20
		0x00,  // horizontal

		//# resolution and layout
		SET_DISP_START_LINE | 0x00, // 0x40
		SET_SEG_REMAP | 0x01,  //# column addr 127 mapped to SEG0

		SET_MUX_RATIO, // 0xA8
		height - 1,

		SET_COM_OUT_DIR | 0x08,  //# scan from COM[N] to COM0  (0xC0 | val)
		SET_DISP_OFFSET, // 0xD3
		0x00,

		//SET_COM_PIN_CFG, // 0xDA
		//0x02 if self.width > 2 * self.height else 0x12,
		//width > 2*height ? 0x02 : 0x12,
		//SET_COM_PIN_CFG, height == 32 ? 0x02 : 0x12,

		//# timing and driving scheme
		SET_DISP_CLK_DIV, // 0xD5
		0x80,

		SET_PRECHARGE,
		//0x22 if self.external_vcc else 0xF1,
		external_vcc ? 0x22 : 0xF1,

		SET_VCOM_DESEL,
		//0x30,  //# 0.83*Vcc
		0x40, // changed by mcarter

		//# display
		SET_CONTRAST, // 0x81
		0xFF,  //# maximum

		SET_ENTIRE_ON,  //# output follows RAM contents // 0xA4
		SET_NORM_INV,  //# not inverted 0xA6

		SET_CHARGE_PUMP, // 0x8D
		//0x10 if self.external_vcc else 0x14,
		external_vcc ? 0x10 : 0x14,

		SET_DISP | 0x01
	};

	// write all the commands
	for(int i=0; i<sizeof(cmds); i++)
		write_cmd(cmds[i]);
	//write_cmds(cmds, sizeof(cmds));
	fill_scr(0);
	show_scr();
}

/*
# Subclassing FrameBuffer provides support for graphics primitives
# http://docs.micropython.org/en/latest/pyboard/library/framebuf.html
class SSD1306(framebuf.FrameBuffer):
def __init__(self, width, height, external_vcc):
self.width = width
self.height = height
self.external_vcc = external_vcc
self.pages = self.height // 8
self.buffer = bytearray(self.pages * self.width)
super().__init__(self.buffer, self.width, self.height, framebuf.MONO_VLSB)
self.init_display()


class SSD1306_I2C(SSD1306):
def __init__(self, width, height, i2c, addr=0x3C, external_vcc=False):
self.i2c = i2c
self.addr = addr
self.temp = bytearray(2)
self.write_list = [b"\x40", None]  # Co=0, D/C#=1
super().__init__(width, height, external_vcc)

def write_data(self, buf):
self.write_list[1] = buf
self.i2c.writevto(self.addr, self.write_list)
*/

void init_i2c()
{
	// This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL)
	i2c_init(I2C_PORT, 100 * 1000);
	gpio_set_function(4, GPIO_FUNC_I2C);
	gpio_set_function(5, GPIO_FUNC_I2C);
	gpio_pull_up(4);
	gpio_pull_up(5);
}

void draw_pixel(int16_t x, int16_t y, int color) 
{
	if(x<0 || x >= width || y<0 || y>= height) return;

	int page = y/8;
	int bit = 1<<(y % 8);
	u8* ptr = scr + x*8 + page  + 1; 

	switch (color) {
		case 1: // white
			*ptr |= bit;
			break;
		case 0: // black
			*ptr &= ~bit;
			break;
		case -1: //inverse
			*ptr ^= bit;
			break;
	}

}



void draw_letter_at(u8 x, u8 y, char c)
{
	//char c = 'C';
	//c= 'Q';
	if(c< ' ' || c>  0x7F) c = '?'; // 0x7F is the DEL key

	int offset = 4 + (c - ' ' )*6;
	for(int col = 0 ; col < 6; col++) {
		u8 line =  ssd1306_font6x8[offset+col];
		for(int row =0; row <8; row++) {
			//char x = (line & (1<<7)) ? 'X' : '_';
			draw_pixel(x+col, y+row, line & 1);
			//cout << x;
			line >>= 1;
		}
		//cout << '\n';
	}

	//for(int row = 6; row<8; col++) {
	for(int row = 0; row<8; row++) {
		draw_pixel(x+6, y+row, 0);
		draw_pixel(x+7, y+row, 0);
	}
	//cout << "________\n";

}


void draw_letter(char c) { draw_letter_at(0, 0, c); }
void pixel(int x, int y)
{
	//int x = 100, y=50;
	int page = y/8;
	u8 patt = 1<<(y%8);
	//scr[1+ 0] |= 0xff;
	scr[1+ x*8 + page] |= patt;

}
void once()
{
	fill_scr(0);
	draw_letter_at(0,0, 'P');
	draw_letter_at(60,0, 'M');
	draw_letter_at(60,30, 'C');
#if 0
	for(int x = 0; x< 10; x++) 
		for(int y = 0; y< 30; y++) 
			draw_pixel(y+1, x+1, 1);
#endif
	for(int i = 0; i< 60; i++) pixel(i, i);
	show_scr();
	sleep_ms(500);
}

int cursorx = 0, cursory = 0;
void ssd1306_print(const char* str)
{
	char c;
	while(c = *str) {
		str++;
		if(c == '\n') {
			cursorx = 0;
			cursory += 8;
			continue;
		}
		draw_letter_at(cursorx, cursory, c);
		cursorx += 8;
	}
}



int main()
{
	init_i2c();
	init_display();
	//once();
	if(0) {
		for(int i = 0; i<64; i++) {
			draw_pixel(i, i, 1);
			draw_pixel(i, 63-i, 1);
		}
		show_scr();
	}

	ssd1306_print("HELLO PICO\n");
	ssd1306_print("OLED 128x64 demo\n");
	ssd1306_print("Written in  C++");
	show_scr();

	int update = 0;
	for(int y = 32; y < 64; y++) {
		for(int x = 0; x < 128; x++) {
			draw_pixel(x, y, 1);
			if((update++ % 16) == 0)show_scr();
			//sleep_ms(5);
		}
	}
	show_scr();

	for(;;);
	return 0;
}
