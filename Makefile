CODE=fib.c task.c queue.c
HDRS=task.h queue.h
BKUP=fib_server.tgz
CC=gcc
CCOPTS=-g

all:	fib

fib:	fib.c task.o q.o
	$(CC) $(CCOPTS) fib.c -o $@ task.o q.o -pthread

task.o:	task.c task.h
	$(CC) $(CCOPTS) -c task.c

q.o:	q.c q.h
	$(CC) $(CCOPTS) -c q.c

clean:
	rm -rf fib *.o
	rm -rf $(BKUP)

backup:	clean
	(cd .. ; tar -zcvf $(BKUP) ./fib )

status:	clean
	git status
