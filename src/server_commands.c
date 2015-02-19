/* cranberry-server. A small C web server application with lua scripting, 
 * session and sqlite support. https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#include <string.h>

#include "server_commands.h"

// function definitions
//------------------------------------------------------------------------
//int _query(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//  void * _query_init(void *data);
//  int _query_free(void *data);
//int _login(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//int _logout(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//int _tables(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//  void * _tables_init(void *data);
//  int _tables_free(void *data);
//int _jsutils(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//
////int _datatest(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//int _dstatus(REQ_INFO * uinfo, WebArgs * args, void * init_data);
//------------------------------------------------------------------------

typedef struct {
    /// command name
    const char *cmd;
    /// does the command need a valid websession?
    int need_valid_session;
    /// server command function pointer
    int (*function)(http_req_info_t *, thread_arg_t *, void *init_data);
    /// server command init function
    void * (*init_fun)(void*);
    /// server command free function
    int (*free_fun)(void*);
    /// pointer to init data returned by server command init function
    void *init_data;
} ServerCommandSettings;

/// server commands array
static ServerCommandSettings _commands [] = {
    {0,0,0,0,0,0},
//  {"_tables",     0,  &_tables,       &_tables_init,  &_tables_free,  0 },
//  {"_query",      0,  &_query,        &_query_init,   &_query_free,   0 },
//  {"_jsutils",    0,  &_jsutils,      0,              0,              0 },
    {0,0,0,0,0,0} };

/**
 * initializes all server commands.
 * @return pointer to initialized data
 */
void * server_commands_init()
{
    int i;
    for( i = 1; _commands[i].cmd != 0; ++i ) {
        if( _commands[i].init_fun != 0 )
            _commands[i].init_data = _commands[i].init_fun( NULL );
    }
    return &_commands;
}

/**
 * frees data of all server commands by calling
 * each free function when available.
 * @param init_data pointer to initialized data
 * @return integer, 1 if all free functions return successfully, 0 else
 */
int server_commands_free( void * init_data )
{
    ServerCommandSettings *commands = init_data;
    int ret = 1, i;
    if( !commands ) return 0;
    for(i = 1; commands[i].cmd != 0; ++i) {
        if( commands[i].free_fun != 0)
            ret = ret && commands[i].free_fun(commands[i].init_data);
    }
    return ret;
}

static int server_commands_count( void *init_data )
{
    ServerCommandSettings *commands = init_data;
    int i;
    for(i = 0; commands[i+1].cmd != 0; ++i);
    return i;
}

const char* server_command_get_name( void *init_data, int cmd_id )
{
    ServerCommandSettings *commands = init_data;
    static int count = 0;
    if( !count ) count = server_commands_count( init_data );
    if( cmd_id < 1 || cmd_id > count ) return 0;
    return commands[cmd_id].cmd;
}

/**
 * checks if the given character string is a server command.
 * @param init_data pointer to initialized data
 * @param cmd character string
 * @return integer id of server command or 0 if not a command
 */
int is_server_command( void *init_data, const char* cmd )
{
    ServerCommandSettings *commands = init_data;
    int i;
    for( i = 1; commands[i].cmd != 0; ++i ) {
        if( strcmp(cmd, commands[i].cmd) == 0 )
            return i;
    }
    return 0;
}

int server_command_needs_valid_session( void *init_data, int cmd_id )
{
    ServerCommandSettings *commands = init_data;
    return commands[cmd_id].need_valid_session;
}

int server_command_run_by_name( void *init_data, const char *cmd, http_req_info_t *uinfo, thread_arg_t *args )
{
    unsigned int i;
    ServerCommandSettings *commands = init_data;
    for( i = 1; commands[i].cmd != 0; ++i ) {
        if( strcmp(cmd, commands[i].cmd) == 0 ) {
            if( commands[i].function ) {
                return commands[i].function( uinfo, args, commands[i].init_data );
            }
            return SERVER_COMMAND_NO_FUNC_PTR;
        }
    }
    return SERVER_COMMAND_NOT_FOUND;
}

int server_command_run_by_id( void *init_data, int cmd_id, http_req_info_t *uinfo, thread_arg_t *args )
{
    ServerCommandSettings *commands = init_data;
    if( commands[cmd_id].function ) {
        return commands[cmd_id].function( uinfo, args, commands[cmd_id].init_data );
    }
    return SERVER_COMMAND_NO_FUNC_PTR;
}
