/* cranberry-server
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */

/** @file cthreads.h
 *
 *  CThreads wraps simple threads, mutexes, semaphores and a readers-writer_lock
 *  that can be used on both Posix and MS Windows systems.
 */

#ifndef CTHREADS_H_
#define CTHREADS_H_

#if _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
#else
    #include <pthread.h>
    #include <semaphore.h>
#endif

/* CThread typedefs and defines. */
#if _WIN32
    typedef CRITICAL_SECTION c_mutex;
    typedef HANDLE c_thread;
    typedef HANDLE c_semaphore;
    #define CTHREAD_RET DWORD WINAPI
	/* typedef DWORD WINAPI CTHREAD_RET; */
    typedef DWORD CTHREAD_RETURN;
    typedef LPTHREAD_START_ROUTINE CTHREAD_FUN;
    typedef LPVOID CTHREAD_ARG;
#else
    typedef pthread_mutex_t c_mutex;
    typedef pthread_t c_thread;
    typedef sem_t c_semaphore;
    typedef void* CTHREAD_ARG;
    typedef void* CTHREAD_RET;
	typedef void* CTHREAD_RET_ARG;
    typedef void* CTHREAD_RETURN;
    typedef CTHREAD_RET(*CTHREAD_FUN)(CTHREAD_ARG);
#endif

/** A simple read write lock data type.
 * This rw-lock has a writers-preference. */
typedef struct {
    c_semaphore write;
    c_semaphore readers;
    unsigned int max_readers;
} c_rwlock;

/** The function starts a new thread in the calling process.
 * The new thread starts execution by invoking function();
 * parameter is passed as the sole argument of function().
 * Returns 0 on error. */
int cthread_create( c_thread *thread_handle, CTHREAD_FUN function, CTHREAD_ARG parameter );

/** Returns the thread handle to the current thread. */
c_thread cthread_self();

/** Compares to thread handles. Returns non-zero if the threads are equal.
 * Zero otherwise. */
int cthread_equal( c_thread thread1, c_thread thread2 );

/** The function waits for the thread specified by thread
 * to terminate.  If that thread has already terminated, then the function
 * returns immediately.  The thread specified by thread must be joinable. */
int cthread_join( c_thread *thread );

/** The function waits for the thread specified by thread
 * to terminate.  If that thread has already terminated, then the function
 * returns immediately.  The thread specified by thread must be joinable.
 * The threads return value will be returned in thread_return. */
 int cthread_join_return_value( c_thread *thread, CTHREAD_RETURN *thread_return );

/** Sleep for the given milliseconds. Returns 0 on failure. */
int cthread_sleep(unsigned int milliseconds);

/** Only functional on POSIX systems. Returns 0 on error.
 * The cthread_detach() function marks the thread identified by thread
 * as detached.  When a detached thread terminates, its resources are
 * automatically released back to the system without the need for
 * another thread to join with the terminated thread.
 * Attempting to detach an already detached thread results in
 * unspecified behavior.*/
int cthread_detach(c_thread * thread_handle);

/** Initialize a mutex */
int cthread_mutex_init( c_mutex *mutex );

/** Destroy a mutex */
int cthread_mutex_destroy( c_mutex *mutex );

/** Obtain a mutex lock. */
int cthread_mutex_lock( c_mutex *mutex );

/** The cthread_mutex_trylock() function shall be equivalent to cthread_mutex_lock(),
 * except that if the mutex object referenced by mutex is currently locked (by any thread,
 * including the current thread), the call shall return immediately. */
int cthread_mutex_trylock( c_mutex *mutex );

/** Unlock a mutex */
int cthread_mutex_unlock( c_mutex *mutex );

/** Set the default stack size for each thread (only for posix threads).
 * Returns 0 on error. */
size_t cthread_attr_getstacksize();

/** Get the default stack size (only for posix threads). */
int cthread_attr_setstacksize(size_t stacksize);

/** Initialize a semaphore. */
int cthread_sem_init( c_semaphore *sem, unsigned int init_val );

/** Destroy a semaphore. */
int cthread_sem_destroy( c_semaphore *sem );

/** Acquire a semaphore (decrement the semaphore). */
int cthread_sem_wait( c_semaphore *sem );

/** Identical to cthread_sem_wait() but will return immediately and not block if the semaphore
 * cannot be decremented.*/
int cthread_sem_trywait( c_semaphore *sem );

/** Unblock (increments) a semaphore. */
int cthread_sem_post( c_semaphore *sem );

/** Initialize a rwlock with a given maximum of simultaneous readers */
int cthread_rwlock_init( c_rwlock *rwlock, unsigned int max_readers );

/** Destroy a previously initialized rwlock. The effect of subsequent use
 * on an rwlock or uninitialized rwlock is undefined. */
int cthread_rwlock_destroy( c_rwlock *rwlock );

/** cthread_rwlock_read_wait() decrements (locks) the reader semaphore in rwlock.
 * If the semaphore's value is greater than zero, then the decrement proceeds,
 * and the function returns, immediately. If the reader semaphore currently
 * has the value zero, then the call blocks until either it becomes possible to
 * perform the decrement (i.e., the semaphore value rises above zero),
 * or a signal handler interrupts the call. No reader locks can be obtained while
 * the writer semaphore is locked. */
int cthread_rwlock_read_wait( c_rwlock *rwlock );

/** cthread_rwlock_read_trywait() is the same as cthread_rwlock_read_wait(),
 * except that if the decrement cannot be immediately performed,
 * then the call returns 0. */
int cthread_rwlock_read_trywait( c_rwlock *rwlock );

/** cthread_rwlock_read_post() increments (unlocks) the reader semaphore in rwlock.
 * If the semaphore's value consequently becomes greater than zero, then another
 * process or thread blocked in a cthread_rwlock_read_wait() call will be
 * woken up and proceed to lock the semaphore.  */
int cthread_rwlock_read_post( c_rwlock *rwlock );

/** cthread_rwlock_write_wait() will try to acquire the writer semaphore.
 * If the writer semaphore currently has the value zero, then the call
 * blocks until it becomes perform the decrement. After obtaining the lock
 * the function waits for all readers on the rwlock to finish before returning. */
int cthread_rwlock_write_wait( c_rwlock *rwlock );

/** The cthread_rwlock_write_trywait() will try to acquire the writer semaphore of the rwlock
 * and return immediately if not possible at the moment. */
int cthread_rwlock_write_trywait( c_rwlock *rwlock );

/** Increments (unlocks) the writer semaphore in rwlock. */
int cthread_rwlock_write_post( c_rwlock *rwlock );

#endif /* CTHREADS_H_ */
