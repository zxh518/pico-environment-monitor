#include "lcd_i2c.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

// PCF8574 backpack bit map: P0=RS, P1=RW, P2=E, P3=backlight, P4-P7=D4-D7
#define LCD_RS        0x01
#define LCD_ENABLE    0x04
#define LCD_BACKLIGHT 0x08

static i2c_inst_t *bus;
static uint8_t addr;
static uint8_t bl = LCD_BACKLIGHT;

static void write_raw(uint8_t val) {
    i2c_write_blocking(bus, addr, &val, 1, false);
}

// The HD44780 latches the data bus on the falling edge of E.
static void send_nibble(uint8_t high_nibble, uint8_t mode) {
    uint8_t val = (high_nibble & 0xF0) | mode | bl;
    write_raw(val | LCD_ENABLE);
    sleep_us(1);
    write_raw(val & ~LCD_ENABLE);
    sleep_us(50);
}

static void lcd_send(uint8_t value, uint8_t mode) {
    send_nibble(value & 0xF0, mode);
    send_nibble((uint8_t)(value << 4), mode);
}

static void lcd_command(uint8_t cmd) { lcd_send(cmd, 0); }

uint8_t lcd_bus_scan(i2c_inst_t *i2c) {
    uint8_t first = 0;
    printf("Scanning I2C bus...\n");
    for (uint8_t a = 0x08; a < 0x78; a++) {
        uint8_t dummy;
        if (i2c_read_blocking(i2c, a, &dummy, 1, false) >= 0) {
            printf("  device at 0x%02X\n", a);
            if (!first) first = a;
        }
    }
    if (!first) printf("  nothing found: check SDA/SCL, power and ground\n");
    return first;
}

bool lcd_init(i2c_inst_t *i2c, uint8_t address) {
    bus = i2c;

    if (address == 0) {
        address = lcd_bus_scan(i2c);
        if (address == 0) return false;
        printf("Using LCD at 0x%02X\n", address);
    }
    addr = address;

    sleep_ms(50);   // controller needs time after power-up

    // Wake-up sequence: three 8-bit function sets, then drop to 4-bit mode.
    send_nibble(0x30, 0); sleep_ms(5);
    send_nibble(0x30, 0); sleep_us(150);
    send_nibble(0x30, 0); sleep_us(150);
    send_nibble(0x20, 0); sleep_us(150);

    lcd_command(0x28);              // 4-bit, 2 lines, 5x8 font
    lcd_command(0x08);              // display off
    lcd_command(0x01); sleep_ms(2); // clear
    lcd_command(0x06);              // entry mode: increment, no shift
    lcd_command(0x0C);              // display on, cursor off, no blink

    return true;
}

void lcd_clear(void) {
    lcd_command(0x01);
    sleep_ms(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    static const uint8_t row_offset[4] = {0x00, 0x40, 0x14, 0x54};
    if (row > 3) row = 3;
    lcd_command((uint8_t)(0x80 | (row_offset[row] + col)));
}

void lcd_string(const char *s) {
    while (*s) lcd_send((uint8_t)*s++, LCD_RS);
}

void lcd_backlight(bool on) {
    bl = on ? LCD_BACKLIGHT : 0x00;
    write_raw(bl);
}