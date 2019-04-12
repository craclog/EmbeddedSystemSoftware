CC = arm-none-linux-gnueabi-gcc -static
#CC = gcc

OBJS = 20131579.o reader.o processor.o writer.o util.o mode1_clock.o mode2_counter.o
SRCS = 20131579.c reader.c processor.c writer.c util.c mode1_clock.c mode2_counter.c
TARGETS = 20131579

$(TARGETS) : $(OBJS)
	$(CC) -o $(TARGETS) $(OBJS)

clean:
	rm $(OBJS) $(TARGETS)