#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char * utoa(int digit) {
	char *buff = malloc(8);
	buff+=7;
	do {
		*--buff = digit % 10 + '0';
	} while(digit /= 10);
	return buff;
}
int main(void) {
	char *buff;
	int digit = 123;
	buff = utoa(digit);
	printf("%s\n", buff);
	return 0;
}
