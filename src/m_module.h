#ifndef _M_MODULE_H
#define _M_MODULE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
		size_t size;
		const uint8_t *data;
} buffer_t;

typedef struct {
		int priority;
		char *filepath;
		void *handler;

		buffer_t (*inbound)(buffer_t);
		buffer_t (*outbound)(buffer_t);
} module;

typedef struct aux {
		module *module;
		struct aux *next;
} moduleNode;

extern const char *modules_dir;

extern moduleNode modules_head;

void freeModuleList();

bool loadModule(const char *filename);
bool unloadModule(const char *filename);

#endif
