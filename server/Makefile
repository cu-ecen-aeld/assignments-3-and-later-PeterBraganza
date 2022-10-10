# Make file for Assignment 5
# used https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

#selecting complier
CROSS_COMPILE=
CC ?=$(CROSS_COMPILE)gcc

#selecting Targer file
TARGET ?= aesdsocket

#setting flags
CFLAGS  ?= -Werror -Wall
LDFLAGS ?= -pthread -lrt

#selecting all needed source files
SRC_FILES = $(TARGET).c

all: $(TARGET)
#clean:
#	rm -f *.o writer

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(SRC_FILES) -o $(TARGET) $(LDFLAGS)
clean:
	rm -f *.o $(TARGET)
	
	

