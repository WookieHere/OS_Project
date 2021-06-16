CC	= gcc
LDFLAGS	= -lm -lnsl 
CFLAGS	= -g
TARGET	= server client

default: $(TARGET)

server: server.o
	gcc $(CFLAGS) -o $@ $? $(LDFLAGS)

client: client.o
	gcc $(CFLAGS) -o $@ $? $(LDFLAGS)

clean:
	-rm -f *.o *~

cleanall: clean
	-rm -f $(TARGET) 


