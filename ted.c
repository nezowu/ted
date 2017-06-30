#include "myhead.h"

int main(int argc, char *argv[]) {
	Block t = {NULL, 0, 0, 0, 1, NULL, 0, 0, 1};
	File f;
	char suff;
	char duff[8] = {0};
	char *tmp, *d;

        const char *wellcom = "Введите команду[m - справка]";
        const char *profi = "\033[0;33m--> \033[0m";
        int len_profi = 15;
        int len_wellcom = 49;
	size_t sparta = 1, range = 5;
	size_t tmp_size;

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
    	print_line(&t);
        write(STDOUT_FILENO, profi, len_profi);

        while(read(f.in, &suff, 1) == 1) {
		if(suff == '\177') {
			write(STDOUT_FILENO, "\033[1G\033[2K", 8);
			write(STDOUT_FILENO, profi, len_profi);
			t.first = 0;
			t.last = 0;
			memset(duff, 0, 8);
			continue;
		}
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
			memset(duff, 0, 8);
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
			if(t.last < t.first) //поставил
				t.last = t.first;
			memset(duff, 0, 8);
                }
                switch(suff) {
                        case 'p': //print
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'j': //вниз на одну строку
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(t.current < t.count)
					t.current++;
                                print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'k': //вверх на одну строку
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(t.current > 1)
					t.current--;
                                print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
                        case '\040': // клавиша Space
				if(sparta) {
					write(STDOUT_FILENO, "Space", 5); 
					write(STDOUT_FILENO, "\n", 1); 
				}
                                if(t.first > 0)
                                       range = t.first;
                                if(t.current < t.count)
                                        t.current++;
				t.first = t.current;
				t.last = t.current + range - 1;
				if(t.last > t.count)
					t.last = t.count;
                                print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
                        case 'c':
				if(t.last) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
					break;
				}
				if(t.first)
					t.current = t.first;
				else
					t.first = t.current;
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, " [заменить]\033[0;36m\n", 27);
				}
				del_line(&t);
				get_count(&t);
				t.current = t.first;
				ins_line(&t, &f);
				get_count(&t);
				tmp_size = get_size(t.len);
				if(t.size != tmp_size) {
					t.size = tmp_size;
					t.p = realloc(t.p, tmp_size);
				}
				if(sparta) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
				}
                                break;
			case 'D': //групповушка
				if(t.last) {
					t.current = t.first;
					while(t.first <= t.last) {
						del_line(&t);
						get_count(&t);
						t.last--;
					}
				}
				else if(t.first)
					t.current = t.first;
                        case 'd':
				if(suff == 'd' && t.first) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
					break;
				}
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, " [удалино]\n", 18);
				}
				if(!t.last) {
					del_line(&t);
					get_count(&t);
				}
				tmp_size = get_size(t.len);
				if(t.size != tmp_size) {
					t.size = tmp_size;
					t.p = realloc(t.p, tmp_size);
				}
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'A':
				if(t.first) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
					break;
				}
				t.current = t.count;
			case 'a':
				if(t.last) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
					break;
				}
				if(t.first) {
					t.current = t.first;
					t.first = 0;
				}
				t.current++;
			case 'i':
				if(t.last) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
					break;
				}				
				if(t.first)
					t.current = t.first;
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					if(suff == 'a' || suff == 'A')
						write(STDOUT_FILENO, " [добавить]", 19);
					else
						write(STDOUT_FILENO, " [вставить]", 19);
				}
				write(STDOUT_FILENO, "\n\033[0;36m", 8);
				ins_line(&t, &f);
				get_count(&t);
				tmp_size = get_size(t.len);
				if(t.size != tmp_size) {
					t.size = tmp_size;
					t.p = realloc(t.p, tmp_size);
				}
				write(STDOUT_FILENO, "\033[0m\n", 5);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'q':
				if(sparta)
					write(STDOUT_FILENO, &suff, 1);
                                write(STDOUT_FILENO, " [выход без сохранения - q, сохранить - w]\n", 70);
                                write(STDOUT_FILENO, profi, len_profi);
				read(f.in, &suff, 1);
                                if(suff == 'q') {
					write(STDOUT_FILENO, &suff, 1);
                                        goto stop;
				}
                                if(suff != 'w') {
					write(STDOUT_FILENO, &suff, 1);
                                        break;
				}
				t.last = 10;
                        case 'w':
				if(sparta)
					write(STDOUT_FILENO, &suff, 1);
                                if(f.d > 0) {
                                        lseek(f.d, 0L, SEEK_SET);
                                        write(f.d, t.p, t.len);
                                        ftruncate(f.d, t.len);
					write(STDOUT_FILENO, " [записано]", 19);
                                }
				else {
                                        write(STDOUT_FILENO, " [введите имя файла]\n", 36);
                                        write(STDOUT_FILENO, profi, len_profi);
                                        tmp = malloc(128);
                                        while(f.d <= 0) {
                                        	d = tmp;
                                                while(read(f.in, &suff, 1) && suff != '\004') {
							if(!isprint(suff))
								continue;
                                                        write(STDOUT_FILENO, &suff, 1);
                                                        memcpy(d++, &suff, 1);
                                                }
                                                if((f.d = open(tmp, O_RDWR|O_CREAT|O_EXCL, 0664)) == -1) {
                                                        if(errno == EEXIST) {
                                                                write(STDERR_FILENO, " [файл с таким именем существует]\n", 60);
								write(STDOUT_FILENO, profi, len_profi);
                                                                write(STDERR_FILENO, "[введите новое имя файла]\n", 46);
                                                                write(STDOUT_FILENO, profi, len_profi);
								continue;
                                                        } else {
                                                                perror("\n");
                                                                write(STDERR_FILENO, "[не удалось записать файл]", 49);
								break;
                                                        }
                                                }
						memset(tmp, 0, strlen(tmp));
                                        }
                                        write(f.d, t.p, t.len);
					if(f.d > 0)
						write(STDOUT_FILENO, " [записано] ", 20);
					write(STDOUT_FILENO, tmp, strlen(tmp));
                                        free(tmp);
				}
				if(t.first == 0 && t.last == 10)
					goto stop;
				write(STDOUT_FILENO, "\n", 1);
				write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'n': //нумерация строк
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
			case 's': //спартанский интерфейс
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
                        case 'm': //справка
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
                                phelp();
                                write(STDOUT_FILENO, profi, len_profi);
                                break;
			default:
				break;
		}
		t.first = 0;
		t.last = 0;
//		memset(duff, 0, 8);
	}
stop:
	write(STDOUT_FILENO, "\n", 1);
	tcsetattr(f.in, TCSANOW, &saved_attr);
	exit(EXIT_SUCCESS);
}
