#include "r_monitor.h"
#include "m_module.h"
#include "r_proxy.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
	const char *modules_dir;

	const char *inbound_address;
	unsigned short inbound_port;
	const char *outbound_address;
	unsigned short outbound_port;
} parameters;

parameters params;

bool parseArgs(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <dirname> [inaddr:]inport:[outaddr:]outport\n", argv[0]);
		return false;
	}

	params.modules_dir = argv[1];

	return true;
}

int main(int argc, char *argv[])
{
	if (!parseArgs(argc, argv)) {
		return -1;
	}

	initModuleList();

	if (!initMonitor(params.modules_dir)) {
		return -1;
	}

	if (!initProxy(params.inbound_address, params.inbound_port,
	               params.outbound_address, params.outbound_port)) {
		return -1;
	}

	runProxy();
	runMonitoring();
}
