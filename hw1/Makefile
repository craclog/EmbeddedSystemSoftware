CC = arm-none-linux-gnueabi-gcc -static
#CC = gcc

OBJS = 20131579.o reader.o processor.o writer.o \
	mode1_clock.o mode2_counter.o mode3_texteditor.o \
	mode4_drawboard.o mode5_timer.o
SRCS = 20131579.c reader.c processor.c writer.c \
	mode1_clock.c mode2_counter.c mode3_texteditor.c \
	mode4_drawboard.c mode5_timer.c
TARGETS = 20131579

$(TARGETS) : $(OBJS)
	$(CC) -o $(TARGETS) $(OBJS)

clean:
	rm $(OBJS) $(TARGETS)