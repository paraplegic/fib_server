CODE=fib.c task.c queue.c
HDRS=task.h queue.h
BKUP=fib_server.tgz
EXES=fib q_test client
CC=gcc
CCOPTS=-g
NOWARN=-Wno-pointer-to-int-cast -Wno-int-to-pointer-cast

all:	$(EXES)

fib:	fib.c task.o q.o lst.o
	$(CC) $(CCOPTS) $(NOWARN) -DTEST fib.c -pthread -o $@ task.o q.o lst.o
	size $@

task.o:	task.c task.h
	$(CC) $(CCOPTS) -c task.c

fib.o:	fib.c fib.h
	$(CC) $(CCOPTS) $(NOWARN) -c fib.c

q.o:	q.c q.h
	$(CC) $(CCOPTS) -c q.c

lst.o:	lst.c lst.h
	$(CC) $(CCOPTS) -c lst.c

clean:
	rm -rf $(EXES) *.o
	rm -rf ../$(BKUP)

backup:	clean
	(cd .. ; tar -zcvf $(BKUP) ./fib )

q_test:	q_test.c q.o task.o
	$(CC) $(CCOPTS) q_test.c task.o q.o -pthread -o $@
	size $@

client.o:	client.c
	$(CC) $(CCOPTS) -c client.c

client:	client.o fib.o task.o client.o lst.o
	$(CC) $(CCOPTS) client.o task.o fib.o q.o lst.o -o client

test:	q_test fib client
	./fib 2222 &
	time ./client localhost 2222 &
	time ./client localhost 2222 &
	time ./client localhost 2222 &
	time ./client localhost 2222 &
	time ./client localhost 2222 &
##	time ./q_test 10000000

kill:	
	killall fib

status:	clean
	git status
