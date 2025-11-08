#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv) {
	char* pathenv = getenv("PATH");
	if (!pathenv) {
		fprintf(stderr, "Failed to find PATH enviroment variable\n");
		return 1;
	}

	if (argc == 2) {
		struct stat file;
		if (stat(argv[1], &file) == -1) {
			return 0;
		}
		char* dir = NULL;
		for (dir = strtok(pathenv, ":"); (dir = strtok(NULL, ":"));) {
			struct stat buf;
			if (stat(dir, &buf) == -1) {
				fprintf(stderr, "%s: %s\n", strerror(errno), argv[1]);
				continue;
			}
			if (S_ISDIR(buf.st_mode) && buf.st_mtime > file.st_mtime) {
				return 0;
			}
		}
		return 1;
	}

	char path[PATH_MAX];

	char* dir = NULL;
	for (dir = strtok(pathenv, ":"); dir ;dir = strtok(NULL, ":")) {
		DIR* dirfd = opendir(dir);
		if (!dirfd) {
			fprintf(stderr, "%s: %s\n", strerror(errno), dir);
			continue;
		}

		struct dirent* d = NULL;
		while ((d = readdir(dirfd))) {
			int r = snprintf(path, sizeof(path), "%s/%s", dir, d->d_name);
			if (r <= 0 || r >= PATH_MAX) continue;
			struct stat buf;
			if (stat(path, &buf) == -1) {
				fprintf(stderr, "%s: %s\n", strerror(errno), path);
				continue;
			}
			if (S_ISREG(buf.st_mode) && !access(path, X_OK)) {
				puts(d->d_name);
			}
		}
		closedir(dirfd);
	}
	return 0;
}
