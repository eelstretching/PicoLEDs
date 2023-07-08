#ifndef DATA_H
#define DATA_H

#include "pico/util/datetime.h"

typedef struct Data_T {
    datetime_t dt;
    uint8_t pop;
    uint8_t high;
    uint8_t low;
    char sunrise[5];
    char sunset[5];
} data_t;

data_t *fetch_data();

#endif