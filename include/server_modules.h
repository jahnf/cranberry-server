/**
 */

#ifndef SERVER_MODULE__H_
#define SERVER_MODULE__H_

// server module struct - TODO use only internally
typedef struct {
	/// command name
	const char *cmd;
	/// does the command need a valid websession?
	int need_valid_session;
	/// server command function pointer
	int (*function)(REQ_INFO *, WebArgs *, void *init_data);
	/// server command init function
	void * (*init_fun)(void*);
	/// server command free function
	int (*free_fun)(void*);
	/// pointer to init data returned by server command init function
	void *init_data;
} ServerCommandSettings;



int server_module_register( init_fun, free_fun );



#endif /* SERVER_MODULE__H_ */