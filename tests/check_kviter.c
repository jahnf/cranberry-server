/*
 * test_kviter.c
 *  key value iterator TEST
 *  Created on: 25.07.2012
 */

/* include header for 'check' unit testing */
#include <check.h>
#include <stdlib.h>

#include "kv_iter.h"  

/* define TEST STRINGS and VALUES*/
#define KEY01 "foo"
#define VAL01 "bar"
#define KEY02 "ding"
#define VAL02 "bar"

#define SEP_VAL_S "="
#define SEP_KEY_S "&"
#define SEP_VAL_C '='
#define SEP_KEY_C '&'

/* Iterator reset test */
START_TEST (kv_iter_reset)
{
    const char *qs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    kviter_t kvi;
    /* overwrite kvi with 01010101 bit pattern in memory */
    memset( &kvi, 85, sizeof(kvi) );
    /* call kviter_reset function */
    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, qs, strlen(qs) );
    fail_unless( kvi.key_sep == SEP_KEY_C );
    fail_unless( kvi.val_sep == SEP_VAL_C );
    fail_unless( kvi.s == qs );
    fail_unless( kvi.len == strlen(qs) );
    fail_unless( kvi.pos == 0 );
    fail_unless( kvi.keylen == 0 );
    fail_unless( kvi.vallen == 0 );
    fail_unless( kvi.key == NULL );
    fail_unless( kvi.val == NULL );
}
END_TEST

/* Simple test with correct input string */
START_TEST (kv_iter_simple)
{
    kviter_t kvi;
    size_t count = 0;
    /* qs := "foo=bar&ding=bar" */
    const char* qs = KEY01 SEP_VAL_S VAL01 SEP_KEY_S KEY02 SEP_VAL_S VAL02;
    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, qs, strlen(qs) );
    while( kviter_next(&kvi) ) {
        ++count;
       char* key = (char*) malloc(kvi.keylen + 1);
       strncpy(key, kvi.key, kvi.keylen);
       key[kvi.keylen] = 0;
       char* val = (char*) malloc(kvi.vallen + 1);
       strncpy(val, kvi.val, kvi.vallen);
       val[kvi.vallen] = 0;

       fail_if( count == 1 && strcmp(key,KEY01) != 0 );
       fail_if( count == 1 && strcmp(val,VAL01) != 0 );
       fail_if( count == 2 && strcmp(key,KEY02) != 0 );
       fail_if( count == 2 && strcmp(val,VAL02) != 0 );

       free(key);
       free(val);
    }

    fail_if( count != 2 );
}
END_TEST

/* simple test with correct input string (with leading white spaces before key) 
 * using kviter_next_i */
START_TEST (kv_iter_leading_char)
{
  /* unit test code */
    kviter_t kvi;
    size_t count = 0;
    /* qs := "   foo=bar&     ding=bar" */
    const char* qs = "   " KEY01 SEP_VAL_S VAL01 SEP_KEY_S "     " KEY02 SEP_VAL_S VAL02;
    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, qs, strlen(qs) );
    while( kviter_next_i(&kvi, 1, ' ') ) {
        ++count;
       char* key = (char*) malloc(kvi.keylen + 1);
       strncpy(key, kvi.key, kvi.keylen);
       key[kvi.keylen] = 0;
       char* val = (char*) malloc(kvi.vallen + 1);
       strncpy(val, kvi.val, kvi.vallen);
       val[kvi.vallen] = 0;

       fail_if( count == 1 && strcmp(key,KEY01) != 0 );
       fail_if( count == 1 && strcmp(val,VAL01) != 0 );
       fail_if( count == 2 && strcmp(key,KEY02) != 0 );
       fail_if( count == 2 && strcmp(val,VAL02) != 0 );

       free(key);
       free(val);
    }

    fail_if( count != 2 );
}
END_TEST

/* test kviter with an empty string */
START_TEST (kv_iter_str_empty1)
{
    kviter_t kvi;
    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, "", 0 );
    fail_if( kviter_next(&kvi) );

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, NULL, 0 );
    fail_if( kviter_next(&kvi) );

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, "TEST", 0 );
    fail_if( kviter_next(&kvi) );
}
END_TEST

/* test kviter with an empty string 2 */
START_TEST (kv_iter_str_empty2)
{
    kviter_t kvi;

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, "", 0 );
    fail_if( kviter_next_i(&kvi, 1, '\0') );

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, NULL, 0 );
    fail_if( kviter_next_i(&kvi, 1, '\0') );

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, "WHAT", 0 );
    fail_if( kviter_next_i(&kvi, 1, '\0') );
}
END_TEST

/* test kviter with a string that contains only one key*/
START_TEST (kv_iter_str_key_only)
{
    char keybuf[128];
    size_t count = 0;
    kviter_t kvi;

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, KEY01, sizeof(KEY01)-1 );
    while( kviter_next(&kvi) ) {
        ++count;
        fail_unless( kvi.val == NULL );
        fail_unless( kvi.vallen == 0 );
        fail_unless( kvi.keylen == sizeof(KEY01)-1 );
        strncpy(keybuf, kvi.key, kvi.keylen);
        keybuf[kvi.keylen] = 0;
        fail_unless( strcmp(KEY01, keybuf) == 0 );
    } 
    fail_unless( count == 1 );
}
END_TEST

/* test kviter with a string that contains only one key*/
START_TEST (kv_iter_str_key_only2)
{
    size_t count = 0;
    kviter_t kvi;

    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, KEY01, sizeof(KEY01)-2 );
    while( kviter_next(&kvi) ) {
        ++count;
        fail_unless( kvi.val == NULL );
        fail_unless( kvi.vallen == 0 );
        fail_unless( kvi.keylen == sizeof(KEY01)-2 );
    } 
    fail_unless( count == 1 );
}
END_TEST

/* test kviter with a string that contains multiple keys */
START_TEST (kv_iter_str_key_only3)
{
    char keybuf[128];
    size_t count = 0;
    kviter_t kvi;
    const char* qs = "   " KEY01 SEP_KEY_S "     " KEY02;
    kviter_reset( &kvi, SEP_KEY_C, SEP_VAL_C, qs, strlen(qs) );
    while( kviter_next_i(&kvi,1,' ') ) {
        ++count;
        fail_unless( kvi.val == NULL );
        fail_unless( kvi.vallen == 0 );
        fail_if( count == 1 && kvi.keylen != sizeof(KEY01)-1 );
        fail_if( count == 2 && kvi.keylen != sizeof(KEY02)-1 );
        strncpy(keybuf, kvi.key, kvi.keylen);
        keybuf[kvi.keylen] = 0;
        fail_if( count == 1 && strcmp(KEY01, keybuf) != 0 );
        fail_if( count == 2 && strcmp(KEY02, keybuf) != 0 );
    } 
    fail_unless( count == 2 );
}
END_TEST


/*  function that returns the key value iterator test suite */
Suite *kv_iter_suite( void )
{
    Suite *s = suite_create ("KeyValue Iterator");

    /* Core test cases */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, kv_iter_simple);
    tcase_add_test (tc_core, kv_iter_reset);
    tcase_add_test (tc_core, kv_iter_leading_char);
    tcase_add_test (tc_core, kv_iter_str_empty1);
    tcase_add_test (tc_core, kv_iter_str_empty2);
    tcase_add_test (tc_core, kv_iter_str_key_only);
    tcase_add_test (tc_core, kv_iter_str_key_only2);
    tcase_add_test (tc_core, kv_iter_str_key_only3);
    suite_add_tcase (s, tc_core);

    return s;
}

/*  MAIN */
int main (void)
{
    int number_failed;
    Suite *s = kv_iter_suite();
    SRunner *sr = srunner_create( s );

    /* for cygwin.. (cygwin check version does not support fork) */
    srunner_set_fork_status( sr, CK_NOFORK );

    srunner_run_all( sr, CK_NORMAL );
    number_failed = srunner_ntests_failed( sr );
    srunner_free( sr );
    return( number_failed == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
