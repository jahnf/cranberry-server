/** @file server_commands.h
 *	@author Jahn Fuchs
 *
 *  $LastChangedDate: 2011-01-19 10:20:36 +0100 (Wed, 19 Jan 2011) $
 *  $LastChangedBy: Fuchs $
 */

#ifndef SERVER_COMMANDS_H_
#define SERVER_COMMANDS_H_

#include "http_request.h"
#include "webthread.h"

enum {
	SERVER_COMMAND_OKAY = 0,
	SERVER_COMMAND_ERROR = -1,
	SERVER_COMMAND_NOT_FOUND = -2,
	SERVER_COMMAND_NO_FUNC_PTR = -3
};


void * server_commands_init();
int server_commands_free(void * init_data);

int is_server_command(void * init_data, const char* cmd);
int server_command_run_by_name(void * init_data, const char* cmd, http_req_info_t * uinfo, thread_arg_t * args);
int server_command_run_by_id(void * init_data, int cmd_id, http_req_info_t * uinfo, thread_arg_t * args);
int server_command_needs_valid_session(void * init_data, int cmd_id);
const char* server_command_get_name( void *init_data, int cmd_id );

#endif /* SERVER_FUNCTIONS_H_ */
