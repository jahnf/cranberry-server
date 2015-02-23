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

#define RWTEST1_READER 5
#define RWTEST1_WRITER 3

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
    ck_assert(cthread_rwlock_init( &rwlock, RWTEST1_READER ));

    ck_assert(cthread_rwlock_write_trywait( &rwlock ) != 0); 
    ck_assert(cthread_rwlock_write_post( &rwlock ) != 0); 

    for( i = 0; i < RWTEST1_READER; ++i ) {
        ck_assert(cthread_rwlock_read_trywait( &rwlock ) != 0); 
        ck_assert(cthread_rwlock_write_trywait( &rwlock ) == 0); 
    }
    ck_assert(cthread_rwlock_read_trywait( &rwlock ) == 0); 

    for( i = 0; i < RWTEST1_READER; ++i ) {
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
    if(trylock_result) {
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

/* This test only tests the return values of the cthread_rwlock_* 
 * functions and the user data increment in the writer threads */
START_TEST (cthread_rwlock1)
{
    int i, shared_data=0;
    c_thread readerthreads[RWTEST1_READER];
    c_thread writerthreads[RWTEST1_WRITER];
    c_rwlock rwlock;
    /* Initialize rwlock with less max readers than reader threads in the test */
    ck_assert(cthread_rwlock_init( &rwlock, RWTEST1_READER-1 ));
    
    for( i = 0; i < RWTEST1_WRITER; ++i ) {
        /* it's the thread's job to free data */
        rwlock_thread_data *data = malloc( sizeof(rwlock_thread_data) );
        ck_assert(data!=NULL);
        data->num = i;
        data->shared_data = &shared_data;
        data->rwlock = &rwlock;
        if( !cthread_create(&writerthreads[i], rwlock_writer, data) ) {
            ck_abort_msg("Writer thread creation error.");
            printf("write thread creation error(%d)\n", i);
            free( data ); 
        }
    }  
    
    for( i = 0; i < RWTEST1_READER; ++i ) {
        rwlock_thread_data *data = malloc( sizeof(rwlock_thread_data) );
        ck_assert(data!=NULL);
        data->num = i;
        data->shared_data = &shared_data;
        data->rwlock = &rwlock;
        if( !cthread_create(&readerthreads[i], rwlock_reader, data) ) {
            ck_abort_msg("Reader thread creation error.");
            printf("reader thread creation error(%d)\n", i);
            free( data ); 
        }
    }
    
    /* Wait for all threads to finish */
    for( i = 0; i < RWTEST1_READER; ++i ) {
        ck_assert(cthread_join( &readerthreads[i] ));
    }
    for( i = 0; i < RWTEST1_WRITER; ++i ) {
        ck_assert(cthread_join( &writerthreads[i] ));
    }

    ck_assert(cthread_rwlock_destroy( &rwlock ));
}
END_TEST

/*  function that returns the test suite */
Suite *cthread_test_suite( void )
{
    Suite *s = suite_create ("CThread");

    /* Core test cases */
    TCase *tc_core = tcase_create ("Core");
    
    tcase_add_test (tc_core, cthread_mutex_basic);
    tcase_add_test (tc_core, cthread_rwlock_basic);
    tcase_add_test (tc_core, cthread_semaphore_basic);
    tcase_add_test (tc_core, cthread_rwlock1);
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
