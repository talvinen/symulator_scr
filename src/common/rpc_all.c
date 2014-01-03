#include "rpc_all.h"

void send_to(tpl_node *tn, int sockfd) {
	tpl_dump(tn, TPL_FD, sockfd);
}
