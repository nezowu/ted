#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

typedef struct block {
	char *p;
	size_t first;
	size_t count;
	size_t last;
	size_t current;
	char *endp;
	size_t len;
	size_t size;
	size_t num;
} Block;

typedef struct file {
	int d;
	int ds;
	int in;
} File;

size_t SIZE;

char *untoa(int, char *);
char *getline_p(size_t line, char *);
void get_count(Block*);
size_t get_size(size_t);
size_t print_line(Block*);
void phelp(void);

int main(int argc, char *argv[]) {
	SIZE = sysconf(_SC_PAGESIZE);
	Block t = {NULL, 0, 0, 0, 1, NULL, 0, 0, 1};
	File f;
	char suff;
	char duff[8] = {0};

        const char *wellcom = "Введите команду[m - справка]";
        const char *profi = "\033[0;33m--> \033[0m";
        int len_profi = 15;
        int len_wellcom = 49;
	size_t sparta = 1;

        struct stat staff;
        struct termios saved_attr;
        struct termios set_attr;

        f.in = open("/dev/tty", O_RDONLY);
        tcgetattr(f.in, &saved_attr);
        set_attr = saved_attr;
        set_attr.c_lflag &= ~(ICANON|ECHO|ISIG);
        set_attr.c_cc[VMIN] = 1;
        set_attr.c_cc[VTIME] = 0;
	tcsetattr(f.in, TCSAFLUSH, &set_attr);

        if(argc == 1) {
		t.len = 0;
		t.size = get_size(t.len);
		t.p = calloc(1, t.size);
        }
	else if(argc == 2){
                f.d = open(argv[1], O_RDWR|O_CREAT, 0664);
                stat(argv[1], &staff);
		t.len = staff.st_size;
		t.size = get_size(t.len);
		t.p = calloc(1, t.size);
		read(f.d, t.p, t.len);
		
        }
	else {
                write(STDERR_FILENO, "Использование: ted [имя_файла.txt]\n", 56);
                exit(EXIT_FAILURE);
        }
	get_count(&t);

        write(STDOUT_FILENO, wellcom, len_wellcom);
        write(STDOUT_FILENO, profi, len_profi);
        write(STDOUT_FILENO, "\n", 1);
    	t.current = print_line(&t);
        write(STDOUT_FILENO, profi, len_profi);

        while(read(f.in, &suff, 1) == 1 && suff != '\004') {
                if(isdigit(suff) && !t.first ) { 
                        if(suff == '0' && strlen(duff) < 1)
                                continue;
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        strncat(duff, &suff, 1); 
                        continue;
                }
                if(strlen(duff) > 0 && !t.first) {
                        t.first = atoi(duff);
                        duff[0] = 0;
                        if(t.first > t.count)
                                t.first = t.count;
                }
                if(suff == '$' && !t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        t.first = t.count;
                        continue;
                }
                if( suff == '-' && t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        continue;
                }
                if(isdigit((int)suff) && t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        strncat(duff, &suff, 1); 
                        continue;
                }
                if(suff == '$' && t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1);
                        t.last = t.count;
                        continue;
                }
                if(t.first && strlen(duff) > 0) {
                        t.last = atoi(duff);
                        if(t.last > t.count)
                                t.last = t.count;
			memset(duff, 0, 8);
                }
                switch(suff) {
                        case 'm': //справка
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
                                phelp();
                                write(STDOUT_FILENO, profi, len_profi);
                                break;
                        case 'p': //print
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				t.current = print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'n': //вывод номеров строк
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(t.num)
					t.num = 0;
				else
					t.num = 1;
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
				break;
			case 's': //строго, только текст
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(sparta)
					sparta = 0;
				else
					sparta = 1;
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
				break;
		}
		t.first = 0;
		t.last = 0;
	}

	tcsetattr(f.in, TCSANOW, &saved_attr);
	exit(EXIT_SUCCESS);
}
void get_count(Block *t) {
	if(t->len > 0) {
		t->endp = t->p;
		size_t leng = t->len;
		for(t->count = 1; (t->endp = memchr(t->endp, '\n', leng)) != NULL; t->count++) {
			leng = leng - (t->endp - t->p);
			t->endp++;
		}
	}
	t->endp = t->p + t->len;
}

size_t get_size(size_t len) {
	return (len + SIZE) / SIZE * SIZE + SIZE;
}

char *getline_p(size_t line, char *p) {
	char *d = p;
	for(; line > 1; line--, d++) {
		d = (char *)memchr(d, '\n', strlen(d));
	}
	return d;
}

char *untoa(int digit, char *buff) {
	buff += 7;
	do {
		*--buff = digit % 10 + '0';
	} while(digit /= 10);
	return buff;
}

size_t print_line(Block *t) {
	if(t->len == 0)
		return 0;
	int i = 1;
	if(t->first == 0)
		i = t->current;
	if(t->first > i)
		i = t->first;
        if(i > t->last)
                t->last = i;
        char *buff = calloc(1, 8);
        char *d;
        char *run = getline_p(i, t->p);
        for(; i <= t->last; i++) {
		if(t->num) {
			d = untoa(i, buff);
			write(STDOUT_FILENO, "\033[1;33m", 7);
			write(STDOUT_FILENO, d, strlen(d));
                	write(STDOUT_FILENO, "\033[0m ", 5);
		}
                do {
                        if(run[0] == '\n')
                                break;
                        write(STDOUT_FILENO, run++, 1);
                } while(run[0] != '\n' && run[0] != '\0');
                write(STDOUT_FILENO, run++, 1);
        }
        free(buff);
        return t->last;
}

void phelp(void) {
        printf("%s\n", "p - вывод текущей строки\n\
 1-$p - вывод с 1 по последнюю строку \n\
 4-8p - вывод с 4 по 8 строку\n\
j - вывод следующей строки\n\
k - вывод предыдущей строки\n\
Space - вывод блока строк, по умолчанию 5\n\
 <размер>Space - установка размера блока\n\
a - добавление с новой строки\n\
A - добавление в конец файла\n\
i - вставка текста\n\
d - удалить текущую строку\n\
 <интервал>D - групповое удаление строк\n\
Ctrl+d - закончить ввод текста\n\
c - замена строки\n\
w - сохранение документа\n\
q - выход\n\
n - номера строк.\n\
s - спартанский режим\n\
m - справка");
}
