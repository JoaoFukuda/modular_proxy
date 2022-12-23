#include "r_monitor.h"

#include "m_module.h"

#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX(x, y) (x > y ? x : y)

int inotify_fd;
int watch_fd;

bool handleEvent()
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
	int dirname_len = strlen(dirname);
	modules_dir = (char *)malloc(dirname_len);
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

void runMonitoring()
{
	bool running = true;

	fd_set set;
	FD_ZERO(&set);

	while (running) {
		FD_SET(STDIN_FILENO, &set);
		FD_SET(inotify_fd, &set);
		select(MAX(STDIN_FILENO, inotify_fd) + 1, &set, NULL, NULL, NULL);

		if (FD_ISSET(STDIN_FILENO, &set)) {
			char *line = NULL;
			size_t line_len = 0;

			if (getline(&line, &line_len, stdin) == -1) {
				free(line);
				fprintf(stderr, "Error: %s\n", strerror(errno));
				return;
			}

			if (line[0] == 'q') {
				running = false;
			}

			free(line);
		}
		else if (FD_ISSET(inotify_fd, &set)) {
			if (!handleEvent()) {
				running = false;
			}
		}
	}

	close(inotify_fd);
	free((void *)modules_dir);
	modules_dir = NULL;
}
