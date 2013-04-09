CC	= gcc
INC1     = ./
INC2     = /usr/local/pgsql/include
INC3     = /usr/local/pgsql/include/server
#LFLAGS = -lpthread -lresolv
LIBDIR = /usr/local/pgsql/lib/
LIBS = pq

TARGET	= mp3 
SRCS    = read_header_mp3.c dbio.c
OBJS	= read_header_mp3.o dbio.o
DEFS = DEBUG

all:	$(TARGET)

test:	$(TARGET)
	./$(TARGET) -test


$(TARGET): $(OBJS)
	$(CC) $(OBJS) -L $(LIBDIR)  -l$(LIBS) -o $(TARGET)

clean:
	-rm -f $(TARGET) $(OBJS) 

.c.o:
	$(CC) -I $(INC1) -I $(INC2) -I $(INC3) -D $(DEFS)  -c $(SRCS)

