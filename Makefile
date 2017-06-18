.PHONY : clean, install, uninstall
P=ted
CFLAGS= -Wall -O0
OBJECTS=
LIBS=
$(P) : $(P).c
	$(CC) $(CFLAGS) $< -o $@
clean :
	rm -f $(P) *~ .*~ .*.swp
install :
	mv $(P) /usr/local/bin/

uninstall :
	rm /usr/local/bin/$(P)
