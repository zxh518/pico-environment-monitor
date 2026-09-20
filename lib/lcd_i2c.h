#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Prints every responding address on the bus. Returns the first one found,
// or 0 if the bus is silent.
uint8_t lcd_bus_scan(i2c_inst_t *i2c);

// addr is typically 0x27 or 0x3F. Pass 0 to auto-detect the first device.
bool lcd_init(i2c_inst_t *i2c, uint8_t addr);

void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_string(const char *s);
void lcd_backlight(bool on);

#endif