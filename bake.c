#include "bake.h"

#include <stdlib.h>

#include "toast_timestamp.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#include "utils/uartstdio.h"


static volatile REFLOW_INSTANCE_T * reflow = NULL;


volatile REFLOW_INSTANCE_T * reflow_get_instance(void)
{
	if (reflow == NULL)
	{
		UARTprintf("Allocating New Reflow Struct\r\n");
		reflow = (REFLOW_INSTANCE_T *)malloc(sizeof(REFLOW_INSTANCE_T));
	}

	return reflow;
}

void reflow_init(volatile REFLOW_INSTANCE_T * instance)
{
	instance->start_time = 0;
	instance->profile = (REFLOW_PROFILE_T *)calloc(20, sizeof(REFLOW_PROFILE_T));	// Space for 20 reflow profile spots
	instance->profile_points = 0;
	instance->active = 0;

	return;
}

void reflow_start(volatile REFLOW_INSTANCE_T * instance)
{
	timestamp_reset();		// Reset Timestamp to 0, avoid overflow
	instance->start_time = timestamp_get();
	instance->active = 1;
}


void reflow_destory(volatile REFLOW_INSTANCE_T * instance)
{
	free(instance->profile);

	return;
}

uint16_t reflow_calc_temp(volatile REFLOW_INSTANCE_T * instance)
{
	uint16_t current_time = timestamp_get() - instance->start_time;


	for (int i = 0; i < (instance->profile_points-1); i++)
	{
		if (instance->profile[i].timestamp <= current_time 
				&& instance->profile[i+1].timestamp >= current_time)
		{
			REFLOW_PROFILE_T * next = &(instance->profile[i+1]);
			REFLOW_PROFILE_T * last = &(instance->profile[i]);

			int16_t slope = (next->temperature - last->temperature)/(next->timestamp - last->timestamp);

			return last->temperature + slope * (current_time -last->timestamp);
		}
	}

	return 0;
}