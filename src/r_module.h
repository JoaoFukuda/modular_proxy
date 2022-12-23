#ifndef _R_MODULE_H
#define _R_MODULE_H

#include "m_module.h"

#include <stddef.h>

buffer_t runThroughInbound(buffer_t buffer);
buffer_t runThroughOutbound(buffer_t buffer);

#endif
