# Make file for Assignment 2
# used https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html

#selecting complier
CC = gcc

#selecting Targer file
TARGET = writer

#setting flags
CFLAGS  = -Werror -Wall

#selecting all needed source files
SRC_FILES = *.c

clean:
	rm -f writer *.o 

SRCR: ${SRC_FILES}
	${CC} ${CFLAGS} ${SRC_FILES} -o ${TARGET}
