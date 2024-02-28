#include "r_monitor.h"

#include "m_module.h"

#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX(x, y) (x > y ? x : y)

int inotify_fd;
int watch_fd;

bool monitor_running;
pthread_t monitor_loop_thread;

bool handleEvent(void)
{
	uint8_t buffer[1024]
	    __attribute__((aligned(__alignof__(struct inotify_event))));
	const struct inotify_event *event;

	ssize_t bytes_read = read(inotify_fd, buffer, sizeof(buffer));
	if (bytes_read == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}
	if (bytes_read < (ssize_t)sizeof(*event)) {
		fprintf(stderr,
		        "Error: Could not read all data of an event (%ld bytes read)\n",
		        bytes_read);
		return false;
	}

	for (const struct inotify_event *event = (void *)buffer;
	     (void *)event < (void *)(buffer + bytes_read);
	     event += sizeof(*event) + event->len) {
		if (event->mask & IN_ISDIR) {
			continue;
		}

		if (event->mask & IN_MOVED_TO || event->mask & IN_CREATE) {
			sleep(1); // Sleep a bit so that when you read the module, it's not empty
			loadModule(event->name);
		}

		if (event->mask & IN_MOVED_FROM || event->mask & IN_DELETE) {
			unloadModule(event->name);
		}
	}

	return true;
}

bool initMonitor(const char *dirname)
{
	printf("Initializing monitor on '%s'\n", dirname);
	int dirname_len = strlen(dirname);
	modules_dir = (char *)calloc(sizeof(char), dirname_len + 1);
	strncpy((char *)modules_dir, dirname, dirname_len);

	if (modules_dir[strlen(modules_dir) - 1] == '/') {
		((char *)modules_dir)[strlen(modules_dir) - 1] = '\0';
	}

	inotify_fd = inotify_init();
	if (inotify_fd == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}

	watch_fd =
	    inotify_add_watch(inotify_fd, modules_dir,
	                      IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE | IN_CREATE);
	if (inotify_fd == -1) {
		fprintf(stderr, "Error: %s\n", strerror(errno));
		return false;
	}

	return true;
}

void *monitorLoop(void *ptr)
{
	fd_set set;
	FD_ZERO(&set);

	monitor_running = true;
	while (monitor_running) {
		FD_SET(inotify_fd, &set);
		select(inotify_fd + 1, &set, NULL, NULL, NULL);

		if (FD_ISSET(inotify_fd, &set)) {
			if (!handleEvent()) {
				monitor_running = false;
			}
		}
	}

	pthread_exit(ptr);
}

void scanDirectory(const char *dirname)
{
	printf("Scanning %s for modules\n", dirname);
	DIR *dir = opendir(dirname);
	if (!dir) {
		return;
	}

	struct dirent *ent = NULL;
	while ((ent = readdir(dir))) {
		if (ent->d_name[0] == '.') {
			continue;
		}

		// No support for modules in subdirectories yet
		switch (ent->d_type) {
			case DT_REG:
				loadModule(ent->d_name);
			default:
				continue;
		}
	}

	closedir(dir);
}

void runMonitor(void)
{
	scanDirectory(modules_dir);
	pthread_create(&monitor_loop_thread, NULL, monitorLoop, NULL);
}

void destroyMonitor(void)
{
	monitor_running = false;
	pthread_join(monitor_loop_thread, NULL);
	freeModuleList();

	close(inotify_fd);
	free((void *)modules_dir);
	modules_dir = NULL;
}
