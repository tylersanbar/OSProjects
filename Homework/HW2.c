#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(void) {
	int fds[2];
	int val = 0;

	pipe(fds);
	int pid = fork();

	if (pid > 0) {
		close(fds[1]);
		val += 17;

		read(fds[0], &val, sizeof(val));

		val += 3;

		printf("%d\n", val);
		wait(NULL);
	}
	else {
		close(fds[0]);
		val += 5;

		write(fds[1], &val, sizeof(val));
	}
	return 0;
}