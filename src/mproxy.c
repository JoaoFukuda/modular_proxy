#include "r_monitor.h"
#include "r_proxy.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
	const char *modules_dir;

	const char *inbound_address;
	const char *inbound_port;
	const char *outbound_address;
	const char *outbound_port;
} parameters;

parameters params;

bool parseArgs(int argc, char *argv[])
{
	params.inbound_address = "0.0.0.0";
	params.inbound_port = "0";
	params.outbound_address = "127.0.0.1";

	if (argc != 3) {
		printf("Usage: %s -d <dirname> [-i [<inaddr>:]<inport>] -o "
		       "[<outaddr>:]<outport>\n",
		       argv[0]);
		return false;
	}

	params.modules_dir = argv[1];
	params.outbound_port = argv[2];

	return true;
}

int main(int argc, char *argv[])
{
	if (!parseArgs(argc, argv)) {
		return -1;
	}

	printf("Loading MProxy...\n");

	printf("Initializing Monitor\n");
	if (!initMonitor(params.modules_dir)) {
		return -1;
	}

	printf("Initializing Proxy\n");
	if (!initProxy(params.inbound_address, params.inbound_port,
	               params.outbound_address, params.outbound_port)) {
		return -1;
	}

	printf("Running Proxy\n");
	runProxy();

	printf("Running Monitor\n");
	runMonitor();

	getchar();

	destroyProxy();
	destroyMonitor();
}
