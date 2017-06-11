#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
	char *buff = malloc(8);
	buff += 7;
	int digit = 123;
	do {
		*--buff = digit % 10 + '0';
	} while(digit /= 10);

	printf("%s\n", buff);
	return 0;
}
