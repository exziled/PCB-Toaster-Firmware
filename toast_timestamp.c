#include "toast_timestamp.h"

extern uint32_t system_timestamp;


void timestamp_reset(void)
{
	system_timestamp = 0;
}


uint32_t timestamp_get(void)
{
	return system_timestamp;
}

void timestamp_increment(void)
{
	system_timestamp += 1;
}