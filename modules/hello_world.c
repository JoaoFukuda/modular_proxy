#include "module.h"

#include <stdio.h>

int priority()
{
	return 10;
}

buffer_t inbound(buffer_t buffer)
{
	printf("INBOUND size(%lu)\n", buffer.size);
	return buffer;
}

buffer_t outbound(buffer_t buffer)
{
	printf("OUTBOUND size(%lu)\n", buffer.size);
	return buffer;
}
