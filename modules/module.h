#ifndef _MODULE_H
#define _MODULE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	size_t size;
	uint8_t *data;
} buffer_t;

int priority(void);

buffer_t inbound(buffer_t buffer);
buffer_t outbound(buffer_t buffer);

#endif
