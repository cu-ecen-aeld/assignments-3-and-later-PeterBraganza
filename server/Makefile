# Make file for Assignment 5
# used https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

#selecting complier
CROSS_COMPILE=
CC=gcc

#selecting Targer file
TARGET = aesdsocket

#setting flags
CFLAGS  = -Werror -Wall

#selecting all needed source files
SRC_FILES = *.c

all: aesdsocket
#clean:
#	rm -f *.o writer

$(TARGET): $(TARGET).c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $(SRC_FILES) -o $(TARGET)
	
	
clean:
	rm -f *.o $(TARGET)
	
	

