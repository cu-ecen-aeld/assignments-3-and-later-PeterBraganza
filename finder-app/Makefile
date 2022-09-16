# Make file for Assignment 2
# used https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

#selecting complier
CC = gcc

CROSS_COMPILE =

#selecting Targer file
TARGET = writer

#setting flags
CFLAGS  = -Werror -Wall

#selecting all needed source files
SRC_FILES = *.c

all: writer
#clean:
#	rm -f *.o writer

SRCR: ${SRC_FILES}
	${CROSS_COMPILE}${CC} ${CFLAGS} ${SRC_FILES} -o ${TARGET}
	
	
clean:
	rm -f *.o writer
	
	

