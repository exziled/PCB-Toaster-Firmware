//*****************************************************************************
//
// io.c - I/O routines for the enet_io example application.
//
// Copyright (c) 2013-2014 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.0.12573 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "io.h"

#include "bake.h"
#include "max31855.h"
#include "triac_mgmt.h"

//*****************************************************************************
//
// Hardware connection for the user LED.
//
//*****************************************************************************
#define LED_PORT_BASE GPIO_PORTN_BASE
#define LED_PIN GPIO_PIN_0

//*****************************************************************************
//
// Hardware connection for the animation LED.
//
//*****************************************************************************
#define LED_ANIM_PORT_BASE GPIO_PORTN_BASE
#define LED_ANIM_PIN GPIO_PIN_1

//*****************************************************************************
//
// The system clock speed.
//
//*****************************************************************************
extern uint32_t g_ui32SysClock;

//*****************************************************************************
//
// The current speed of the on-screen animation expressed as a percentage.
//
//*****************************************************************************
volatile unsigned long g_ulAnimSpeed = 10;

//*****************************************************************************
//
// Set the timer used to pace the animation.  We scale the timer timeout such
// that a speed of 100% causes the timer to tick once every 20 mS (50Hz).
//
//*****************************************************************************


//*****************************************************************************
//
// Initialize the IO used in this demo
//
//*****************************************************************************
void
io_init(void)
{
    //
    // Configure Port N0 for as an output for the status LED.
    //
    ROM_GPIOPinTypeGPIOOutput(LED_PORT_BASE, LED_PIN);

    //
    // Configure Port N0 for as an output for the animation LED.
    //
    ROM_GPIOPinTypeGPIOOutput(LED_ANIM_PORT_BASE, LED_ANIM_PIN);

    //
    // Initialize LED to OFF (0)
    //
    ROM_GPIOPinWrite(LED_PORT_BASE, LED_PIN, 0);

    //
    // Initialize animation LED to OFF (0)
    //
    ROM_GPIOPinWrite(LED_ANIM_PORT_BASE, LED_ANIM_PIN, 0);

    //
    // Enable the peripherals used by this example.
    //
    // ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

    // //
    // // Configure the timer used to pace the animation.
    // //
    // ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);

    // //
    // // Setup the interrupts for the timer timeouts.
    // //
    // ROM_IntEnable(INT_TIMER2A);
    // ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Set the timer for the current animation speed.  This enables the
    // timer as a side effect.
    // //
    // io_set_timer(g_ulAnimSpeed);
}

//*****************************************************************************
//
// Set the status LED on or off.
//
//*****************************************************************************
void
io_set_led(bool bOn)
{
    //
    // Turn the LED on or off as requested.
    //
    ROM_GPIOPinWrite(LED_PORT_BASE, LED_PIN, bOn ? LED_PIN : 0);
}

//*****************************************************************************
//
// Return LED state
//
//*****************************************************************************
void
io_get_ledstate(char * pcBuf, int iBufLen)
{
    //
    // Get the state of the LED
    //
    if(ROM_GPIOPinRead(LED_PORT_BASE, LED_PIN))
    {
        usnprintf(pcBuf, iBufLen, "ON");
    }
    else
    {
        usnprintf(pcBuf, iBufLen, "OFF");
    }

}








void io_get_temperature(char * pcBuf, int iBufLen)
{
    uint16_t temp = getToasterTemp();

    usnprintf(pcBuf, iBufLen, "%d,%d", timestamp_get(), temp);
}

void io_get_power(char * pcBuf, int iBufLen, char * heater, int size)
{
    uint8_t h_index = atoi(heater);
    uint8_t power = 0;

    if (h_index < 2)
    {
        power = triac_map[h_index].duty_cycle;
    }

    usnprintf(pcBuf, iBufLen, "%d", power);
}

void io_get_status(char * pcBuf, int iBufLen)
{
    if (reflow_get_instance()->active) {
        usnprintf(pcBuf, iBufLen, "on");
    } else {
        usnprintf(pcBuf, iBufLen, "off");
    }
}


void io_start_bake(char * reflow_points, int size)
{
    char * token;
    uint8_t count = 0;
    uint16_t test;
    char seperator[] = ",";

    volatile REFLOW_INSTANCE_T * reflow = reflow_get_instance();
    volatile REFLOW_PROFILE_T * profile = reflow->profile;


    // reflow_init(reflow);

    // UARTprintf(reflow_points);

    token = strtok(reflow_points, seperator);
    while(token != NULL)
    {
        if (count % 2)
        {
            profile[count/2].temperature = atoi(token);
        } else {
            profile[count/2].timestamp = atoi(token);
        }

        count++;
        token = strtok(NULL, seperator);
    }

    reflow->profile_points = count / 2;

    reflow_start(reflow);

    for (int i = 0; i < reflow->profile_points; i++)
    {
        UARTprintf("%d - %d\r\n", reflow->profile[i].timestamp, reflow->profile[i].temperature);
    }
}