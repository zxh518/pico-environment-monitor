#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "stdio.h"
#include "dht.h"
#include "lcd_i2c.h"
#include "pico/stdio_usb.h"

#define LED_NORMAL 15
#define LED_ALERT 14
#define DHT_PIN 2

#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

#define TEMP_THRESHOLD 33.0f
#define HUMIDITY_THRESHOLD 72.0f

int main() {
    stdio_init_all();
    for (int i = 0; i < 100 && !stdio_usb_connected(); i++) sleep_ms(100);
    sleep_ms(500);

    gpio_init(LED_NORMAL);
    gpio_init(LED_ALERT);
    gpio_set_dir(LED_NORMAL, GPIO_OUT);
    gpio_set_dir(LED_ALERT, GPIO_OUT);

    // 100 kHz is the safe default. Long jumper leads will not tolerate 400 kHz.
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    bool lcd_ok = lcd_init(I2C_PORT, 0);   // 0 means auto-detect the address
    if (lcd_ok) {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_string("Env monitor");
        lcd_set_cursor(1, 0);
        lcd_string("starting...");
    } else {
        printf("LCD not found, continuing without it\n");
    }

    printf("--- DHT line test ---\n");
    dht_line_test(DHT_PIN);
    sleep_ms(2000);
    printf("---------------------\n");
    printf("---------------------\n");

    dht_reading reading;
    bool have_good = false;
    char line[17];

    while (true) {
        dht_result r = read_dht(DHT_PIN, &reading);

        if (r == DHT_OK) {
            have_good = true;
            printf("Temperature: %.1f C, Humidity: %.1f%%\n",
                   reading.temperature, reading.humidity);
            if (lcd_ok) {
                snprintf(line, sizeof(line), "Temp: %.1f C    ", reading.temperature);
                lcd_set_cursor(0, 0);
                lcd_string(line);
                snprintf(line, sizeof(line), "Hum:  %.1f %%    ", reading.humidity);
                lcd_set_cursor(1, 0);
                lcd_string(line);
            }
        } else {
            printf("Read failed (%s)\n", dht_result_str(r));
            if (lcd_ok && !have_good) {
                lcd_set_cursor(0, 0);
                lcd_string("Sensor error    ");
                lcd_set_cursor(1, 0);
                lcd_string("                ");
            }
        }

        bool alert = have_good && (reading.temperature > TEMP_THRESHOLD ||
                                   reading.humidity > HUMIDITY_THRESHOLD);

        if (!have_good) {
            gpio_put(LED_NORMAL, 0);
            gpio_put(LED_ALERT, 0);
            sleep_ms(2000);
        } else if (alert) {
            gpio_put(LED_NORMAL, 0);
            for (int i = 0; i < 5; i++) {
                gpio_put(LED_ALERT, 1);
                sleep_ms(200);
                gpio_put(LED_ALERT, 0);
                sleep_ms(200);
            }
        } else {
            gpio_put(LED_NORMAL, 1);
            gpio_put(LED_ALERT, 0);
            sleep_ms(2000);
        }
    }
}