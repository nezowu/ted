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
#include <sys/ioctl.h>

void phelp(void);
size_t size_page_current(off_t);
void *getline_p(int, void*);
int print_line(int, int, void*);
char *untoa(int, char*);

int main(int argc, char *argv[]) {
	const char *wellcom = "Введите команду[m - справка]";
	const char *profi = "\033[0;33m--> \033[0m";
	int len_profi = 15;
	int len_wellcom = 49;
	int fd, first = 0, last = 0, count_line = 1, current_line, first_range = 1, last_range = 1, range = 5;
	char *filename;
	char suff;
	char duff[8] = {0};
	struct termios saved_attr;
	struct termios set_attr;
	struct winsize w;
	int in = open("/dev/tty", O_RDONLY);
	size_t PAGE;
	struct stat buff;
	void *p, *d, *end_p, *last_line_p;
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
	for(count_line = 0; (d = memchr(d + 1, '\n', strlen(d+1))) != NULL; count_line++) {
		if(strlen(d+1) > 0)
			last_line_p = d + 1;
		else
			end_p = d + 1;
	}
	current_line = 1;
	tcsetattr(in, TCSAFLUSH, &set_attr);
//	write(STDOUT_FILENO, "\033[0;0H\033[2J", 10); //очистим экран
//	write(STDOUT_FILENO, "\033[s", 3); //запомним позицию курсора
	write(STDOUT_FILENO, wellcom, len_wellcom);
	write(STDOUT_FILENO, profi, len_profi);
	write(STDOUT_FILENO, "\n", 1);
	print_line(1, 1, p);
	write(STDOUT_FILENO, profi, len_profi);

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
			if(first > count_line)
				first = count_line;
		}
		if(suff == '$' && !first) {
			write(STDOUT_FILENO, &suff, 1);
			first = count_line;
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
			last = count_line;
			continue;
		}
		if(first && strlen(duff) > 0) {
			last = atoi(duff);
			if(last > count_line + 1)
				last = count_line;
			duff[0] = 0;
		}
		switch(suff) {
			case 'm': //справка
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n", 1);
				phelp();
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'p': //print
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n", 1);
				if(!first && !last)
					current_line = print_line(current_line, last, p);
				else
					current_line = print_line(first, last, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'k': //вверх на одну строку
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				write(STDOUT_FILENO, &suff, 1);
//				write(STDOUT_FILENO, "\n", 1);
				if(current_line < 2)
					current_line++;
				write(STDOUT_FILENO, "\n", 1);
				current_line = print_line(current_line - 1, last, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'j': //вниз на одну строку
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				write(STDOUT_FILENO, &suff, 1);
				if(current_line > count_line - 1)
					current_line--;
				write(STDOUT_FILENO, "\n", 1);
				current_line = print_line(current_line + 1, last, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
//			case 'h':
//				write(STDOUT_FILENO, "\033[1D", 4);
//				break;
//			case 'l':
//				write(STDOUT_FILENO, "\033[1C", 4);
//				break;
//			case 'b': //установка блока, по умолчанию 5 строк
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\nУстановлен новый размер блока - Space\n", 41 + 24);
				range = first - 1;
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case '\040': // клавиша Space
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				if(first > 0)
					range = first - 1;
				write(STDOUT_FILENO, "Space", 5);
				write(STDOUT_FILENO, "\n", 1);
				ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
				if(range > w.ws_row - 3)
					range = w.ws_row - 3;
				first_range = (first_range < count_line)? current_line + 1: current_line;
				last_range = current_line + range;
				if(last_range > count_line)
					last_range = count_line;
				current_line = print_line(first_range, last_range, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			default:
//				write(STDOUT_FILENO, "\033[u\033[0J", 7); //восстановим позицию курсора
//				write(STDOUT_FILENO, wellcom, len_wellcom);
//				write(STDOUT_FILENO, profi, len_profi);
				write(STDOUT_FILENO, "?", 1);
				write(STDOUT_FILENO, "\n", 1);
				write(STDOUT_FILENO, profi, len_profi);
				break;
		}
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
	write(STDOUT_FILENO, "\n", 1);
	exit(EXIT_SUCCESS);
}

void phelp(void) {
	printf("%s\n", "p - вывод текущей строки\n\
1-$p - вывод с 1 по последнюю строку\n\
4-8p - вывод с 4 по 8 строку\n\
j - вывод следующей строки\n\
k - вывод предыдущей строки\n\
Space - вывод блока строк, по умолчанию 5\n\
(N)b - установка размера блока (напр: --> 25b)\n\
a - добавление в конец файла.\n\
i - вставка текста\nd - удалить текущую строку\n\
c - замена строки.\n\
w [filename] - сохранение документа\nq - выход с записью.\n\
q - выход с сохранением\n\
z - выход без сохранения.\n\
m - справка");
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

void *getline_p(int line, void *p) {
	void *d = p;
	for(; line > 1; line--, d++) {
		d = (char *)memchr(d, '\n', strlen(d));
	}
	return d;
}

int print_line(int first, int last, void *p) {
	char *buff = malloc(8);
	char *d;
	char *run = getline_p(first, p);
	if(first > last)
		last = first;
	for(int i = first; i <= last; i++) {
		write(STDOUT_FILENO, "\033[1;33m", 7);
		d = untoa(i, buff);
		write(STDOUT_FILENO, d, strlen(d));
		write(STDOUT_FILENO, "\033[0m ", 5);
		do {
			if(run[0] == '\n')
				break;
			write(STDOUT_FILENO, run++, 1);
		} while(run[0] != '\n' && run[0] != '\0');
		write(STDOUT_FILENO, run++, 1);
	}
	free(buff);
	return (first > last)? first : last;
}

char *untoa(int digit, char *buff) {
	buff += 7;
	do {
		*--buff = digit % 10 + '0';
	} while(digit /= 10);
	return buff;
}
