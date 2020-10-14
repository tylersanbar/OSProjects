CC = gcc
BASE = pipe_test7
CFLAGS = -Wall -c -g

all: $(BASE)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(BASE): $(BASE).o
	$(CC) $(BASE).o -o $(BASE)

clean:
	rm $(ALL)

tar:
	tar -cvf pipe.tar *.c Makefile

install:
	cp *.tar /home/fagg/notepad/classes/cs3113/export/code
