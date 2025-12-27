#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "main.h"

#define I2C_ADDR 0x27
#define ROWS 2
#define COLS 16

#define LCD_CHR 1
#define LCD_CMD 0

#define LINE1 0x80
#define LINE2 0xC0

#define LCDCLR 0x01
#define CURRHOME 0x02
#define ENTRYMODE 0x04
#define DISPCTRL 0x08
#define CURSORSHIFT 0x10
#define FUNCTIONSET 0x20
#define SETCGRAMADDR 0x40
#define SETDDRAMADDR 0x80

#define ENTRY_SH 0x01
#define ENTRY_ID 0x02

#define DISP_BLINK 0x01
#define DISP_CURSOR 0x02
#define DISP_ON 0x04

#define SHIFT_RL 0x04
#define SHIFT_SC 0x08

#define FSET_5x10DOTS 0x04
#define FSET_2LINE 0x08
#define FSET_8BIT 0x10

#define BACKLIGHT_PIN 3

void lcd_init(I2C_HandleTypeDef *hi2c);
void lcd_send_cmd(unsigned char cmd);
void lcd_send_data(unsigned char data);
void lcd_send_string(char *str);
void lcd_put_cur(unsigned char row, unsigned char col);
void lcd_clear(void);
void lcd_print_char(char c);

#endif