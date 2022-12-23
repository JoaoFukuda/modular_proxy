#ifndef _M_MODULE_H
#define _M_MODULE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	size_t size;
	uint8_t *data;
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
} moduleListNode;

typedef struct {
	moduleListNode *head;
} moduleList;

extern const char *modules_dir;

extern moduleList module_list;

void initModuleList();

bool loadModule(const char *filename);
bool unloadModule(const char *filename);

#endif
