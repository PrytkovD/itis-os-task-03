#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

int produced = 0;

void signal_handler(int signal) {
	printf("Produced: %d\n", produced);
}

int main(int argc, char *argv[]) {
	int pipe10[2], pipe02[2], pipe20[2];

	if (pipe(pipe10) == -1 || pipe(pipe02) == -1 || pipe(pipe20) == -1) {
		perror("pipe");
		_exit(EXIT_FAILURE);
    	}

	printf("Created pipes\n");

	pid_t p1, p2;

	p1 = fork();

	if (p1 < 0) {
		perror("fork");
		_exit(EXIT_FAILURE);
	}

	if (p1 == 0) {
		// child process
		close(pipe02[0]);
		close(pipe02[1]);
		close(pipe20[0]);
		close(pipe20[1]);

		close(pipe10[0]);
		dup2(pipe10[1], STDOUT_FILENO);

		char *argv[] = { "-i0", "-oL", "-e0", "./out/producer.exe", NULL };

		execve("/usr/bin/stdbuf", argv, NULL);
		perror("execve");
		_exit(EXIT_FAILURE);
	} else {
		// parent process
		close(pipe10[1]);
	}

	p2 = fork();

	if (p2 < 0) {
		perror("fork");
		_exit(EXIT_FAILURE);
	}

	if (p2 == 0) {
		// child process
		close(pipe10[0]);
		close(pipe10[1]);

		close(pipe02[1]);
		close(pipe20[0]);
		dup2(pipe02[0], STDIN_FILENO);
		dup2(pipe20[1], STDOUT_FILENO);

		char *argv[] = { "-i0", "-oL", "-e0", "/usr/bin/bc", NULL };

		execve("/usr/bin/stdbuf", argv, NULL);
		perror("execve");
		_exit(EXIT_FAILURE);
	} else {
		// parent process
		close(pipe02[0]);
		close(pipe20[1]);
	}

	char buffer10[1024] = "";
	char buffer20[1024] = "";

	signal(SIGUSR1, signal_handler);

	while (read(pipe10[0], buffer10, sizeof(buffer10)) >= 0) {
		write(pipe02[1], buffer10, strlen(buffer10));
		read(pipe20[0], buffer20, sizeof(buffer20));
		buffer10[strlen(buffer10) - 1] = 0;
		if (buffer20[strlen(buffer20) - 2] == '\n') {
			buffer20[strlen(buffer20) - 1] = 0;
		}
		printf("%s = %s", buffer10, buffer20);
		produced++;
	}

	printf("Produced: %d\n", produced);

	return 0;
}
