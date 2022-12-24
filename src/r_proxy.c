#include "r_proxy.h"

#include "r_module.h"

#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>

int listening_socket;
struct sockaddr outbound_addr;
socklen_t outbound_addrlen;
pthread_t proxy_loop_thread;

bool initProxy(const char *inboundAddress, const char *inboundPort,
               const char *outboundAddress, const char *outboundPort)
{
	struct addrinfo hint;
	struct addrinfo *inbound_res = NULL, *outbound_res = NULL;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = IPPROTO_TCP;

	int s = getaddrinfo(outboundAddress, outboundPort, &hint, &outbound_res);
	if (s != 0) {
		fprintf(stderr, "Error: Could not resolve outbound address %s:%s, %s\n",
		        inboundAddress, inboundPort, gai_strerror(s));
		freeaddrinfo(outbound_res);
		return false;
	}

	s = getaddrinfo(inboundAddress, inboundPort, &hint, &inbound_res);
	if (s != 0) {
		fprintf(stderr, "Error: Could not resolve inbound address %s:%s, %s\n",
		        inboundAddress, inboundPort, gai_strerror(s));
		freeaddrinfo(outbound_res);
		freeaddrinfo(inbound_res);
		return false;
	}

	outbound_addr = *outbound_res->ai_addr;
	outbound_addrlen = outbound_res->ai_addrlen;
	freeaddrinfo(outbound_res);

	listening_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listening_socket == -1) {
		fprintf(stderr, "Error: Could not create inbound socket\n");
		freeaddrinfo(inbound_res);
		return false;
	}

	if (bind(listening_socket, inbound_res->ai_addr, inbound_res->ai_addrlen) ==
	    -1) {
		fprintf(stderr, "Error: Could not bind inbound socket\n");
		freeaddrinfo(inbound_res);
		close(listening_socket);
		return false;
	}

	freeaddrinfo(inbound_res);

	listen(listening_socket, 16);

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(listening_socket, (struct sockaddr *)&sin, &len) == -1) {
		fprintf(stderr, "Error: Could not get listening socket info, %s\n",
		        strerror(errno));
	}
	else {
		printf("Listening on port number %d\n", ntohs(sin.sin_port));
	}

	return true;
}

void *proxyLoop(void *ptr)
{
	printf("Running proxy loop in a separate thread\n");

	int inbound_socket = accept(listening_socket, NULL, NULL);
	if (inbound_socket == -1) {
		fprintf(stderr, "Error: Could not accept on listening_socket\n");
		pthread_exit(ptr);
	}

	int outbound_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (outbound_socket == -1) {
		fprintf(stderr, "Error: Could not create outbound socket\n");
		close(inbound_socket);
		pthread_exit(ptr);
	}

	if (connect(outbound_socket, &outbound_addr, outbound_addrlen) == -1) {
		fprintf(stderr, "Error: Could not connect to outbound address\n");
		close(inbound_socket);
		close(outbound_socket);
		pthread_exit(ptr);
	}

	fd_set set;
	FD_ZERO(&set);

	char buf[4096];

	while (true) {
		FD_SET(inbound_socket, &set);
		FD_SET(outbound_socket, &set);

		select(MAX(inbound_socket, outbound_socket) + 1, &set, NULL, NULL, NULL);

		if (FD_ISSET(inbound_socket, &set)) {
			int data_len = recv(inbound_socket, buf, sizeof(buf), 0);
			if (data_len == -1) {
				fprintf(stderr, "Error: Could not read from inbound_socket, %s\n",
				        strerror(errno));
				pthread_exit(ptr);
			}

			if (data_len == 0) {
				close(inbound_socket);
				close(outbound_socket);
				break;
			}

			buffer_t buffer;
			buffer.size = data_len;
			buffer.data = (const uint8_t *)buf;

			buffer = runThroughOutbound(buffer);

			data_len = send(outbound_socket, buffer.data, buffer.size, 0);
			if (data_len == -1) {
				fprintf(stderr, "Error: Could not send to outbound_socket, %s\n",
				        strerror(errno));
				pthread_exit(ptr);
			}
		}

		if (FD_ISSET(outbound_socket, &set)) {
			int data_len = recv(outbound_socket, buf, sizeof(buf), 0);
			if (data_len == -1) {
				fprintf(stderr, "Error: Could not read from outbound_socket, %s\n",
				        strerror(errno));
				pthread_exit(ptr);
			}

			if (data_len == 0) {
				close(outbound_socket);
				close(inbound_socket);
				break;
			}

			buffer_t buffer;
			buffer.size = data_len;
			buffer.data = (const uint8_t *)buf;

			buffer = runThroughInbound(buffer);

			data_len = send(inbound_socket, buffer.data, buffer.size, 0);
			if (data_len == -1) {
				fprintf(stderr, "Error: Could not send to inbound_socket, %s\n",
				        strerror(errno));
				pthread_exit(ptr);
			}
		}
	}

	pthread_exit(ptr);
}

void runProxy()
{
	printf("Proxy running\n");

	pthread_create(&proxy_loop_thread, NULL, proxyLoop, NULL);
}

void destroyProxy()
{
	close(listening_socket);

	pthread_join(proxy_loop_thread, NULL);
}
