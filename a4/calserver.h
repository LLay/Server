#ifndef CALSERVER_H
#define CALSERVER_H

struct client {
	int fd;
	char *user_name;
	struct in_addr ipaddr;
	struct client *next;
};

int process_arg(int fd, int cmd_argc, char **cmd_argv);
struct client *find_client(int fd);
void bind_and_listen(void);
struct client *add_client(int fd, struct in_addr addr);
static void remove_client(int fd);
void new_connection(void);
time_t convert_time(int local_argc, char** local_argv);
char **split_str(char *str); //REMOVE?