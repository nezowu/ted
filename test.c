#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/mman.h>

void phelp(void);
size_t size_page_current(off_t);

void *getline_ptr(int, void*);
int print_line(int, int, void*);

int main(int argc, char *argv[]) {
	const char *wellcom = "Введите команду[h - справка]--> ";
//	const char *profi = "--> ";
//	int len_profi = 4;
	int len_wellcom = 53;
	int fd, first = 0, last = 0, count_str = 1, current_str;
	char *filename;
	char suff;
	char duff[256] = {0}; //duff[8]
	struct termios saved_attr;
	struct termios set_attr;
	int in = open("/dev/tty", O_RDONLY);
	size_t PAGE;
	struct stat buff;
	void *p, *d, *end_p, *last_line;
	tcgetattr(in, &saved_attr);
	set_attr = saved_attr;
	set_attr.c_lflag &= ~(ICANON|ECHO|ISIG);
	set_attr.c_cc[VMIN] = 1;
	set_attr.c_cc[VTIME] = 0;

	if(argc == 1) {
		filename = ".file~";
		PAGE = sysconf(_SC_PAGESIZE);
		fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0664);
	}
	else if(argc == 2) {
		filename = argv[1];
		if(stat(argv[1], &buff) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}
		PAGE = size_page_current(buff.st_size);
		fd = open(filename, O_RDWR);
	}
	else {
		fprintf(stderr, "%s\n", "Использование: ted <имя_файла.txt> или ted (без параметров)");
		exit(EXIT_FAILURE);
	}

	if(fd == -1) {
		perror("open:36,45");
		exit(EXIT_FAILURE);
	}

	p = mmap(0, PAGE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(p == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	d = p - 1;
	for(count_str = 0; (d = memchr(d + 1, '\n', strlen(d+1))) != NULL; count_str++) {
		if(strlen(d+1) > 0)
			last_line = d + 1;
		else
			end_p = d + 1;
	}
	current_str = count_str;
	tcsetattr(in, TCSAFLUSH, &set_attr);
	write(STDOUT_FILENO, "Введите команду[h - справка]--> ", 53);

	while(read(in, &suff, 1) && suff != '\004') { //нужна проверка read
		if(isdigit(suff) && !first ) {
			if(suff == '0' && strlen(duff) < 1)
				continue;
			write(STDOUT_FILENO, &suff, 1);
			strncat(duff, &suff, 1);
			continue;
		}
		if(strlen(duff) > 0 && !first) {
			first = atoi(duff);
			duff[0] = 0;
			if(first > count_str)
				first = count_str;
		}
		if(suff == '$' && !first) {
			write(STDOUT_FILENO, &suff, 1);
			first = count_str;
			continue;
		}
		if( suff == '-' && first) {
			write(STDOUT_FILENO, &suff, 1);
			continue;
		}
		if(isdigit((int)suff) && first) {
			write(STDOUT_FILENO, &suff, 1);
			strncat(duff, &suff, 1);
			continue;
		}
		if(suff == '$' && first) {
			write(STDOUT_FILENO, &suff, 1);
			last = count_str;
			continue;
		}
		if(first && strlen(duff) > 0) {
			last = atoi(duff);
			if(last > count_str)
				last = count_str;
			duff[0] = 0;
		}
		switch(suff) {
			case 'h':
				write(STDOUT_FILENO, &suff, 1);
				phelp();
				write(STDOUT_FILENO, "Введите команду[h - справка]--> ", 53);
				break;
			case 'p':
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n", 1);
				if(!first && !last)
					current_str = print_line(current_str, last, p);
				else
					current_str = print_line(first, last, p);
				write(STDOUT_FILENO, wellcom, len_wellcom);
				break;
			default:
				write(STDOUT_FILENO, "Всякая хрень попадается\n", 45);
				break;
		}

		write(STDOUT_FILENO, "Конец главного цикла...\n", 42);
		first = 0;
		last = 0;
	}
//	if(errno) {
//		perror("read");
//		tcsetattr(in, TCSANOW, &saved_attr);
//		exit(EXIT_FAILURE);
//	}

//	write(STDOUT_FILENO, duff, strlen(duff));

	tcsetattr(in, TCSANOW, &saved_attr);
	exit(EXIT_SUCCESS);
}

void phelp(void) {
	printf("%s", "\nh - справка\np - вывод текущей строки\n\
i - вставка текста\nd - удалить текущую строку\n\
w - сохранение документа\nq - выход с записью.\n\
x - выход без записи.\n");
}
size_t size_page_current(off_t st_size) { 
        size_t size;
        size = (size_t)sysconf(_SC_PAGE_SIZE);
        if(st_size / size == 0 && (st_size % size) > 0)
                return size;
        else if((st_size / size) > 0 && (st_size % size) == 0)
                return size * (st_size / size);
        else if((st_size / size) > 0 && (st_size % size) > 0)
                return size * (st_size / size) + size;
        return sysconf(_SC_PAGESIZE);
}

void *getline_ptr(int line, void *p) {
	void *d = p;
	for(; line > 1; line--) {
		d = memchr(d + 1, '\n', strlen(d+1));
		d++;
	}
	return d;
}

//int print_line(int first, int last, void *d) {
//	void *run = getline_ptr(first);
//	void *end = getline_ptr(last);
//	char *c;
//	if(last)
//		write(STDOUT_FILENO, run, (size_t)(end - run));
//		write(STDOUT_FILENO, "Введите команду[h - справка]--> ", 53);
//	}
//	return last;
//}

int print_line(int first, int last, void *p) {
	char *run = (char*)getline_ptr(first, p);
	if(first > last)
		last = first;
	for(int i = first; i <= last; i++) {
		do {
			write(STDOUT_FILENO, run++, 1);
		} while((int)run[0] != '\n');
		write(STDOUT_FILENO, run++, 1);
	}
	return (first > last)? first : last;
}
