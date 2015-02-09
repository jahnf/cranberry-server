/** @file cmdline.h
 *	@author Jahn Fuchs
 *
 */

#ifndef CMDLINE_H_
#define CMDLINE_H_

#include "webthread.h"

enum {
	CMDLINE_OKAY = 0,
	CMDLINE_HELP_REQUESTED,
	CMDLINE_FORMAT_ERROR
};

/** Prints command line help to stdout, if detail !=0 it prints a detailed version. */
void cmdline_print_help( const char * name, const int detail );

/** Parses the command line parameters, fills settings in WebArgs and sets
 * configuration file if set as parameter, returns CMDLINE_OKAY on success. */
int cmdline_parse( thread_arg_t *args, const int argc, char **argv, char **config_file );

#endif /* CMDLINE_H_ */
