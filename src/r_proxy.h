#ifndef _R_PROXY_H
#define _R_PROXY_H

#include <stdbool.h>

bool initProxy(const char *inboundAddress, const char *inboundPort,
               const char *outboundAddress, const char *outboundPort);

void runProxy();
void destroyProxy();

#endif
