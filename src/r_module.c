#include "r_module.h"

buffer_t runThroughInbound(buffer_t buffer)
{
	moduleNode *aux = modules_head.next;
	while (aux) {
		buffer = aux->module->inbound(buffer);
		aux = aux->next;
	}

	return buffer;
}

buffer_t runThroughOutbound(buffer_t buffer)
{
	moduleNode *aux = modules_head.next;
	while (aux) {
		buffer = aux->module->outbound(buffer);
		aux = aux->next;
	}

	return buffer;
}
