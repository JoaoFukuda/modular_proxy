#include "r_module.h"

buffer_t runThroughInbound(buffer_t buffer)
{
	moduleListNode *aux = module_list.head->next;
	while (aux) {
		buffer = aux->module->inbound(buffer);
	}

	return buffer;
}

buffer_t runThroughOutbound(buffer_t buffer)
{
	moduleListNode *aux = module_list.head->next;
	while (aux) {
		buffer = aux->module->outbound(buffer);
	}

	return buffer;
}
