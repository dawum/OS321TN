#include <stdio.h>
#include <unistd.h>

int main() {
	unsigned int a;
	a = 0u;
	while (1) {
		printf("%u\n", a);
		sleep(1);
		a++;
	}
	return 0;
}

