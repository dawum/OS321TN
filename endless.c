#include <stdio.h>

int main() {
	unsigned int a, b;
	volatile unsigned int c;
	a = 0u; c = 0u;
	while (1) {
		printf("%u\n", a);
		for (b=0u;b<(1u<<30);b++) {
			c++;
		}
		a++;
	}
	return 0;
}

