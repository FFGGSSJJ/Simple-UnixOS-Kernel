# Makefile C to ASM
# Aamir Hasan
# 8/30/20


PROG := search

CFLAGS += -m32 -Wall -std=c99

.PHONY: clean

all: CFLAGS += -O0
all: $(PROG)

debug: CFLAGS += -O0 -g
debug: $(PROG)

$(PROG): search_asm.o search.o utilities.o main.o
	 $(CC) -m32 $^ -o $@

search_asm.o: search_asm.S
	$(CC) $(CFLAGS) -c $< -o $@

search.o: search.c
	$(CC) $(CFLAGS) -c $< -o $@

utitilites.o: utilities.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROG)
