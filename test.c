#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	int fd, in;
	char *filename;
	char suff;
	char duff[256];
	if(argc == 1)
		filename = ".file~";
	else if(argc == 2)
		filename = argv[1];
	else {
		fprintf(stderr, "%s\n", "Usage: ./ted filename.txt or ./ted without parameters");
		exit(EXIT_FAILURE);
	}
	fd = open(filename, O_RDWR|O_CREAT, 0664);
	if(fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	in = open("/dev/tty", O_RDONLY);
	while(read(in, &suff, 1) != -1 && suff != '\n') {
		strncat(duff, &suff, 1);
	}
	if(errno) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	strncat(duff, "\n", 1);
	write(STDOUT_FILENO, duff, strlen(duff));

	exit(EXIT_SUCCESS);
}
