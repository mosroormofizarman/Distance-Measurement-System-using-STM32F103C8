#include "i2c-lcd.h"
#include "main.h"
#include <string.h>

I2C_HandleTypeDef *_hi2c;

void lcd_send_four_bits(unsigned char nibble)
{
    unsigned char data = (nibble << 4) | 0x0F;
    HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, &data, 1, 100);
    
    HAL_Delay(1);
    
    data = (nibble << 4) | 0x0B;
    HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, &data, 1, 100);
    
    HAL_Delay(1);
}

void lcd_send_cmd(unsigned char cmd)
{
    unsigned char upper_nibble = cmd & 0xF0;
    unsigned char lower_nibble = (cmd << 4) & 0xF0;
    
    lcd_send_four_bits(upper_nibble >> 4);
    lcd_send_four_bits(lower_nibble >> 4);
    
    HAL_Delay(2);
}

void lcd_send_data(unsigned char data)
{
    unsigned char upper_nibble = data & 0xF0;
    unsigned char lower_nibble = (data << 4) & 0xF0;
    
    unsigned char byte;
    
    byte = upper_nibble | 0x05;
    HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, &byte, 1, 100);
    HAL_Delay(1);
    
    byte = upper_nibble | 0x01;
    HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, &byte, 1, 100);
    HAL_Delay(1);
    
    byte = lower_nibble | 0x05;
    HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, &byte, 1, 100);
    HAL_Delay(1);
    
    byte = lower_nibble | 0x01;
    HAL_I2C_Master_Transmit(_hi2c, I2C_ADDR << 1, &byte, 1, 100);
    HAL_Delay(1);
}

void lcd_init(I2C_HandleTypeDef *hi2c)
{
    _hi2c = hi2c;
    
    HAL_Delay(50);
    
    lcd_send_cmd(0x33);
    HAL_Delay(5);
    
    lcd_send_cmd(0x32);
    HAL_Delay(5);
    
    lcd_send_cmd(0x28);
    HAL_Delay(2);
    
    lcd_send_cmd(0x0C);
    HAL_Delay(2);
    
    lcd_send_cmd(0x06);
    HAL_Delay(2);
    
    lcd_clear();
}

void lcd_clear(void)
{
    lcd_send_cmd(0x01);
    HAL_Delay(2);
}

void lcd_put_cur(unsigned char row, unsigned char col)
{
    unsigned char position;
    
    if (row == 0)
        position = 0x80 + col;
    else
        position = 0xC0 + col;
    
    lcd_send_cmd(position);
}

void lcd_send_string(char *str)
{
    while (*str)
    {
        lcd_send_data(*str++);
    }
}

void lcd_print_char(char c)
{
    lcd_send_data(c);
}