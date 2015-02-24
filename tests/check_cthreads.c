#include "cthreads.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WINDOWS
    #include <unistd.h>
#endif

/* include header for 'check' unit testing */
#include <check.h>

#define THREAD_STACK_SIZE 512000

#define SEMAPHORE_INIT_VALUE 10
#define INITIAL_PROTECTED_DATA_VALUE 268435455
#define RWTEST_MAX_READER 9

typedef struct {
    int id;
    int *protected_data;
    c_mutex *mutex;
} test_mutex_thread_data;

typedef struct {
    int num;
    int *shared_data;
    c_rwlock *rwlock;
} rwlock_thread_data;

CTHREAD_RET rwlock_reader(CTHREAD_ARG data)
{
    rwlock_thread_data *mydata = (rwlock_thread_data*) data;
    printf("rwlock_reader thread #%d wait\n", mydata->num );
    cthread_rwlock_read_wait( mydata->rwlock );
    printf("rwlock_reader thread #%d lock & sleep\n", mydata->num );
    cthread_sleep( 400 );
    printf("rwlock_reader thread #%d post\n", mydata->num );
    cthread_rwlock_read_post( mydata->rwlock );
    free( data );
    return (CTHREAD_RET) 1;
}

CTHREAD_RET rwlock_writer(CTHREAD_ARG data)
{
    rwlock_thread_data *mydata = (rwlock_thread_data*) data;
    printf("rwlock_writer thread #%d wait\n", mydata->num );
    cthread_rwlock_write_wait( mydata->rwlock );
    printf("rwlock_writer thread #%d got lock & sleep\n", mydata->num );
    ck_assert(mydata->num == *mydata->shared_data);
    *mydata->shared_data = mydata->num+1;
    cthread_sleep( 700 );
    printf("rwlock_writer thread #%d post\n", mydata->num );
    cthread_rwlock_write_post( mydata->rwlock );
    free( data );
    return (CTHREAD_RET) 1;
}

START_TEST (cthread_rwlock_basic)
{
    c_rwlock rwlock; int i = 0;
    ck_assert(cthread_rwlock_init( &rwlock, RWTEST_MAX_READER ));

    ck_assert(cthread_rwlock_write_trywait( &rwlock ) != 0); 
    ck_assert(cthread_rwlock_write_post( &rwlock ) != 0); 

    for( i = 0; i < RWTEST_MAX_READER; ++i ) {
        ck_assert(cthread_rwlock_read_trywait( &rwlock ) != 0); 
        ck_assert(cthread_rwlock_write_trywait( &rwlock ) == 0); 
    }
    ck_assert(cthread_rwlock_read_trywait( &rwlock ) == 0); 

    for( i = 0; i < RWTEST_MAX_READER; ++i ) {
        ck_assert(cthread_rwlock_write_trywait( &rwlock ) == 0); 
        ck_assert(cthread_rwlock_read_post( &rwlock ) != 0); 
    }

    ck_assert(cthread_rwlock_write_trywait( &rwlock ) != 0); 
    ck_assert(cthread_rwlock_read_trywait( &rwlock ) == 0); 
    ck_assert(cthread_rwlock_write_trywait( &rwlock ) == 0); 
    ck_assert(cthread_rwlock_write_post( &rwlock ) != 0); 
    
    ck_assert(cthread_rwlock_destroy( &rwlock ));
}
END_TEST

START_TEST (cthread_mutex_basic)
{
    c_mutex mutex; int trylock_result = 0;
    ck_assert(cthread_mutex_init( &mutex ));
    trylock_result = cthread_mutex_trylock( &mutex );
    ck_assert(trylock_result != 0);
    if( trylock_result != 0 ) {
        ck_assert(cthread_mutex_trylock( &mutex ) == 0);
        ck_assert(cthread_mutex_unlock( &mutex ));
    }
    ck_assert(cthread_mutex_destroy( &mutex ));
}
END_TEST

START_TEST (cthread_semaphore_basic)
{
    c_semaphore semaphore; int i = 0;
    ck_assert(cthread_sem_init( &semaphore, SEMAPHORE_INIT_VALUE ) != 0);
    for( i = 0; i < SEMAPHORE_INIT_VALUE; ++i ) {
        ck_assert(cthread_sem_trywait( &semaphore ) != 0); 
    }
    ck_assert(cthread_sem_trywait( &semaphore ) == 0); 
    for( i = 0; i < SEMAPHORE_INIT_VALUE; ++i ) {
        ck_assert(cthread_sem_post( &semaphore ) != 0); 
    }
    ck_assert(cthread_sem_destroy( &semaphore ));
}
END_TEST

CTHREAD_RET mutex_threads_function1(CTHREAD_ARG data)
{
    test_mutex_thread_data *mydata = (test_mutex_thread_data*) data;
    int read_value;
    
    ck_assert(cthread_mutex_trylock( mydata->mutex ) == 0);
    ck_assert(cthread_mutex_lock( mydata->mutex ) == 0);

    read_value = *mydata->protected_data;
    *mydata->protected_data = mydata->id;
    ck_assert(read_value==INITIAL_PROTECTED_DATA_VALUE);
    
    ck_assert(cthread_mutex_unlock( mydata->mutex ) == 0);
    return (CTHREAD_RET) (long) read_value;
}

START_TEST (cthread_mutex_threads)
{
    test_mutex_thread_data td1;
    c_thread thread1; c_mutex mutex; 
    CTHREAD_RET thread1_return = 0;
    int trylock_result = 0;
    int protected_data = INITIAL_PROTECTED_DATA_VALUE;
    
    td1.id = 1; 
    td1.protected_data = &protected_data;
    td1.mutex = &mutex;
    
    ck_assert(cthread_mutex_init( &mutex ));
    trylock_result = cthread_mutex_trylock( &mutex );
    ck_assert(trylock_result != 0);
    if( trylock_result != 0 ) {
        /* Create thread */
        ck_assert(cthread_create(&thread1, mutex_threads_function1, &td1));
        /* check values and unlock mutex */
        ck_assert(protected_data==INITIAL_PROTECTED_DATA_VALUE);
        ck_assert(cthread_mutex_unlock( &mutex ));
        /* Join thread - thread should get mutex lock, read, write and unlock mutex again */
        ck_assert(cthread_join_return_value( &thread1, &thread1_return ));
        /* Check threads return value*/
        ck_assert((int)(long)thread1_return == INITIAL_PROTECTED_DATA_VALUE);
        /* Check values */
        ck_assert(cthread_mutex_trylock( &mutex ));
        ck_assert(protected_data==td1.id);
        ck_assert(cthread_mutex_unlock( &mutex ));
    }
    
    ck_assert(cthread_mutex_destroy( &mutex ));
}
END_TEST

/*  function that returns the test suite */
Suite *cthread_test_suite( void )
{
    Suite *s = suite_create ("CThread");

    /* Core test cases */
    TCase *tc_core = tcase_create ("Core");
    
    tcase_add_test (tc_core, cthread_mutex_basic);
    tcase_add_test (tc_core, cthread_mutex_threads);
    tcase_add_test (tc_core, cthread_rwlock_basic);
    tcase_add_test (tc_core, cthread_semaphore_basic);
    suite_add_tcase (s, tc_core);

    return s;
}

/* Test runner main */
int main (void)
{
    int number_failed;
    Suite *s = cthread_test_suite();
    SRunner *sr = srunner_create( s );

    /* for cygwin.. (cygwin check version does not support fork) */
    srunner_set_fork_status( sr, CK_NOFORK );

    srunner_run_all( sr, CK_NORMAL );
    number_failed = srunner_ntests_failed( sr );
    srunner_free( sr );
    return( number_failed == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;
}