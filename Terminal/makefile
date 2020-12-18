PHONY=clean run

# change application name here (executable output name)
TARGET=terminal

# compiler
CC=gcc
# debug
DEBUG=-g
# optimisation
OPT=-O0
# warnings
WARN=-Wall
# ptheads library
PTHREAD=-pthread
# dinamic compilation
DYNAMIC=-export-dynamic

CCFLAGS=$(OPT) $(WARN) $(PTHREAD) $(DYNAMIC) -pipe

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

# linker
# LDFLAGS=$(PTHREAD) $(GTKLIB)

SRCS=src/main.c src/disk_int.c

# OBJS=main.o

all: $(SRCS)
	$(CC)  $(SRCS) -o $(TARGET) $(CCFLAGS) $(GTKLIB)
    
# $(OBJS): $(SRCS)
# 	$(CC) -c $(CCFLAGS) $(SRCS) -o $(OBJS)

run: $(TARGET)
	./$(TARGET)
    
clean:
	rm -f *.o $(TARGET)