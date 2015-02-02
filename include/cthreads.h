/** @file cthreads.h
 *	@author Jahn Fuchs
 *
 *  cthreads wraps simple threads, mutexes, semaphores and a readers-writer_lock
 *  that can be used without changes on both posix and ms windows systems.
 */

#ifndef CTHREADS_H_
#define CTHREADS_H_

#if _MSC_VER
	#include <windows.h>
#else
    #include <pthread.h>
    #include <semaphore.h>
#endif

#if _MSC_VER
	typedef CRITICAL_SECTION c_mutex;
	typedef HANDLE c_thread;
	typedef HANDLE c_semaphore;
	#define CTHREAD_RET DWORD WINAPI
    typedef DWORD CTHREAD_RETURN;
	typedef LPTHREAD_START_ROUTINE CTHREAD_FUN;
	typedef LPVOID CTHREAD_ARG;

#else
    typedef pthread_mutex_t c_mutex;
    typedef pthread_t c_thread;
    typedef sem_t c_semaphore;
    typedef void* CTHREAD_ARG;
    typedef void* CTHREAD_RET;
    typedef void* CTHREAD_RETURN;
    typedef CTHREAD_RET(*CTHREAD_FUN)(CTHREAD_ARG);
#endif

c_thread cthread_self();
int cthread_equal( c_thread thread1, c_thread thread2 );

int cthread_create( c_thread *thread_handle, CTHREAD_FUN function, CTHREAD_ARG parameter );
int cthread_join( c_thread *thread );
// sleep for n milliseconds..
int cthread_sleep(unsigned int milliseconds);

// returns 0 on error  // only does something on unix
int cthread_detach(c_thread * thread_handle);

int cthread_mutex_init( c_mutex *mutex );
int cthread_mutex_destroy( c_mutex *mutex );
int cthread_mutex_lock( c_mutex *mutex );
int cthread_mutex_trylock( c_mutex *mutex );
int cthread_mutex_unlock( c_mutex *mutex );

// set and get default thread stacksizes (only for unix/posix threads)
size_t cthread_attr_getstacksize();
int cthread_attr_setstacksize(size_t stacksize);

// semaphore stuff
int cthread_sem_init( c_semaphore *sem, unsigned int init_val );
int cthread_sem_destroy( c_semaphore *sem );
int cthread_sem_wait( c_semaphore *sem );
int cthread_sem_trywait( c_semaphore *sem );
int cthread_sem_post( c_semaphore *sem );

// simple read write lock data type and functions
// this rw-lock and functions have a writers-preference
typedef struct {
    c_semaphore write;
    c_semaphore readers;
    unsigned int max_readers;
} c_rwlock;

int cthread_rwlock_init( c_rwlock *rwlock, unsigned int max_readers );
int cthread_rwlock_destroy( c_rwlock *rwlock );
int cthread_rwlock_read_wait( c_rwlock *rwlock );
int cthread_rwlock_read_trywait( c_rwlock *rwlock );
int cthread_rwlock_read_post( c_rwlock *rwlock );
int cthread_rwlock_write_wait( c_rwlock *rwlock );
int cthread_rwlock_write_post( c_rwlock *rwlock );
	
#endif /* CTHREADS_H_ */
