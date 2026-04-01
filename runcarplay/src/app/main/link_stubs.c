/**
 * Stubs for link/carplay symbols used by liblvgl.a.
 * - request_link_action: stub (events_init.c calls it)
 * - zlink_client_run: empty stub only when ENABLE_CARPLAY=0 (when ENABLE_CARPLAY=1, zlink_client.o provides it)
 */
#include "linksupport.h"

#ifndef ENABLE_CARPLAY
void zlink_client_run(void) {}
#endif

int request_link_action(LinkType type, int action, int64_t lparam, const char *sparam)
{
	(void)type;
	(void)action;
	(void)lparam;
	(void)sparam;
	return 0;
}
