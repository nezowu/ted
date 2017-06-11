#include <stdio.h>
#include <string.h>

int main(void) {
	char buff[4] = {0};
	int digit = 123;
	int i = 2;
	while(digit) {
		buff[i--] = digit % 10 + '0';
		digit = digit / 10;
	}
	printf("%-20s%zu %s %c%sf\n", "\033[33mtable", strlen(buff), buff, buff[0], "\033[m");
	printf("%s%%%-3.0F%s", "\033[33melse?", 10.8, "\033[m");
	return 0;
}
