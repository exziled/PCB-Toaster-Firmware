#ifndef __TRIAC_H
#define __TRIAC_H

#include <stdint.h>

#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"


typedef struct triac_ctrl
{
	uint32_t triac_port;
	uint8_t triac_pin;
	uint32_t timer_base;
	uint32_t timer_chan;
} TRIAC_T;

static volatile TRIAC_T triac_map[] =
{
	{GPIO_PORTE_BASE, GPIO_PIN_0, TIMER4_BASE, TIMER_A},		// Mapped to Timer 4 A Channel
	{GPIO_PORTE_BASE, GPIO_PIN_1, TIMER4_BASE, TIMER_B}			// Mapped to Timer 4 B Channel
};

void init_triac(volatile TRIAC_T * triac);
void trigger_triac(volatile TRIAC_T * triac);

void init_triac_timer(void);
void trigger_triac_timers(void);
void triac_delay(volatile TRIAC_T * triac);

#endif