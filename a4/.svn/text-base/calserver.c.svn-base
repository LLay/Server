#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "lists.h"

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 8
#define DELIM " \0"

// If a port was not set by the makefile, set it here
# ifndef PORT
#   define PORT 48654
# endif

struct client { // Put in header file. header file not working rn
  int fd;
  struct in_addr ipaddr;
  struct client *next;
  char *user_name;
  // Attributes to be used to keep track of input from the client
  char *buf; // The buffer in which we acumulate input from the client
  int inbuf; // Indicates where in the buf to write (to complete incomplete commands)
};

// For use in bind and accept
socklen_t socklen = sizeof (struct sockaddr_in);

// Declare empty data structures.
Calendar *calendar_list = NULL;
User *user_list = NULL;
struct client *client_list = NULL;

// messages to be sent to clients by new_connection
static char greeting[] =
    "What is your user name?\r\n";
static char command_prompt[] =
  "Go ahead and enter calendar commands>\r\n";

// The listening socket
static int listenfd;

void *safe_malloc(int nbytes);
int find_network_newline(char *buf, int inbuf);
int find_by_name(char *name);
void bind_and_listen(void);
void add_client(int fd, struct in_addr addr);
void remove_client(int fd);
time_t convert_time(int local_argc, char** local_argv);
int process_arg(int fd, int cmd_argc, char **cmd_argv);
int new_connection();
void whatsup(struct client *p);


int main(int argc, char **argv) {
  struct client *p;
  /* List to keep track of sockets*/
  fd_set fdlist;
  /* Variable for select. Stores number of ready clients each iteration. */
  int rc;

  // Set up a listenting socket for this server.
  bind_and_listen(); /* aborts on error */

  /* Begin the server! Handle multiple client I/O indefinitely.
   * The only way the server exits is by being killed. */
  while (1) {
    /* Reset fd list. Turn on server listening socket*/
    int maxfd = listenfd;
    FD_ZERO(&fdlist);
    FD_SET(listenfd, &fdlist);

    /* Turn on each socket associated with a client.
     * Increase the client count (maxfd) if necessary. */
    for (p = client_list; p; p = p->next) {
        FD_SET(p->fd, &fdlist);
        if (p->fd > maxfd)
            maxfd = p->fd;
    }
    printf("Waiting for client activity.\n");
    /* Wait until there is activity on the sockets. */
    if ((rc = select(maxfd + 1, &fdlist, NULL, NULL, NULL)) < 0) {
        perror("select");

    }  else { // Successful select
            /* Find the first ready client.
             * This will ignore all other ready clients,
             * but that's ok, we'll get them on the next iteration */
            for (p = client_list; p; p = p->next)
                if (FD_ISSET(p->fd, &fdlist)) {
                    break; // Found the first ready client
                }
            /* Some client is ready, go and handle I/O. */
            if (p){
                printf("WHATSUP\n");
                whatsup(p);  /* might remove p from list, so can't be in the loop */
                printf("inbuf at end of while %d\n",p->inbuf);
            }
            /* There is a connection on the listening fd.
             * A new client has arrived! Let's go greet them. */
            if (FD_ISSET(listenfd, &fdlist)) {
                new_connection();
            }

        }
  }
  return(0);
}


void *safe_malloc(int nbytes) {
  void *target = malloc(nbytes);
  if (!target) {
      fprintf(stderr, "failed to allocate memory.\n");
      exit(1);
  }
  return target;
}

/* Search the first inbuf characters of buf for a network newline ("\r\n").
 * Return the location of the '\r' if the network newline is found,
 * or -1 otherwise. */
int find_network_newline(char *buf, int inbuf) {
  int count = 0; // current buffer index
  while (count <= inbuf) {
    if (buf[count] == '\r') {
      if (count < inbuf && buf[count+1] == '\n') {
          return count; // return the location of '\r'
      }
    }
    count++;
  }
  return -1; // Did not find network newline
}


/* If there is a client with this name, return their fd.
 * False otherwise */
int find_by_name(char *name) {
  struct client *cur = client_list;
  while (cur) {
    if (cur->user_name && !strncmp(cur->user_name, name, strlen(name))) {
      return cur->fd;
    }
    cur = cur->next;
  }
  return 0;
}


/* Set up the socket on which the server will listen. */
void bind_and_listen(void) {
  // struct to handle internet addresses
  struct sockaddr_in r;

  // Create a file descriptor for the server side
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }
  // Allow port to be reused shortly after quit
  int on = 1;
  int status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
      (const char *) &on, sizeof(on));
  if(status == -1) {
      perror("setsockopt -- REUSEADDR");
  }

  // Reset everything in r
  memset(&r, '\0', socklen);
  // Setup
  r.sin_family = AF_INET; //r.?
  r.sin_addr.s_addr = INADDR_ANY; // Communicate on and network interface
  r.sin_port = htons(PORT);

  // Bind PORT to given socket
  if (bind(listenfd, (struct sockaddr *)&r, socklen)) {
    perror("bind");
    exit(1);
  }
  // Begin listening to up to 5 clients on the server socket
  if (listen(listenfd, 5)) {
    perror("listen");
    exit(1);
    }
}

/* Add a client to the global client list with the given file descriptor
 * Return a pointer to the new client
 * Note that the username won't be set until we get is from the client */
void add_client(int fd, struct in_addr addr) {

  struct client *new_client = safe_malloc(sizeof(struct client));
  if (!new_client) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }
  printf("Adding client %s\n", inet_ntoa(addr));
  fflush(stdout);

  // instantiate new_client's variables
  new_client->fd = fd;
  new_client->ipaddr = addr;
  // We will get username later.
  new_client->user_name = NULL;
  new_client->buf = safe_malloc(INPUT_BUFFER_SIZE);
  // Initialize buf and associated info
  new_client->inbuf = 0; // buffer is empty; has no bytes

  // add the new client to head of the global client list
  new_client->next = client_list;
  client_list = new_client;
}

/* Remove a client to the global client list with the given file descriptor */
void remove_client(int fd) {
  struct client **cptr;
  // find the client with the given fd
  for (cptr = &client_list; *cptr && (*cptr)->fd != fd; cptr = &(*cptr)->next);
  if (cptr) { // remove client from client list
    struct client *next = (*cptr)->next;
    printf("Removing client %s\n", inet_ntoa((*cptr)->ipaddr));
    fflush(stdout);
    /* Free previously allocated memory. */
    free((*cptr)->user_name);
    free((*cptr)->buf);
    free(*cptr);
    /* remove client from client list*/
    *cptr = next;
  } else {
    fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
    fflush(stderr);
  }
}


/*
 * Return a calendar time representation of the time specified
 *  in local_args.
 *    local_argv[0] must contain hour.
 *    local_argv[1] may contain day, otherwise use current day
 *    local_argv[2] may contain month, otherwise use current month
 *    local_argv[3] may contain day, otherwise use current year
 */
time_t convert_time(int local_argc, char** local_argv) {

   time_t rawtime;
   struct tm * info;

   // Instead of expicitly setting the time, use the current time and then
   // change some parts of it.
   time(&rawtime);            // get the time_t represention of the current time
   info = localtime(&rawtime);

   // The user must set the hour.
   info->tm_hour = atof(local_argv[0]);

   // We don't want the current minute or second.
   info->tm_min = 0;
   info->tm_sec = 0;

   if (local_argc > 1) {
       info->tm_mday = atof(local_argv[1]);
   }
   if (local_argc > 2) {
       // months are represented counting from 0 but most people count from 1
       info->tm_mon = atof(local_argv[2])-1;
   }
   if (local_argc > 3) {
       // tm_year is the number of years after the epoch but users enter the year AD
       info->tm_year = atof(local_argv[3])-1900;
   }

   // start off by assuming we won't be in daylight savings time
   info->tm_isdst = 0;
   mktime(info);
   // need to correct if we guessed daylight savings time incorrect since
   // mktime changes info as well as returning a value. Check it out in gdb.
   if (info->tm_isdst == 1) {
       // this means we guessed wrong about daylight savings time so correct the hour
       info->tm_hour--;
   }
   return  mktime(info);
}

/*
 * Read and process calendar commands from the client
 * Return:  -1 for quit command
 *          0 otherwise
 * Success and error messages are witten to the client.
 * This function can remove clients.
 */
int process_arg(int fd, int cmd_argc, char **cmd_argv) {

  // Create pointers to the relevant global data structures
  Calendar **calendar_list_ptr = &calendar_list;
  User **user_list_ptr = &user_list;

  // Declare buffers that will be used to write
  // results of calendar funtions to the user
  char *msg;
  char buf[INPUT_BUFFER_SIZE];
  // char msg[INPUT_BUFFER_SIZE];
  // for subscribe broadcast
  int usr;

  printf("Calendar command 1 is: %s\n", cmd_argv[0]);
  printf("%d element(s) in the command\n", cmd_argc);


  if (cmd_argc <= 0) {
    printf("No args\n");

  } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) { //not working
    printf("quitting.\n");
    // struct client *client = find_client(fd);
    // printf("Disconnect from %s\n", inet_ntoa(client->ipaddr));
    // sprintf(msg, "Goodbye %s\r\n", inet_ntoa(client->ipaddr));
    // write(fd, msg, strlen(msg));
    // remove_client(fd); // this crashes. see func for why

    return -1;

  } else if (strcmp(cmd_argv[0], "add_calendar") == 0 && cmd_argc == 2) {
      if (add_calendar(calendar_list_ptr, cmd_argv[1]) == -1) {
          msg = "Calendar by this name already exists\r\n";
          write(fd, msg, strlen(msg));
      } else {
        printf("Successfully added calendar.\n");
      }

  } else if ((strncmp(cmd_argv[0], "add_user", strlen("add_user")) == 0) && cmd_argc == 2) {
    if (add_user(user_list_ptr, cmd_argv[1]) == -1) {
      msg = "User by this name already exists.\r\n";
      write(fd, msg, strlen(msg));
    } else {

      sprintf(buf, "User %s added.\r\n", cmd_argv[1]);
      write(fd, buf, strlen(buf));
    }

  } else if ((strncmp(cmd_argv[0], "add_event", strlen("add_event")) == 0) && cmd_argc >= 4) {
    // Parameters for convert_time are the values in the array cmd_argv but
    // starting at cmd_argv[3]. The first 3 are not part of the time.
    // So subtract these 3 from the count and pass the pointer
    // to element 3 (w the first is element 0).
    time_t time = convert_time(cmd_argc-3, &cmd_argv[3]);

    if (add_event(calendar_list, cmd_argv[1], cmd_argv[2], time) == -1) {
      msg = "Calendar by this name does not exist\r\n";
      write(fd, msg, strlen(msg));
    } else {
      printf("Successfully added event.\r\n");
    }
  } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
    msg = list_users(user_list);
    write(fd, msg, strlen(msg) + 1);

  } else if (strcmp(cmd_argv[0], "list_calendars") == 0 && cmd_argc == 1) {
    msg = list_calendars(calendar_list);
    printf("result:\n%s", msg);
    write(fd, msg, strlen(msg) + 1);

  } else if (strcmp(cmd_argv[0], "list_events") == 0 && cmd_argc == 2) {
    printf("getting events\n");
    msg = list_events(calendar_list, cmd_argv[1]);
    printf("result:\n%s", msg);
    write(fd, msg, strlen(msg) + 1);

  } else if (strcmp(cmd_argv[0], "subscribe") == 0 && cmd_argc == 3) {
    int return_code = subscribe(user_list, calendar_list, cmd_argv[1], cmd_argv[2]);
    if (return_code == -1) {
           msg = "User by this name does not exist\r\n";
           write(fd, msg, strlen(msg));
        } else if (return_code == -2) {
           msg = "Calendar by this name does not exist\r\n";
           write(fd, msg, strlen(msg));
        } else if (return_code == -3) {
           msg = "This user is already subscribed to this calendar\r\n";
           write(fd, msg, strlen(msg));
        } else if ((usr = find_by_name(cmd_argv[1])) > 0) {
          //broadcast to user that they have been subscribed
          sprintf(buf, "You have been subscribed to calendar %s.\r\n", cmd_argv[2]);
          write(usr, buf, strlen(buf));

        }

  } else { // No matching command was found
    msg = "Incorrect syntax\r\n";
    write(fd, msg, strlen(msg));
  }
  return 0;
}

/* Establish a connection with a new user. Prompt for username input*/
int new_connection() {

  int fd; //Socket to read and write to
  struct sockaddr_in r; //Client address

  if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
    perror("accept");

    } else { // successful connection
      printf("Calserv: connection from %s\n", inet_ntoa(r.sin_addr));
      add_client(fd, r.sin_addr); // add client to client list

      printf("greeting client.\n");

      // Prompt for the client username
      write(fd, greeting, sizeof greeting);
      write(fd, "> ", strlen("> "));
  }
  return fd;
}

/* This function handles client input. It updates and maintains the client's
 * buffer (which contains all the client's input since their last complete line.
 * It interprets full lines as either usernames or calander commands and
 * processes the line accordingly.
 * This function calls proccess_arg(). */
void whatsup(struct client *p) { /* select() said activity; check it out */
    printf("Client inbuf: %d\n", p->inbuf);
    int len, fd;

    // Vars for holding arguments to individual commands passed to sub-procedure
    char *cmd_argv[INPUT_ARG_MAX_NUM];
    int cmd_argc;

    // The clients file descriptor
    fd = p->fd;

    // Read from the client, record how much we got
    len = read(fd, p->buf + p->inbuf, INPUT_BUFFER_SIZE - p->inbuf);

    if (len < 0) {
        /* Read error. */

        write(fd, "Read error!", strlen("Read error!"));
        perror("read");
    } else {
      // Update how buf is so far (that we care about at least)
      p->inbuf += len;
      printf("read %d\n", len);
      printf("Client buf so far: %s\n", p->buf);
      if (len > 0) { /* If we got something, then process the clients buffer */
        printf("Reading from client #%d\n", fd);

        /* Check if the client has a complete line in their input buffer. */
        int w; // postition of network newline.
        w = find_network_newline(p->buf, p->inbuf + 1);
        if (w >= 0) { // Yes! We have a complete line
          /* Change the network newline to a null terminator,
           * so that we can use the buffer in string functions. */
          *(p->buf + w) = '\0';

          /* Check if the input is a username */
          if (!p->user_name) {
            if (!find_by_name(p->buf)) {
              // This is client's first input, ie their username. Let's set it
              p->user_name = safe_malloc(strlen(p->buf)); // sizeof?
              strncpy(p->user_name, p->buf, strlen(p->buf));
              printf("client username set to %s\n", p->buf);
              // Now we prompt the client to enter calendar commands
              write(fd, command_prompt, sizeof command_prompt);
            } else {
              write(fd, "Username taken\r\n", strlen("Username taken\r\n"));
            }
            write(fd, "> ", strlen("> ")); // prompt user

          /* username is set, input must be a calendar command */
          } else {
            // Copy user buf into a temporary buf that we can tokenize/mash up
            char input[strlen(p->buf)];
            strcpy(input, p->buf);

            // Tokenize commands
            char *next_token = strtok(input, DELIM);
            cmd_argc = 0;
            while (next_token) {
              if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
                write(fd, "Too many arguments\r\n", strlen("Too many arguments\r\n"));
                cmd_argc = 0;
                break;
              }
              cmd_argv[cmd_argc] = next_token;
              cmd_argc++;
              next_token = strtok(NULL, DELIM);
              printf("command just after tokenizing: %s\n", cmd_argv[0]);
            }
            // Execute appropriate calendar command
            printf("process calendar command\n");
            if (cmd_argc > 0 && process_arg(fd, cmd_argc, cmd_argv) == -1) {
              remove_client(fd); // can only reach if quit command was entered
            }
          write(fd, "> ", strlen("> ")); // prompt user
          }

          // Prepare for next read. Set where in buffer to write to next
          p->inbuf = p->inbuf - (w + 2);

          /* Move anything after the processed line to the front of the buffer*/
          int i, start;
          i = w;
          start = 0;
          while (start <= INPUT_BUFFER_SIZE){
            p->buf[start] = p->buf[i];
            start++;
            i++;
          }

        } else {
          printf("Still waiting for full line\n");
        }
      } else if (len == 0) { /* The client disconnected */
        printf("Disconnect from %s\n", inet_ntoa(p->ipaddr));
        fflush(stdout);
        remove_client(fd);
      }
    }
}
