int find_network_newline(char *message, int inbuf);

int buffer_message(char *message, int *inbuf, int *espace, char *after);

void cmdHandler(char *message, char *prevCmd, char **prevArgs);

char *getCmd(char message[]);

char **getArgs(char message[], char *prevCmd);

#define LG_MESSAGE 256