.PHONY : clean
P=test
CFLAGS= -Wall -O0
OBJECTS=
LIBS=
$(P) : $(P).c
	$(CC) $(CFLAGS) $< -o $@
clean :
	rm -f $(P) *~ .*~ .*.swp
