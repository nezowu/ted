#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(void) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	printf("%d\n", w.ws_row);
	return 0;
}
