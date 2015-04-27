#ifndef __TIME_H
#define __TIME_H

#include <stdint.h>

static uint32_t system_timestamp;

void timestamp_reset(void);
uint32_t timestamp_get(void);
void timestamp_increment(void);


#endif