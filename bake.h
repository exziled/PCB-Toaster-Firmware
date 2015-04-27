#ifndef __BAKE_H
#define __BAKE_H

#include <stdint.h>
#include <stdlib.h>

typedef struct profile_params
{
	uint16_t timestamp;				// Time since reflow start
	uint16_t temperature;		// Temperature in C
} REFLOW_PROFILE_T;

typedef struct reflow_instance
{
	uint16_t start_time;
	REFLOW_PROFILE_T * profile;
	uint8_t profile_points;
	uint8_t active;
} REFLOW_INSTANCE_T;

volatile REFLOW_INSTANCE_T * reflow_get_instance(void);
void reflow_init(volatile REFLOW_INSTANCE_T * instance);
void reflow_start(volatile REFLOW_INSTANCE_T * instance);
uint16_t reflow_calc_temp(volatile REFLOW_INSTANCE_T * instance);

#endif