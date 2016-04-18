CODE=fib.c task.c queue.c
HDRS=task.h queue.h
BKUP=fib_server.tgz
EXES=fib q_test
CC=gcc
CCOPTS=-g

all:	$(EXES)

fib:	fib.c task.o q.o
	$(CC) $(CCOPTS) fib.c -o $@ task.o q.o -pthread
	size $@

task.o:	task.c task.h
	$(CC) $(CCOPTS) -c task.c

q.o:	q.c q.h
	$(CC) $(CCOPTS) -c q.c

clean:
	rm -rf $(EXES) *.o
	rm -rf ../$(BKUP)

backup:	clean
	(cd .. ; tar -zcvf $(BKUP) ./fib )

q_test:	q_test.c q.o task.o
	$(CC) $(CCOPTS) q_test.c task.o q.o -pthread -o $@
	size $@

test:	q_test
	time ./q_test 10000000

status:	clean
	git status
