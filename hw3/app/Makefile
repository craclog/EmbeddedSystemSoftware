CC = arm-none-linux-gnueabi-gcc -static
#CC = gcc

OBJS = app.o 
SRCS = app.c
TARGETS = app

$(TARGETS) : $(OBJS)
	$(CC) -o $(TARGETS) $(OBJS)

clean:
	rm $(OBJS) $(TARGETS)