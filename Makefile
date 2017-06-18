.PHONY : clean, install, uninstall
P=ted
CFLAGS= -Wall -O0
OBJECTS=
LIBS=
$(P) : $(P).c
	$(CC) $(CFLAGS) $< -o $@
clean :
	rm -f $(P) *~ .*~ .*.swp
install : ted.1.gz
	mv $(P) /usr/local/bin/
	mv ted.1.gz /usr/share/man/man1/

ted.1.gz : ted.1
	gzip -k ted.1

uninstall :
	rm /usr/local/bin/$(P)
	rm /usr/shar/man/man1/ted.1.gz
