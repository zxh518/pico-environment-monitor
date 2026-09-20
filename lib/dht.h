#ifndef DHT_H
#define DHT_H

#include "pico/stdlib.h"

typedef struct {
    float humidity;
    float temperature;
} dht_reading;

typedef enum {
    DHT_OK = 0,
    DHT_ERR_NO_RESPONSE,
    DHT_ERR_RESPONSE_HIGH,
    DHT_ERR_DATA_START,
    DHT_ERR_BIT_TIMEOUT,
    DHT_ERR_CHECKSUM
} dht_result;

dht_result read_dht(uint data_pin, dht_reading *result);
const char *dht_result_str(dht_result r);

// Pulses the line and reports whether the sensor answers at all.
void dht_line_test(uint data_pin);

#endif