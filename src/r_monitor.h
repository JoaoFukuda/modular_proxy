#ifndef _R_MONITOR_H
#define _R_MONITOR_H

#include <stdbool.h>
#include <sys/inotify.h>

bool initMonitor(const char *modules_dir);

// Load or unload modules based on events from the configured directory
void runMonitor();
void destroyMonitor();

#endif
