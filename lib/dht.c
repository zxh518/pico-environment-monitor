#include "dht.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include <stdio.h>

#define DHT_MIN_INTERVAL_MS 2000

// Busy-wait until pin reads `level`. Returns elapsed microseconds, or -1 on timeout.
static int wait_level(uint pin, bool level, uint32_t timeout_us) {
    uint32_t start = time_us_32();
    while (gpio_get(pin) != level) {
        if ((time_us_32() - start) > timeout_us) return -1;
    }
    return (int)(time_us_32() - start);
}

const char *dht_result_str(dht_result r) {
    switch (r) {
        case DHT_OK:               return "ok";
        case DHT_ERR_NO_RESPONSE:  return "no response: line never pulled low (wiring, power or pull-up)";
        case DHT_ERR_RESPONSE_HIGH:return "sensor held low, never released";
        case DHT_ERR_DATA_START:   return "no start of data after response pulse";
        case DHT_ERR_BIT_TIMEOUT:  return "timed out mid-transfer";
        case DHT_ERR_CHECKSUM:     return "checksum mismatch";
        default:                   return "unknown";
    }
}

dht_result read_dht(uint data_pin, dht_reading *result) {
    static bool have_read = false;
    static uint32_t last_ms = 0;

    result->temperature = 0.0f;
    result->humidity = 0.0f;

    // DHT22 sampling period is 2 s. Reading faster returns stale or no data.
    if (have_read) {
        uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - last_ms;
        if (elapsed < DHT_MIN_INTERVAL_MS) sleep_ms(DHT_MIN_INTERVAL_MS - elapsed);
    }

    uint8_t data[5] = {0};
    dht_result status = DHT_OK;

    gpio_init(data_pin);
    gpio_put(data_pin, 0);              // preload the output latch before driving
    gpio_set_dir(data_pin, GPIO_OUT);   // start pulse: host holds the line low
    sleep_ms(2);                        // DHT22 needs >= 1 ms (DHT11 needs 18 ms)

    // USB and timer interrupts will corrupt these microsecond-scale measurements.
    uint32_t ints = save_and_disable_interrupts();

    gpio_set_dir(data_pin, GPIO_IN);
    gpio_pull_up(data_pin);

    // Sensor answers 20-40 us after release: 80 us low, then 80 us high.
    if      (wait_level(data_pin, 0, 300) < 0) status = DHT_ERR_NO_RESPONSE;
    else if (wait_level(data_pin, 1, 150) < 0) status = DHT_ERR_RESPONSE_HIGH;
    else if (wait_level(data_pin, 0, 150) < 0) status = DHT_ERR_DATA_START;

    if (status == DHT_OK) {
        for (int i = 0; i < 40; i++) {
            // Each bit: 50 us low, then 26-28 us high for 0 or ~70 us high for 1.
            if (wait_level(data_pin, 1, 100) < 0) { status = DHT_ERR_BIT_TIMEOUT; break; }
            int high_us = wait_level(data_pin, 0, 150);
            if (high_us < 0) { status = DHT_ERR_BIT_TIMEOUT; break; }
            if (high_us > 45) data[i / 8] |= (uint8_t)(1u << (7 - (i % 8)));
        }
    }

    restore_interrupts(ints);

    last_ms = to_ms_since_boot(get_absolute_time());
    have_read = true;

    if (status != DHT_OK) return status;

    if (data[4] != (uint8_t)(data[0] + data[1] + data[2] + data[3])) return DHT_ERR_CHECKSUM;

    result->humidity    = (float)(((uint16_t)data[0] << 8) | data[1]) / 10.0f;
    result->temperature = (float)((((uint16_t)(data[2] & 0x7F)) << 8) | data[3]) / 10.0f;
    if (data[2] & 0x80) result->temperature = -result->temperature;

    return DHT_OK;
}

void dht_line_test(uint data_pin) {
    gpio_init(data_pin);
    gpio_set_dir(data_pin, GPIO_IN);
    gpio_pull_up(data_pin);
    sleep_ms(10);
    int idle = gpio_get(data_pin);
    printf("idle line level = %d (expect 1)\n", idle);
    if (idle == 0) {
        printf("  line is stuck low: data pin shorted to ground, or sensor is miswired\n");
        return;
    }

    gpio_put(data_pin, 0);
    gpio_set_dir(data_pin, GPIO_OUT);
    sleep_ms(20);

    uint32_t ints = save_and_disable_interrupts();
    gpio_set_dir(data_pin, GPIO_IN);
    gpio_pull_up(data_pin);
    int t = wait_level(data_pin, 0, 5000);
    restore_interrupts(ints);

    if (t < 0) printf("  no answer within 5 ms: sensor is not driving the line at all\n");
    else       printf("  sensor answered after %d us (expect 20-40)\n", t);
}