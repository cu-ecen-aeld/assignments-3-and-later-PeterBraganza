/*
 * @file: threading.c
 * @breif: contains threadfunc and start_tread_obtaining_mutex() which are used to 
 *         create a thread and obtain a mutex given the time to wait andtime to release
 * @author: Peter Braganza
 * @date: 09/14/2022
 *
 */


#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct timespec rem;
    rem.tv_nsec = 0;
    rem.tv_sec = 0;
    
    //sleeping for wait_to_obtain amount of time
    int ret = nanosleep(&thread_func_args->wait_to_obtain , &rem);
    if(ret == -1)
    {
    	syslog(LOG_ERR, "Nano sleep interrupted with error: %s", strerror(errno));
    	thread_func_args->thread_complete_success = false;
    	return thread_param;
    }
    
    //locking the mutex
    ret = pthread_mutex_lock(thread_func_args->mutex);
    if(ret != 0)
    {
    	syslog(LOG_ERR, "pthread_mutex_lock error: %s", strerror(errno));
    	thread_func_args->thread_complete_success = false;
    	return thread_param;
    }
    
    //sleeping for wait_to_release amount of time
    ret = nanosleep(&thread_func_args->wait_to_release , &rem);
    if(ret == -1)
    {
    	syslog(LOG_ERR, "Nano sleep interrupted with error: %s", strerror(errno));
    	thread_func_args->thread_complete_success = false;
    	return thread_param;
    }
    
    //releasing the mutex
    ret = pthread_mutex_unlock(thread_func_args->mutex);
    if(ret != 0)
    {
    	syslog(LOG_ERR, "pthread_mutex_unlock error: %s", strerror(errno));
    	thread_func_args->thread_complete_success = false;
    	return thread_param;
    }


    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
     
    struct thread_data* thread1 = (struct thread_data *)malloc(sizeof(struct thread_data));
    
    thread1->wait_to_obtain.tv_sec = wait_to_obtain_ms/1000;
    thread1->wait_to_obtain.tv_nsec = (wait_to_obtain_ms%1000) * 1000000;
    thread1->wait_to_release.tv_sec = wait_to_release_ms/1000;
    thread1->wait_to_release.tv_nsec = (wait_to_release_ms%1000) * 1000000;
    thread1->mutex = mutex;
    
    
    int ret = pthread_create(&thread1->tid, NULL, threadfunc, thread1);
    if (!ret)
    {
        *thread = thread1->tid;
        return true;
    }
    
    return false;
}

