#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

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
		close(pipe02[1]);
		close(pipe20[0]);
		dup2(pipe02[0], STDIN_FILENO);
		dup2(pipe20[1], STDOUT_FILENO);

		char *argv[] = { "-i0", "-oL", "-e0", "/usr/bin/bc", NULL };

		execve("/usr/bin/stdbuf", argv, NULL);
		perror("execve");
		_exit(EXIT_FAILURE);
	} else {
		close(pipe02[0]);
		close(pipe20[1]);
	}

//	close(pipe10[0]);
//	close(pipe10[1]);
//	close(pipe02[1]);
//	close(pipe20[0]);

	char buffer10[1024] = "";
	char buffer20[1024] = "";
	int count = 0;

//	int result = fcntl(pipe20[0], F_SETFL, fcntl(pipe20[0], F_GETFL) | O_NONBLOCK);
//
//	if (result < 0) {
//		perror("fcntl");
//	}

	while (read(pipe10[0], buffer10, sizeof(buffer10)) >= 0) {
		write(pipe02[1], buffer10, strlen(buffer10));
		read(pipe20[0], buffer20, sizeof(buffer20));
		buffer10[strlen(buffer10) - 1] = 0;
		if (buffer20[strlen(buffer20) - 2] == '\n') {
			buffer20[strlen(buffer20) - 1] = 0;
		}
		printf("%s = %s", buffer10, buffer20);
		count++;
	}

	signal(SIGUSR1, SIG_IGN);
	printf("Produced: %d\n", count);

	return 0;
}
