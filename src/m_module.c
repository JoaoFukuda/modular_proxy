#include "m_module.h"

#include <dlfcn.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

const char *modules_dir;

moduleNode modules_head;

char *getModulePath(const char *filename)
{
	int filepath_len = strlen(modules_dir) + strlen(filename) + 2;
	char *filepath = (char *)calloc(sizeof(char), filepath_len);
	snprintf(filepath, filepath_len, "%s/%s", modules_dir, filename);

	return filepath;
}

bool isModule(const char *filepath)
{
	return !strcmp(".so", &filepath[strlen(filepath) - 3]);
}

bool parseModule(module *module, const char *filepath)
{
	module->filepath = strdup(filepath);

	module->handler =
	    dlopen(module->filepath, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
	if (!module->handler) {
		fprintf(stderr, "Error: Failed opening %s, %s\n", module->filepath,
		        dlerror());
		free(module->filepath);
		return false;
	}

	int (*getPriority)(void) = (int (*)(void))dlsym(module->handler, "priority");
	module->priority = getPriority ? getPriority() : 10;

	module->inbound = (buffer_t(*)(buffer_t))dlsym(module->handler, "inbound");
	if (!getPriority) {
		fprintf(stderr, "Error: Failed getting inbound function from %s\n",
		        module->filepath);
		dlclose(module->handler);
		free(module->filepath);
		return false;
	}

	module->outbound = (buffer_t(*)(buffer_t))dlsym(module->handler, "outbound");
	if (!getPriority) {
		fprintf(stderr, "Error: Failed getting outbound function from %s\n",
		        module->filepath);
		dlclose(module->handler);
		free(module->filepath);
		return false;
	}

	return true;
}

bool loadModule(const char *filename)
{
	char *filepath = getModulePath(filename);

	if (!isModule(filepath)) {
		free(filepath);
		return false;
	}

	module *new_module = (module *)calloc(sizeof(module), 1);
	if (!parseModule(new_module, filepath)) {
		free(filepath);
		free(new_module);

		return false;
	}

	printf("Loading %s\n\tPriority: %d\n\tInbound: %p\n\tOutbound: %p\n",
	       new_module->filepath, new_module->priority,
	       (void *)new_module->inbound, (void *)new_module->outbound);

	moduleNode *new_node = (moduleNode *)calloc(sizeof(moduleNode), 1);
	new_node->module = new_module;

	moduleNode *aux = &modules_head;
	while (aux->next && new_module->priority < aux->next->module->priority) {
		aux = aux->next;
	}

	new_node->next = aux->next;
	aux->next = new_node;

	free(filepath);
	return true;
}

bool unloadModule(const char *filename)
{
	char *filepath = getModulePath(filename);

	moduleNode *old_module_node = modules_head.next, *prev = &modules_head;
	while (old_module_node &&
	       strcmp(old_module_node->module->filepath, filepath)) {
		prev = old_module_node;
		old_module_node = old_module_node->next;
	}

	if (!old_module_node) {
		fprintf(stderr, "Error: %s was not loaded\n", filepath);
		free(filepath);
		return false;
	}

	module *old_module = old_module_node->module;

	printf("Unloading %s\n\tPriority: %d\n\tInbound: %p\n\tOutbound: %p\n",
	       old_module->filepath, old_module->priority,
	       (void *)old_module->inbound, (void *)old_module->outbound);

	dlclose(old_module->handler);
	free(old_module->filepath);
	free(old_module);
	prev->next = old_module_node->next;
	free(old_module_node);

	free(filepath);
	return true;
}

void freeModuleList()
{
	while (modules_head.next) {
		moduleNode *aux = modules_head.next;
		modules_head.next = aux->next;
		dlclose(aux->module->handler);
		free(aux->module->filepath);
		free(aux->module);
		free(aux);
	}
}
