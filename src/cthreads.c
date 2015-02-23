/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
/** @file cthreads.c
 *  @author Jahn Fuchs
 *
 *  cthreads wraps simple threads, mutexes, semaphores and a readers-writer_lock
 *  that can be used without changes on both posix and ms windows systems.
 */
 
#include <time.h>
#include "cthreads.h"

#ifdef _WIN32
    static size_t cthread_stacksize = 0;
#else
    static int bAttrInit = 0;
    static pthread_attr_t _pt_attr;
    #include <sys/time.h>
#endif

int cthread_sleep(unsigned int milliseconds)
{
    if( milliseconds ) {
    #ifdef _WIN32
        Sleep( milliseconds );
        return 1;
    #else
        struct timespec rqtp, rmtp;
        rqtp.tv_sec = (milliseconds / 1000);
        rqtp.tv_nsec = ((long)(milliseconds % 1000))*1000000L;

        if(nanosleep(&rqtp , &rmtp) < 0 )   
        {
            /* rmtp should contain the remaining time if nanosleep fails */
            /* Fallback alternative for sleep with pthread timedwait */
            struct timespec timeToWait;
            struct timeval now;
            pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_cond_t fakeCond = PTHREAD_COND_INITIALIZER;

            gettimeofday(&now,NULL);

            timeToWait.tv_nsec = now.tv_usec*1000 + (rmtp.tv_nsec);
            timeToWait.tv_sec = now.tv_sec + rmtp.tv_sec + (timeToWait.tv_nsec / 1000000000);
            timeToWait.tv_nsec %= 1000000000;

            pthread_mutex_lock( &fakeMutex );
            pthread_cond_timedwait( &fakeCond, &fakeMutex, &timeToWait );
            pthread_mutex_unlock( &fakeMutex );
        }
        return 1;
    #endif
    }
    return 1;
}

int cthread_equal(c_thread this, c_thread other)
{
    #ifdef _WIN32
        return (this == other);
    #else
        return pthread_equal(this, other);
    #endif
}

c_thread cthread_self()
{
    #ifdef _WIN32
        return GetCurrentThread();
    #else
        return pthread_self();
    #endif
}

int cthread_attr_setstacksize( size_t stacksize )
{
    #ifdef _WIN32
        cthread_stacksize = stacksize;
        return 1;
    #else
        if( !bAttrInit ) {
            pthread_attr_init(&_pt_attr);
            bAttrInit = 1;
        }
        return !pthread_attr_setstacksize (&_pt_attr, stacksize);
    #endif
}

/* Get default stacksize */
size_t cthread_attr_getstacksize()
{
    #ifdef _WIN32
        return cthread_stacksize;
    #else
        size_t stacksize = 0;
        if( !bAttrInit ) {
            pthread_attr_init(&_pt_attr);
            bAttrInit = 1;
        }
        pthread_attr_getstacksize (&_pt_attr, &stacksize);
        return stacksize;
    #endif
}

/* returns 0 on error */
int cthread_detach(c_thread * thread_handle)
{
    #ifdef _WIN32
        return 1;
    #else
        return pthread_detach(*thread_handle) == 0;
    #endif
}


/* returns 0 on error */
int cthread_create( c_thread *thread_handle, CTHREAD_FUN function, CTHREAD_ARG parameter )
{
    #ifdef _WIN32
        *thread_handle = CreateThread(  NULL,              /* default security attributes */
                                        cthread_stacksize, /* use default stack size      */ 
                                        function,          /* thread function name        */
                                        parameter,         /* argument to thread function */
                                        0,                 /* use default creation flags  */
                                        0);
        return ( (*thread_handle) != 0 );
    #else
        if( !bAttrInit ) {
            pthread_attr_init(&_pt_attr);
            bAttrInit = 1;
        }
        return pthread_create(thread_handle, &_pt_attr, function, parameter) == 0;
    #endif
}

/* returns 0 on error */
int cthread_join( c_thread *thread )
{
    #ifdef _WIN32
        return( WAIT_FAILED != WaitForSingleObject( *thread,     /* handle to mutex */
                                                    INFINITE) ); /* wait time */
    #else
        return  pthread_join(*thread, NULL) == 0;
    #endif
}

/* returns 0 on error */
int cthread_mutex_init( c_mutex *mutex )
{
    #ifdef _WIN32
        InitializeCriticalSection(mutex);
        return 1;
    #else
        return  pthread_mutex_init(mutex, NULL) == 0;
    #endif
}

int cthread_mutex_destroy( c_mutex *mutex )
{
    #ifdef _WIN32
        DeleteCriticalSection( mutex );
        return 1;
    #else
        /* pthread_mutex_unlock( mutex ); */
        return  pthread_mutex_destroy( mutex ) == 0;
    #endif
}

/* returns 0 on fail */
int cthread_mutex_trylock( c_mutex *mutex )
{
    #ifdef _WIN32
        return TryEnterCriticalSection( mutex );
    #else
        return pthread_mutex_trylock( mutex ) == 0;
    #endif
}

/* returns 0 on error */
int cthread_mutex_lock( c_mutex *mutex )
{
    #ifdef _WIN32
        EnterCriticalSection( mutex );
        return 1;
    #else
        return pthread_mutex_lock( mutex ) == 0;
    #endif
}

/* returns 0 on error */
int cthread_mutex_unlock( c_mutex *mutex )
{
    #ifdef _WIN32
        LeaveCriticalSection( mutex );
        return 1;
    #else
        return pthread_mutex_unlock( mutex ) == 0;
    #endif
}

int cthread_sem_init( c_semaphore *sem, unsigned int init_val )
{
    #ifdef _WIN32
        *sem = CreateSemaphore( NULL,       /* default security attributes */
                                init_val,   /* initial count */
                                init_val,   /* maximum count */
                                NULL);      /* unnamed semaphore */
        return ( (*sem) != 0 );
    #else
        return sem_init( sem, 0, init_val ) != -1;
    #endif
}

int cthread_sem_destroy( c_semaphore *sem )
{
    #ifdef _WIN32
        return CloseHandle( *sem );
    #else
        return sem_destroy( sem ) == 0;
    #endif
}

int cthread_sem_wait( c_semaphore *sem ) 
{
    #ifdef _WIN32
        return( WAIT_FAILED != WaitForSingleObject( *sem,        /* handle to mutex */
                                                    INFINITE) ); /* wait time */
    #else
        return sem_wait( sem ) == 0;
    #endif
}

int cthread_sem_trywait( c_semaphore *sem ) 
{
    #ifdef _WIN32
        return( WAIT_OBJECT_0 == WaitForSingleObject( *sem,  /* handle to mutex */
                                                      0));   /* wait time */
    #else
        return sem_trywait( sem ) == 0;
    #endif
}

int cthread_sem_post( c_semaphore *sem ) 
{
    #ifdef _WIN32
        return ReleaseSemaphore( *sem,  /* handle to semaphore */
                                    1,  /* increase count by one */
                                 NULL); /* not interested in previous count */
    #else
        return sem_post( sem ) == 0;
    #endif
}

int cthread_rwlock_init( c_rwlock *rwlock, unsigned int max_readers ) 
{
    rwlock->max_readers = max_readers;
    if( !cthread_sem_init( &rwlock->write, 1 ) ) return 0;
    if( !cthread_sem_init( &rwlock->readers, max_readers ) ) return 0;
    return 1;
}

int cthread_rwlock_destroy( c_rwlock *rwlock ) 
{
    if( !cthread_sem_destroy( &rwlock->write ) ) return 0;
    if( !cthread_sem_destroy( &rwlock->readers ) )  return 0;
    return 1;
}

int cthread_rwlock_read_wait( c_rwlock *rwlock ) 
{
    if( !cthread_sem_wait( &rwlock->write ) ) return 0;
    if( !cthread_sem_wait( &rwlock->readers ) ) return 0;
    if( !cthread_sem_post( &rwlock->write ) ) return 0;
    return 1;
}

int cthread_rwlock_read_trywait( c_rwlock *rwlock ) 
{
    if( !cthread_sem_trywait( &rwlock->write ) ) return 0;
    if( !cthread_sem_trywait( &rwlock->readers ) ) {
        cthread_sem_post( &rwlock->write );
        return 0;       
    } 
    cthread_sem_post( &rwlock->write );
    return 1;
}

int cthread_rwlock_read_post( c_rwlock *rwlock ) 
{
    return cthread_sem_post( &rwlock->readers );
}

int cthread_rwlock_write_wait( c_rwlock *rwlock ) 
{
    unsigned int i; 
    if( !cthread_sem_wait( &rwlock->write ) ) return 0;
    /* block all readers */
    for( i = 0; i < rwlock->max_readers; ++i )      
        cthread_sem_wait( &rwlock->readers );
    return 1;
}

int cthread_rwlock_write_trywait( c_rwlock *rwlock ) 
{
    unsigned int i; 
    if( !cthread_sem_trywait( &rwlock->write ) ) return 0;
    for( i = 0; i < rwlock->max_readers; ++i ) {      
        if( !cthread_sem_trywait( &rwlock->readers ) ) break;
    }
    if( i != rwlock->max_readers ) {
        cthread_sem_post( &rwlock->write );
        for ( ; i != 0; --i) {
            cthread_sem_post( &rwlock->readers );
        }
        return 0;
    }
    return 1;
}

int cthread_rwlock_write_post( c_rwlock *rwlock ) 
{
    unsigned int i; 
    if( !cthread_sem_post( &rwlock->write ) ) return 0;
    for( i = 0; i < rwlock->max_readers; ++i )      
        cthread_sem_post( &rwlock->readers );
    return 1;
}
