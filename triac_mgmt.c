#include "triac_mgmt.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"


void __triac_int_a(void)
{
    uint32_t status = TimerIntStatus(TIMER4_BASE, true);
    TimerIntClear(TIMER4_BASE, status);

	trigger_triac(&triac_map[0]);
}

void __triac_int_b(void)
{
    uint32_t status = TimerIntStatus(TIMER4_BASE, true);
    TimerIntClear(TIMER4_BASE, status);

	trigger_triac(&triac_map[1]);
}

void init_triac(volatile TRIAC_T * triac)
{
	// Set TRIAC off and configure as output
	GPIOPinWrite(triac->triac_port, triac->triac_pin, 0);
	GPIOPinTypeGPIOOutput(triac->triac_port, triac->triac_pin);

	// default timer to 0 so it doesn't count
	// HWREG(triac->delay_config) = 0;
}

void trigger_triac(volatile TRIAC_T * triac)
{
	// Pulse Triac
	GPIOPinWrite(triac->triac_port, triac->triac_pin, triac->triac_pin);
	SysCtlDelay(400);	// Delay 100 Clock Cycles
	GPIOPinWrite(triac->triac_port, triac->triac_pin, 0);
}

void triac_delay(volatile TRIAC_T * triac)
{
	TimerLoadSet(triac->timer_base, triac->timer_chan, 22241);
	TimerEnable(triac->timer_base, triac->timer_chan);
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

void trigger_triac_timers(void)
{
	TimerEnable(TIMER4_BASE, TIMER_BOTH);
}
