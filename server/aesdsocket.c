/*
*/
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MYPORT "9000"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold
#define BUF_SIZE 1024

bool signal_found = false;
bool reading = false;
int sockfd;


static void signal_handler()
{
    printf("caught signal\n");
    if(!reading)
    {
        close(sockfd);
        unlink("/var/tmp/aesdsocketdata");
        exit(0);
    }
    signal_found = true;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int search_for_return(char *buf, size_t buf_size)
{
    for( int i = 0; i < buf_size; i++)
    {
        if( *(buf+i) == '\n')
            return i+1;
    }

    return -1;
}


void write_to_file(int fd, void *buf, size_t buf_size)
{
    ssize_t bytes_written = 0;
    int len = buf_size;
    
	while ( len != 0 )
	{
		bytes_written = write (fd, buf , len);
		
        if (bytes_written == -1) {
        	//Check if error was due to an interrupt
            if (errno == EINTR)
                continue;
            syslog(LOG_ERR, "Error Occured while trying to write to file, Errno: %s", strerror(errno));
            break;
        }
        //ssize_t write(int fd, const void *buf, size_t count);
        len -= bytes_written;
        buf = buf + bytes_written;
	}
}



int main(int argc, char *argv[])
{

    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    //socklen_t addr_size;
    int new_fd;
    //struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    bool run_deamon = false;
    //char buf[BUF_SIZE];

    if(argc >= 2)
    {
        if(strcmp(argv[1],"-d")  == 0)
        {
            run_deamon = true;
        }
    }



    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        syslog(LOG_ERR, "getaddrinfo failed");
        return 1;
    }

    // make a socket:

/*
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd = -1){
        syslog(LOG_ERR,"socket not created");
    }

    // bind it to the port we passed in to getaddrinfo():

    bind(sockfd, res->ai_addr, res->ai_addrlen);*/


    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    if(run_deamon)
    {
        pid_t pid;
        /* create new process */
        pid = fork ();
        if (pid == -1)
        {
            perror("Fork");
            return -1;
        }
        else if (pid != 0)
        {
 
            exit (EXIT_SUCCESS);
        }
        else
        {
            if(setsid()==-1)
            {
                perror("Session");
                return -1;
            }
            if(chdir("/")==-1)
            {
                perror("Changing directory");
                return -1;
            }
            // redirect fd's 0,1,2 to /dev/null /
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            open ("/dev/null", O_RDWR); // stdin /
            dup (0); // stdout /
            dup (0); // stderror */
        }
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }


    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    /*addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);*/

    while(!signal_found) {  // main accept() loop

        sin_size = sizeof their_addr;
        //int num_of_bytes = 0;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            printf("in accept fail with int\n");
            if(errno == EINTR)
            {
                break;
            }
            perror("accept");
            continue;
        }
        reading = true;
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        //int local_fd = open("/var/tmp/aesdsocketdata", O_CREAT | O_RDWR | O_APPEND, S_IRWXU |  S_IRWXG | S_IRWXO );
        int local_fd = open("/var/tmp/aesdsocketdata", O_CREAT | O_RDWR | O_APPEND, 755);
        printf("file opened\n");
        int max_buf_size = BUF_SIZE;
        char *buf = (char *)malloc(max_buf_size * sizeof(char));
        int return_idx = 0;
        int num_of_relloc = 0;
        bool ret_found = false;

        while ( !ret_found ) //return not found
        {
            if(recv(new_fd, buf + (num_of_relloc * BUF_SIZE), BUF_SIZE, 0) == - 1){
                //log errors
                printf("recv() failed\n");
            }

            return_idx = search_for_return(buf + (num_of_relloc * BUF_SIZE), BUF_SIZE);
            if(return_idx == -1)
            {
                max_buf_size += BUF_SIZE;
                buf = (char *)realloc(buf, max_buf_size * sizeof(char));
                num_of_relloc++;
            }
            else
            {
                printf("writing to file\n");
                write_to_file(local_fd, buf, max_buf_size - (BUF_SIZE - return_idx));
                ret_found = true;
                break;
            }
        }

        int num_of_bytes = BUF_SIZE;
        lseek(local_fd, 0, SEEK_SET);
        char read_buf[BUF_SIZE];

        printf("Attemping to read and send BUF_SIZE\n");
        while(num_of_bytes == BUF_SIZE)
        {
            num_of_bytes = read(local_fd, read_buf, BUF_SIZE);
            printf("num of bytes: %d\n", num_of_bytes);
            send(new_fd , read_buf, num_of_bytes, 0);
        }
        printf("Sent\n");

        close(local_fd);  // parent doesn't need this
        close(new_fd);
        printf("file closed\n");
        free(buf);
        printf("buf freed\n");
        printf("signal found: %d", signal_found);
        printf("buf freed\n");
        reading = false;
    }

    close(sockfd);
    printf("server: closed connection from %s\n", s);
    unlink("/var/tmp/aesdsocketdata");
    return 0;
}







/*
 b. Opens a stream socket bound to port 9000, failing and returning -1 if any of the socket connection steps fail.

     c. Listens for and accepts a connection

     d. Logs message to the syslog “Accepted connection from xxx” where XXXX is the IP address of the connected client. 

     e. Receives data over the connection and appends to file /var/tmp/aesdsocketdata, creating this file if it doesn’t exist.
*/
