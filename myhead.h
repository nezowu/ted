#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <stdio_ext.h>
#include <errno.h>

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

char *untoa(int, char *); //перовод чисел в строку
char *getline_p(size_t line, char *); //получить указатель на строку под номером
void get_count(Block*); //посчитать строки
size_t get_size(size_t); //получить требуемый размер буфера с перекрытием в 1 страницу
void print_line(Block*); //вывод строки на печать
void phelp(void); //вывод справки
void del_line(Block*); //удаление строки
void ins_line(Block*, File*); //вставка строки
