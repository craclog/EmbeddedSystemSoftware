CC = gcc

all : processor reader writer
processor : processor.o
	$(CC) -o processor processor.o
reader : reader.o	
	$(CC) -o reader reader.o
writer : writer.o	
	$(CC) -o writer writer.o

clean : 
	rm *.o processor reader writer
