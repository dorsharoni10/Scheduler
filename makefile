HEADER_PATHS_FLAGS= -iquote../task/ -iquote../heap/ -iquote../vector/\
					-iquote../priority_queue/ -iquote../uid/ \
					 

HEADERS = scheduler.h ../task/task.h ../uid/uid.h \
		 ../priority_queue/priority_queue.h \
		 ../heap/heap.h ../vector/vector.h

OBJECTS= test_scheduler.o scheduler.o ../priority_queue/priority_queue.o \
		 ../task/task.o ../uid/uid.o ../heap/heap.o ../vector/vector.o

CC= gc
CFLAGS= -ansi -pedantic-errors -Wall -Wextra -g $(HEADER_PATHS_FLAGS)

a.out : $(HEADERS) $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

scheduler.o : $(HEADERS) scheduler.c
	$(CC) $(CFLAGS) -c scheduler.c

test_scheduler.o : $(HEADERS) test_scheduler.c
	$(CC) $(CFLAGS) -c test_scheduler.c

priority_queue.o : $(HEADERS) priority_queue.c
	$(CC) $(CFLAGS) -c priority_queue.c

heap.o : $(HEADERS) heap.c
	$(CC) $(CFLAGS) -c heap.c

uid.o : ../uid/uid.h ../uid/uid.c
	$(CC) $(CFLAGS) -c ../uid/uid.c

task.o : ../task/task.h ../task/task.c
		$(CC) $(CFLAGS) -c ../task/task.c

vector.o : ../vector/vector.h ../vector/vector.c
		$(CC) $(CFLAGS) -c ../vector/vector.c


.PHONY : clean
clean :
	rm *.o a.out 



