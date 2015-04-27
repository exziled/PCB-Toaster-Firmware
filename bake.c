#include "bake.h"

#include <stdlib.h>

#include "toast_timestamp.h"


extern REFLOW_INSTANCE_T * reflow;

REFLOW_INSTANCE_T * reflow_get_instance(void)
{
	if (reflow == NULL)
	{
		reflow = (REFLOW_INSTANCE_T *)malloc(sizeof(REFLOW_INSTANCE_T));
	}

	return reflow;
}

void reflow_init(REFLOW_INSTANCE_T * instance)
{
	instance->start_time = 0;
	instance->profile = (REFLOW_PROFILE_T *)calloc(20, sizeof(REFLOW_PROFILE_T));	// Space for 20 reflow profile spots
	instance->profile_points = 0;

	return;
}

void reflow_start(REFLOW_INSTANCE_T * instance)
{
	timestamp_reset();		// Reset Timestamp to 0, avoid overflow
	instance->start_time = timestamp_get();
}

void reflow_destory(REFLOW_INSTANCE_T * instance)
{
	free(instance->profile);

	return;
}

uint16_t reflow_calc_temp(REFLOW_INSTANCE_T * instance)
{
	uint16_t current_time = instance->start_time - timestamp_get();

	for (int i = 0; i < instance->profile_points; i++)
	{
		if (instance->profile[i].timestamp <= current_time 
				&& instance->profile[i+1].timestamp >= current_time)
		{
			REFLOW_PROFILE_T * next = &(instance->profile[i+1]);
			REFLOW_PROFILE_T * last = &(instance->profile[i]);

			int16_t slope = (next->temperature - last->temperature)/(next->timestamp - last->timestamp);

			return last->temperature + slope * (current_time - last->timestamp);
		}
	}
}