#ifndef SIMULATOR_REMOTE_CALLS_H
#define SIMULATOR_REMOTE_CALLS_H

#include <glib.h>
#include <tpl.h>
#include "simulator_data.h"

extern void (*callback_function_sim[]) (const tpl_bin * const tb, Harvester_Id harvester_id);

#endif//SIMULATOR_REMOTE_CALLS_H

