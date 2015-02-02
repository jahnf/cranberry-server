/*
 * check_ini.c
 *  strawberry-ini reader/parser/dictionary tests
 */

/* include header for 'check' unit testing */
#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "ini_reader.h"
#include "ini_dictionary.h"
#include "ini_parser.h"

typedef struct {
    unsigned sec_count;
    unsigned key_count;
} ud_t;

int basic_callback_handler( void* user, const char* section,
              const char* name, const char* value) {
    if( name == NULL )
        ++((ud_t*)(user))->sec_count;
    else
        ++((ud_t*)(user))->key_count;
    return INI_OKAY;
}

#define TEST1_SECTION1 "section1"
#define TEST1_SECTION2 "_section2"
#define TEST1_SECTION3 "WWW_sec3"
#define TEST1_SECTION4 "AAA-seC4"

static int test1_handler( void* user, const char* section,
              const char* name, const char* value) {
    if( name == NULL ) {
        switch( ++((ud_t*)(user))->sec_count ) {
        case 1:
            fail_unless( strcasecmp(section, TEST1_SECTION1) == 0 );
            break;
        case 2:
            fail_unless( strcasecmp(section, TEST1_SECTION2) == 0 );
            break;
        case 3:
            fail_unless( strcasecmp(section, TEST1_SECTION3) == 0 );
            break;
        case 4:
            fail_unless( strcasecmp(section, TEST1_SECTION4) == 0 );
            break;
        default:
            break;
        }
    } else
        ++((ud_t*)(user))->key_count;
    return INI_OKAY;
}

/* test for correct section parsing */
START_TEST (ini_parser_sections)
{
    fail_if( sizeof(TEST1_SECTION1)-1 > INI_MAX_SECTION 
             || sizeof(TEST1_SECTION2)-1 > INI_MAX_SECTION
             || sizeof(TEST1_SECTION3)-1 > INI_MAX_SECTION
             || sizeof(TEST1_SECTION4)-1 > INI_MAX_SECTION, 
             "INI_MAX_SECTION is too small to run test." );   
             
    fail_if( INI_SECTION_CALLBACK == 0, "INI_SECTION_CALLBACKS needs to be != 0 to perform test");
    
    const char config[] =
            "; comment4\n"
            "[" TEST1_SECTION1 "]\n"
            "\r\t\n"
            "[" TEST1_SECTION2 "]\n"
            "key:val\n"
            "[" TEST1_SECTION3 "]\n"
            "[" TEST1_SECTION4 "]"
    ;

    int fds[2];
    ud_t ud = {0,0};
    fail_unless( pipe(fds) == 0, "could not create pipe fds" );
    FILE *fp = fdopen( fds[0], "r" );
    fail_if( fp == NULL, "could not open fstream on fds[0]" );
    fail_unless( write(fds[1], config, sizeof(config)-1 ) == sizeof(config)-1,
                 "could not write to fds[1]");
    close( fds[1] );
    unsigned line_err;
    ini_parse_file( fp, test1_handler, &ud, &line_err );

    fail_unless( ud.sec_count == 4 );
    fail_unless( ud.key_count == 1 );

    close( fds[0] );
}
END_TEST

/* test parsing of a file with only blank and comment lines */
START_TEST (ini_parser_comments_blanklines)
{
    const char config[] =
            "\r\t\n"
            "\t\n"
            " ; comment1\n"
            "#; comment2 ;\\\n"
            " # comment3\n"
            "; comment4\n"
            "   \t \n"
    ;
    int fds[2];

    fail_unless( pipe(fds) == 0, "could not create fds" );
    FILE *fp = fdopen( fds[0], "r" );
    fail_if( fp == NULL, "could not open fstream on fds[0]" );
    fail_unless( write(fds[1], config, sizeof(config)-1 ) == sizeof(config)-1,
                 "could not write to fds[1]");
    close( fds[1] );
    unsigned line_err;
    ud_t ud = {0,0};
    ini_parse_file( fp, basic_callback_handler, &ud, &line_err );
    fail_unless( ud.key_count == 0 );
    fail_unless( ud.sec_count == 0 );
    close( fds[0] );
}
END_TEST


/*  function that returns a test suite */
Suite *ini_parser_suite( void )
{
    Suite *s = suite_create ("strawberry-ini Parser");

    /* Core test cases */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, ini_parser_comments_blanklines);
    tcase_add_test (tc_core, ini_parser_sections);
    suite_add_tcase (s, tc_core);

    return s;
}

/*  MAIN */
int main (void)
{
    int number_failed = 0;
    Suite *s = ini_parser_suite();
    SRunner *sr = srunner_create( s );

    /* for cygwin.. (cygwin check version does not support fork) */
    srunner_set_fork_status( sr, CK_NOFORK );

    srunner_run_all( sr, CK_NORMAL );
    number_failed = srunner_ntests_failed( sr );
    srunner_free( sr );
    return( number_failed == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
