#include "r_monitor.h"
#include "r_proxy.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
		const char *modules_dir;
		const char *inbound_address;
		const char *inbound_port;
		const char *outbound_address;
		const char *outbound_port;
} mproxy_config;

mproxy_config params;

void usage(char *filename)
{
	printf("Usage: %s -d <dirname> [-i [<inbound_addr>:]<inbound_port>] -o "
	       "[<outbound_addr>:]<outbound_port>\n",
	       filename);
}

bool parseArgs(int argc, char *argv[])
{
	params.modules_dir = NULL;
	params.inbound_address = "0.0.0.0";
	params.inbound_port = "0";
	params.outbound_address = "127.0.0.1";
	params.outbound_port = NULL;

	if (argc < 4) {
		usage(argv[0]);
		return false;
	}

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d")) {
			params.modules_dir = argv[++i];
		}
		else if (!strcmp(argv[i], "-o")) {
			char *outbound_string = argv[++i];
			char *port = strchr(outbound_string, ':');
			if (!port) {
				params.outbound_port = outbound_string;
			}
			else {
				*port = '\0';
				port++;
				params.outbound_address = outbound_string;
				params.outbound_port = port;
			}
		}
		else if (!strcmp(argv[i], "-i")) {
			char *inbound_string = argv[++i];
			char *port = strchr(inbound_string, ':');
			if (!port) {
				params.inbound_port = inbound_string;
			}
			else {
				*port = '\0';
				port++;
				params.inbound_address = inbound_string;
				params.inbound_port = port;
			}
		}
		else {
			usage(argv[0]);
			return false;
		}
	}

	if (params.modules_dir == NULL || params.outbound_port == NULL) {
		usage(argv[0]);
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{
	if (!parseArgs(argc, argv)) {
		return -1;
	}

	if (!initMonitor(params.modules_dir)) {
		return -1;
	}

	if (!initProxy(params.inbound_address, params.inbound_port,
	               params.outbound_address, params.outbound_port)) {
		return -1;
	}

	runProxy();
	runMonitor();

	getchar();

	destroyProxy();
	destroyMonitor();
}
