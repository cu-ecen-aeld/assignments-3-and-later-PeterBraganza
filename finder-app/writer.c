/*
@file: writer.c
@breif: writer application creates a file and writes a const string to it
@description: Application takes two arguments. First is the full path including 
			  file name. Second is string to be written. It creates a file and 
			  replaces all content in the file with the string.
@date: 9/3/22
@author:Peter Braganza
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

/*
@brief: Prints the usage of the writer applicatin
@input: Void
@return: void

*/
void usage()
{
	printf("Number of arguments entered are not correct\n"
			"writer [file path with file name] [String to write to file]\n"
			);
}

/*
@breif: writer application creates a file and writes a const string to it
@description: Application takes two arguments. First is the full path including 
			  file name. Second is string to be written. It creates a file and 
			  replaces all content in the file with the string.
@input: input from command line
@return: int
		1: error
		0: success
*/
int main( int argc, char *argv[] )  {
   
    openlog(NULL, LOG_NOWAIT, LOG_USER);
    
    /*Checks the number of  arguments entered. Error if args is not three*/
	if ( argc != 3 )
	{
		usage();
		syslog(LOG_ERR,"Wrong usage of writer application");
		return 1;
	}
   
   
   syslog(LOG_ERR, "Error Occured while trying to write to file, Errno: %s", strerror(errno));
	/*Store first argument as file with has full path and file name*/
	const char *file = argv[1];
	/*Store String to be written to file*/
	const char *string = argv[2];
	
	//Creates file which is eadable by all the user groups, but writable by the user only
	int fd = creat (file, 0644);
	if (fd == -1)
		syslog(LOG_ERR, "Error Occured while trying to create file, Errno: %s", strerror(errno));

    
    ssize_t bytes_written = 0;
    int len = strlen(string);
    
	while ( len != 0 )
	{
		bytes_written = write (fd, string , len);
		
        if (bytes_written == -1) {
        	//Check if error was due to an interrupt
            if (errno == EINTR)
                continue;
            syslog(LOG_ERR, "Error Occured while trying to write to file, Errno: %s", strerror(errno));
            break;
        }
        
        len -= bytes_written;
        string = string + bytes_written;
	
	}

	
	//Close file
	if (close (fd) == -1)
        syslog(LOG_ERR, "Error Occured while trying to close file, Errno: %s", strerror(errno));
        
    //Close system log
    closelog();

	return 0;
}
