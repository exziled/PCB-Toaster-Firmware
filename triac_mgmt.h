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
	uint8_t duty_cycle;
} TRIAC_T;

#define TIMER_MAX 1
#define TIMER_MIN 44000

static volatile TRIAC_T triac_map[] =
{
	{GPIO_PORTE_BASE, GPIO_PIN_0, TIMER4_BASE, TIMER_A, 100},		// Mapped to Timer 4 A Channel
	{GPIO_PORTE_BASE, GPIO_PIN_1, TIMER4_BASE, TIMER_B, 100}			// Mapped to Timer 4 B Channel
};

void init_triac(volatile TRIAC_T * triac);
void trigger_triac(volatile TRIAC_T * triac);

void init_triac_timer(void);
void trigger_triac_timers(void);
void triac_delay(volatile TRIAC_T * triac);

#endif