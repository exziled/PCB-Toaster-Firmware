#include "max31855.h"

#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"

void initToasterTemp(void)
{

	// Config SPI Clock
	// Assuming on SSI2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlDelay(3);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	SysCtlDelay(3);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
	SysCtlDelay(3);

	GPIOPinConfigure(GPIO_PD3_SSI2CLK);
	GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
	GPIOPinConfigure(GPIO_PD1_SSI2XDAT0);
	GPIOPinConfigure(GPIO_PD2_SSI2FSS);

	GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
	
	// Setup Chip Select
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_2);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, GPIO_PIN_2);

	SSIConfigSetExpClk(SSI2_BASE, 120000000, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 16);

	SSIEnable(SSI2_BASE);
}

uint16_t getToasterTemp(void)
{
	uint32_t output = 0;
	uint32_t SSIBuff = 0;

	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, 0);

	// Have to put dummy data to actually recieve data...
	SSIDataPut(SSI2_BASE, 0);
	SSIDataGet(SSI2_BASE, &SSIBuff);
	output = ((uint32_t) SSIBuff) << 16;
	SSIDataPut(SSI2_BASE, 0);
	SSIDataGet(SSI2_BASE, &SSIBuff);
	output |= SSIBuff;

	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, GPIO_PIN_2);

	// Detecting thermocouple errors
	if (output & 0x1 || output & 0x2 || output & 0x04)
	{
		return 0;
	}

	output = ((output >> 18)&0xFFFF);
	output /= 4;	// Every bit represents 0.25 degrees C

	return (uint16_t) output;

}