#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main( int argc, char *argv[] )  {
   
	if ( argc != 3 )
	{
	printf("Arguments entered are wrong\n");
	//TODO: MAKe usage function

	return 1;

	}
   
   
	const char *file = argv[1];
	const char *string = argv[2];

	//check flag
	int fd = creat (file, 0644);
	if (fd == -1)
		printf("Fucked\n");

    
    ssize_t bytes_written = 0;
    int len = strlen(string);
    
    
	while ( len != 0 )
	{
		bytes_written = write (fd, string , len);
		
        if (bytes_written == -1) {
                if (errno == EINTR)
                        continue;
                perror ("write");
                break;
        }
        
        len -= bytes_written;
        string = string + bytes_written;
	
	}

	//close
	
	if (close (fd) == -1)
        perror ("close");

	return 0;
}
