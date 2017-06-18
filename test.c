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
#include <errno.h>

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
	int fd, fdb, first = 0, last = 0, count_line = 1, current_line, first_range = 1, last_range = 1, range = 4;
	char *filename, *swapfile;
	char suff, *ins, *tmp;
	char duff[8] = {0};
	struct termios saved_attr;
	struct termios set_attr;
	struct winsize w;
	int in = open("/dev/tty", O_RDONLY);
	size_t PAGE;
	struct stat staff;
	void *p, *d, *end_p, *last_line_p;
	tcgetattr(in, &saved_attr);
	set_attr = saved_attr;
	set_attr.c_lflag &= ~(ICANON|ECHO|ISIG);
	set_attr.c_cc[VMIN] = 1;
	set_attr.c_cc[VTIME] = 0;

	if(argc == 1) {
		swapfile = ".file~";
		staff.st_size = 0;
	} else if(argc == 2) { //копируем в .argv[1]~
		swapfile = malloc(strlen(argv[1] + 2));
		memcpy(swapfile, ".", 1);
		memcpy(swapfile + 1, argv[1], strlen(argv[1]));
		memcpy(swapfile + 1 + strlen(argv[1]), "~", 1);
		filename = argv[1];
		fd = open(filename, O_RDWR|O_CREAT, 0664);
		if(fd == -1) {
			perror("open:61");
			exit(EXIT_FAILURE);
		}
		if(stat(argv[1], &staff) == -1) {
			perror("stat");
			exit(EXIT_FAILURE);
		}
	} else {
		write(STDERR_FILENO, "Использование: ted [имя_файла.txt]\n", 56);
		exit(EXIT_FAILURE);
	}
	fdb = open(swapfile, O_RDWR|O_CREAT|O_TRUNC, 0664);
	if(fdb == -1) {
		perror("open:72");
		exit(EXIT_FAILURE);
	}
	if(!staff.st_size){
		staff.st_size = 1;
		write(fdb, "\n", 1);
	} else {
		tmp = malloc(staff.st_size);
		read(fd, tmp, staff.st_size);
		write(fdb, tmp, staff.st_size);
		free(tmp);
	}
	PAGE = size_page_current(staff.st_size);

	p = mmap(0, PAGE, PROT_READ|PROT_WRITE, MAP_SHARED, fdb, 0);
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

	while(read(in, &suff, 1) == 1) { //нужна проверка read
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
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n", 1);
				phelp();
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'p': //print
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n", 1);
				if(!first && !last)
					current_line = print_line(current_line, last, p);
				else
					current_line = print_line(first, last, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'k': //вверх на одну строку
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n", 1);
				if(current_line < 2)
					current_line++;
				current_line = print_line(current_line - 1, last, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'j': //вниз на одну строку
				write(STDOUT_FILENO, &suff, 1);
				if(current_line > count_line - 1)
					current_line--;
				write(STDOUT_FILENO, "\n", 1);
				current_line = print_line(current_line + 1, last, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case '\040': // клавиша Space
				if(first > 0)
					range = first - 1;
				write(STDOUT_FILENO, "Space", 5);
				write(STDOUT_FILENO, "\n", 1);
				ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
				if(range > w.ws_row - 3)
					range = w.ws_row - 3;
				if(current_line < count_line)
					current_line++;
				first_range = current_line;
				last_range = current_line + range;
				if(last_range > count_line)
					last_range = count_line;
				current_line = print_line(first_range, last_range, p);
				write(STDOUT_FILENO, profi, len_profi);
				break;
//			case 'h':
//				write(STDOUT_FILENO, "\033[1D", 4);
//				break;
//			case 'l':
//				write(STDOUT_FILENO, "\033[1C", 4);
//				break;
			case 'A':
				current_line = count_line;

			case 'a':
				current_line++;
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, " [добавить]\n", 20);
				if(current_line > count_line) {
					while(read(in, &suff, 1) && suff != '\004') {
						if(!isprint((int)suff) && !isspace((int)suff))
							continue;
						write(STDOUT_FILENO, &suff, 1);
						fallocate(fdb, 0, 0, ++staff.st_size); //staff.st_size < PAGE
						memcpy(end_p++, &suff, 1);
						memcpy(end_p, "\n", 1);
					}
					fallocate(fdb, 0, 0, ++staff.st_size); //staff.st_size < PAGE
					memcpy(end_p++, "\n", 1);
					ftruncate(fdb, strlen(p));

					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);

					d = p - 1;
					for(count_line = 0; (d = memchr(d + 1, '\n', strlen(d+1))) != NULL; count_line++) {
						if(strlen(d+1) > 0)
							last_line_p = d + 1;
						else
							end_p = d + 1;
					}
					break;
				}
			case 'i':
				if(suff == 'i')
					write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, " [вставить]\n", 20);
				ins = getline_p(current_line, p);
				tmp = calloc(1, (size_t)sysconf(_SC_PAGESIZE));
				memcpy(tmp, ins, strlen(ins));
				while(read(in, &suff, 1) && suff != '\004') {
					if(!isprint((int)suff) && !isspace((int)suff))
						continue;
					write(STDOUT_FILENO, &suff, 1);
					fallocate(fdb, 0, 0, ++staff.st_size); //staff.st_size < PAGE
					memcpy(ins++, &suff, 1);
					memcpy(ins, "\n", 1);
				}
				ins++;
				fallocate(fdb, 0, 0, ++staff.st_size); //staff.st_size < PAGE
				memcpy(ins, tmp, strlen(tmp));
				free(tmp);
				write(STDOUT_FILENO, "\n", 1);
				write(STDOUT_FILENO, profi, len_profi);


				d = p - 1;
				for(count_line = 0; (d = memchr(d + 1, '\n', strlen(d+1))) != NULL; count_line++) {
					if(strlen(d+1) > 0)
						last_line_p = d + 1;
					else
						end_p = d + 1;
				}
				break;
			case 'q': //нужна проверка свопа и файла на время доступа своп < файл для выхода без подтверждения
				write(STDOUT_FILENO, &suff, 1);
				write(STDOUT_FILENO, "\n[выход без сохранения - q, сохранить - w]\n", 70);
				write(STDOUT_FILENO, profi, len_profi);
				if(read(in, &suff, 1) && suff == 'q') //не нужен break
					goto stop;
				if(suff != 'w')
					break;
			case 'w':
				write(STDOUT_FILENO, &suff, 1);
				if(fd > 0) {
					lseek(fd, 0L, SEEK_SET);
					write(fd, p, strlen(p));
				} else {
					write(STDOUT_FILENO, " [введите имя файла]\n", 36); 
					write(STDOUT_FILENO, profi, len_profi);
					while(fd <= 0) {
						tmp = malloc(1024);
						d = tmp;
						while(read(in, &suff, 1) && suff != '\004') {
							write(STDOUT_FILENO, &suff, 1);
							memcpy(d++, &suff, 1);
						}
						if((fd = open(tmp, O_RDWR|O_CREAT|O_EXCL, 0664)) == -1) {
							if(errno == EEXIST) {
								write(STDERR_FILENO, "[файл с таким именем существует]\n", 59);
								write(STDERR_FILENO, "[введите новое имя файла]\n", 46);
								write(STDOUT_FILENO, profi, len_profi);
							} else {
								perror("\n");
								write(STDERR_FILENO, "[не удалось записать файл]\n", 49);
								write(STDOUT_FILENO, profi, len_profi);
							}
						}
					free(tmp);
					}
					write(fd, p, strlen(p));
				}
				write(STDOUT_FILENO, " [записано]\n", 20);
				write(STDOUT_FILENO, wellcom, len_wellcom);
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

	//удалить своп файл до метки stop
stop:
	close(fdb);
	close(fd);
	tcsetattr(in, TCSANOW, &saved_attr); //наверное нужно обернуть в функцию и добавить через atexit чтобы избавится от goto в switch
	write(STDOUT_FILENO, "\n", 1);
	exit(EXIT_SUCCESS);
}

void phelp(void) {
	printf("%s\n", "p - вывод текущей строки\n\
1-$p - вывод с 1 по последнюю строку. \
4-8p - вывод с 4 по 8 строку\n\
j - вывод следующей строки\n\
k - вывод предыдущей строки\n\
Space - вывод блока строк, по умолчанию 5\n\
nSpace - установка размера блока [напр: --> 25Space]\n\
a - добавление с новой строки.\n\
A - добавление в конец файла с новой строки\n\
i - вставка текста\nd - удалить текущую строку\n\
Ctrl+D - закончить ввод текста.\n\
c - замена строки.\n\
w - сохранение документа\nq - выход с записью.\n\
q - выход с сохранением\nz - выход без сохранения.\n\
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
	char *buff = calloc(1, 8);
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
