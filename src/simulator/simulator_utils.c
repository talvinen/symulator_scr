#include "simulator_utils.h"
#include <stdlib.h>

void gHashTableDestroyNotify(gpointer data) {
	if (data)
		free(data);
}
