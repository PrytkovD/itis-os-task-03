#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
	srand(time(NULL));

	int n = rand() % 61 + 120;

	char *op = "+-*/";

	for (int i = 0; i < n; i++) {
		int x = rand() % 9 + 1;
		int o = rand() % 4;
		int y = rand() % 9 + 1;

		printf("%d %c %d\n", x, op[o], y);
		sleep(1);
	}
}
