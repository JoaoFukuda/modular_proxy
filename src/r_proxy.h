#ifndef _R_PROXY_H
#define _R_PROXY_H

#include <stdbool.h>

bool initProxy(const char *inboundAddress, unsigned short inboundPort,
               const char *outboundAddress, unsigned short outboundPort);

void runProxy();

#endif
