#include "triac_mgmt.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"

#include "utils/uartstdio.h"

extern volatile TRIAC_T triac_map[];

// Interrupts
void __triac_int_a(void)
{
    uint32_t status = TimerIntStatus(TIMER4_BASE, true);
    TimerIntClear(TIMER4_BASE, status);

	trigger_triac(0);
}

void __triac_int_b(void)
{
    uint32_t status = TimerIntStatus(TIMER4_BASE, true);
    TimerIntClear(TIMER4_BASE, status);

	trigger_triac(1);
}

// Initializers
void init_triac(uint8_t triac_index)
{
	// Set TRIAC off and configure as output
	GPIOPinWrite(triac_map[triac_index].triac_port, triac_map[triac_index].triac_pin, 0);
	GPIOPinTypeGPIOOutput(triac_map[triac_index].triac_port, triac_map[triac_index].triac_pin);

	// default timer to 0 so it doesn't count
	// HWREG(triac_map[triac_index].delay_config) = 0;
}

void init_triac_timer(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
	SysCtlDelay(3);
	SysCtlAltClkConfig(SYSCTL_ALTCLK_PIOSC);		// Use 16MHz Internal Oscilator... i'm calling it to call it bro

	TimerConfigure(TIMER4_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_B_ONE_SHOT);

	TimerPrescaleSet(TIMER4_BASE, TIMER_A, 21);
	TimerPrescaleSet(TIMER4_BASE, TIMER_B, 21);

	TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);		// Overflow
	TimerIntEnable(TIMER4_BASE, TIMER_TIMB_TIMEOUT);

	TimerIntRegister(TIMER4_BASE, TIMER_A, __triac_int_a);
	TimerIntRegister(TIMER4_BASE, TIMER_B, __triac_int_b);
}

// Modifiers

// Do a TRIAC Pulse
void trigger_triac(uint8_t triac_index)
{
	// Pulse Triac
	GPIOPinWrite(triac_map[triac_index].triac_port, triac_map[triac_index].triac_pin, triac_map[triac_index].triac_pin);
	SysCtlDelay(400);	// Delay 100 Clock Cycles
	GPIOPinWrite(triac_map[triac_index].triac_port, triac_map[triac_index].triac_pin, 0);
}

// Setup TRIAC Timers
void triac_delay(uint8_t triac_index)
{
	// UARTprintf("Setting Triac with DC: %d", triac_map[triac_index].duty_cycle);

	// Map 0-100% duty cycle to relative timer values
	uint16_t timer_val = (triac_map[triac_index].duty_cycle - 100) * (TIMER_MAX - TIMER_MIN) / (100 - 0);

	TimerLoadSet(triac_map[triac_index].timer_base, triac_map[triac_index].timer_chan, timer_val);
	TimerEnable(triac_map[triac_index].timer_base, triac_map[triac_index].timer_chan);
}


// Not Used
void trigger_triac_timers(void)
{
	TimerEnable(TIMER4_BASE, TIMER_BOTH);
}

void triac_set_duty(uint8_t triac_index, uint8_t duty)
{
	triac_map[triac_index].duty_cycle = duty;
}