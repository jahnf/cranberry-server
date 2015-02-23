/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#ifndef SERVER_MODULE__H_
#define SERVER_MODULE__H_

/* server module struct - TODO use only internally */
typedef struct {
	/** command name */
	const char *cmd;
	/** Valid web session needed for command? */
	int need_valid_session;
	/** Server command function pointer */
	int (*function)(REQ_INFO *, WebArgs *, void *init_data);
	/** Server command init function */
	void * (*init_fun)(void*);
	/** Server command free function */
	int (*free_fun)(void*);
	/** pointer to init data returned by server command init function */
	void *init_data;
} ServerCommandSettings;

/** Register a server module. */
int server_module_register( init_fun, free_fun );

#endif /* SERVER_MODULE__H_ */
