#include "zero_crossing.h"

#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

#include "triac_mgmt.h"

void __int_ZERO(void)
{
    uint32_t status = GPIOIntStatus(GPIO_PORTN_BASE, true);
    GPIOIntClear(GPIO_PORTN_BASE, status);

    // MAP_GPIOIntClear(GPIO_PORTN_BASE, GPIO_INT_PIN_4);

    //
    // Toggle the GPIO
    //
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,
            (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0) ^
             GPIO_PIN_0));

    triac_delay(&triac_map[0]);
    triac_delay(&triac_map[1]);
    //trigger_triac_timers();

    return;
}

void init_zero_crossing(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlDelay(3);

	// Configure GPIO for Interrupts
	MAP_GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_4);
	MAP_GPIOIntTypeSet(GPIO_PORTN_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);
	GPIOIntRegister(GPIO_PORTN_BASE, __int_ZERO);
	MAP_GPIOIntEnable(GPIO_PORTN_BASE, GPIO_INT_PIN_4);
}