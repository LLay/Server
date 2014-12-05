
int main(int argc, char **argv) {

	struct client *cur;

	//getopt?

	bind_and_listen();




	while(1) {
		fd_set fdlist; // file descriptor set for select function
        int maxfd = listenfd;
        FD_ZERO(&fdlist); // (For security) Initialize fdlist to be the empty set
        FD_SET(listenfd, &fdlist); // add listening socket to file descriptor set

        for (cur = client_list; cur; cur = cur->next) { // find the first ready client
            FD_SET(cur->fd, &fdlist); // add client file descriptors to our fd set
            if (cur->fd > maxfd) // increase the max of our fd values as appropriate
                maxfd = cur->fd;
		}
		printf("selecting\n");
		if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
            perror("select");

        } else {
        	printf("finding ready client\n");
        	for (cur = client_list; cur; cur = cur->next) {
        		if (FD_ISSET(cur->fd, &fdlist)) {
        			printf("A client is ready\n");
        			break; // Found a client that is ready.
        		}
        	printf("new connection\n");
        	new_connection();
        	}
        }
	}
}

// 11:30 pm
int main(int argc, char **argv) {

    struct client *cur;

    //getopt?

    bind_and_listen();

    while(1) {
        fd_set fdlist; // file descriptor set for select function
        int maxfd = listenfd;
        FD_ZERO(&fdlist); // (For security) Initialize fdlist to be the empty set
        FD_SET(listenfd, &fdlist); // add listening socket to file descriptor set

        for (cur = client_list; cur; cur = cur->next) { // find the first ready client
            FD_SET(cur->fd, &fdlist); // add client file descriptors to our fd set
            if (cur->fd > maxfd) // increase the max of our fd values as appropriate
                maxfd = cur->fd;
        }
        printf("selecting\n");
        if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
            perror("select");

        } else {
  //        printf("finding ready client\n");
  //        for (cur = client_list; cur; cur = cur->next) {
  //            if (FD_ISSET(cur->fd, &fdlist)) {
  //                printf("A client is ready\n");
                    // break; Found a client that is ready.
  //            }
            printf("new connection\n");
            new_connection();
        //  }
        }
    }
}

// GNU server example AND muffinman - wait. did i just paste this? this  is not the same as what i have in calserver. I'm losing it
int main(int argc, char **argv)
{
    int c;
    struct client *p;
    void bindandlisten(void), newconnection(void), whatsup(struct client *p);
    printf("getopt\n");
    while ((c = getopt(argc, argv, "p:")) != -1) {
        if (c == 'p') {
            if ((port = atoi(optarg)) == 0) {
                fprintf(stderr, "%s: non-numeric port \"number\"\n", argv[0]);
                return(1);
            }
        } else {
            fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
            return(1);
        }
    }
    printf("binding\n");
    // bindandlisten();  /* aborts on error */
    bind_and_listen();

    /* the only way the server exits is by being killed */
    while (1) {
        fd_set fdlist;
        int maxfd = listenfd;
        FD_ZERO(&fdlist);
        FD_SET(listenfd, &fdlist);

        for (p = client_list; p; p = p->next) {
            FD_SET(p->fd, &fdlist);
            if (p->fd > maxfd)
                maxfd = p->fd;
        }
        printf("selecting\n");
        int rc;
        if ((rc = select(maxfd + 1, &fdlist, NULL, NULL, NULL)) < 0) {
            perror("select");
        }
        if (rc > 0) {
            printf("Num ready clients: %d\n", rc);
            for (p = client_list; p; p = p->next) // find the first ready client
                if (FD_ISSET(p->fd, &fdlist)) { // googling told me that FD_ISSET returns 0 FOR FALSE. what??
                    printf("A client is ready\n");
                    break; // p is now the first ready client
                }
                /*
                 * it's not very likely that more than one client will drop at
                 * once, so it's not a big loss that we process only one each
                 * select(); we'll get it later...
                 */
            if (p){ // read form a previous client
                printf("whatsup\n");
                // whatsup(p);  /* might remove p from list, so can't be in the loop */
              // 1. Get input
              // 2. check if completed line
              // 3. if so process line
            }
            if (FD_ISSET(listenfd, &fdlist)) { // Read from a new client
                printf("new connection\n");

                // new_connection();
        int fd; //Socket to read and write to
        struct sockaddr_in r; //Client address

        if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
          perror("accept");

          } else { // successful connection
            printf("Calserv: connection from %s\n", inet_ntoa(r.sin_addr));
            add_client(listenfd, r.sin_addr); // add client to client list

            printf("greeting client.\n");

            // Ask for the client username
            write(fd, greeting, sizeof greeting);
          }

            }
        }
    }

    return(0);
}


// tues morn
int main(int argc, char **argv)
{
    int c;
    struct client *p;
    void bindandlisten(void), newconnection(void), whatsup(struct client *p);
    printf("getopt\n");
    while ((c = getopt(argc, argv, "p:")) != -1) {
        if (c == 'p') {
            if ((port = atoi(optarg)) == 0) {
                fprintf(stderr, "%s: non-numeric port \"number\"\n", argv[0]);
                return(1);
            }
        } else {
            fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
            return(1);
        }
    }
    printf("binding\n");
    // bindandlisten();  /* aborts on error */
    bind_and_listen();

    fd_set fdlist, rset;
    int maxfd = listenfd;
    FD_ZERO(&fdlist);
    FD_SET(listenfd, &fdlist);

    //sockfd = listenfd
    //allset = fdlist

    // using http://www.gnu.org/software/libc/manual/html_node/Server-Example.html

    /* the only way the server exits is by being killed */
    while (1) {

        rset = fdlist;
        // for (p = client_list; p; p = p->next) {
        //     FD_SET(p->fd, &rset);
        //     if (p->fd > maxfd)
        //         maxfd = p->fd;
        // }

        printf("selecting\n");
        int rc;
        if ((rc = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select");
        }
        if (rc > 0) {
            printf("Num ready clients: %d\n", rc);
            for (p = client_list; p; p = p->next) {
                if (FD_ISSET(p->fd, &rset)) {
                    printf("A client is ready\n");
                    break;
                }
            }

            if (p){ // read form a previous client
                printf("whatsup\n");
                // whatsup(p);  /* might remove p from list, so can't be in the loop */
                // 1. Get input
                // 2. check if completed line
                // 3. if so process line
                close(p->fd);
                FD_CLR(p->fd, &fdlist);

            }
            // if (p->fd = listenfd) {
            //  printf("New connection\n");
            // } else {
            //  printf("reading from old connection\n");
            // }

            if (FD_ISSET(listenfd, &rset)) { // Read from a new client
                printf("new connection\n");

                // new_connection();
                int fd; //Socket to read and write to
                struct sockaddr_in r; //Client address

                if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
                    perror("accept");

                } else { // successful connection
                    printf("Calserv: connection from %s\n", inet_ntoa(r.sin_addr));
                    add_client(listenfd, r.sin_addr); // add client to client list

                    FD_SET(fd, &fdlist);
                    printf("greeting client.\n");

                    // Ask for the client username
                    write(fd, greeting, sizeof greeting);
                }
            }
        }
    }

    return(0);
}


// GNU server example
int main(int argc, char **argv)
{
  int c;
  // struct client *p;
  void bindandlisten(void), newconnection(void), whatsup(struct client *p);
  printf("getopt\n");
  while ((c = getopt(argc, argv, "p:")) != -1) {
      if (c == 'p') {
          if ((port = atoi(optarg)) == 0) {
              fprintf(stderr, "%s: non-numeric port \"number\"\n", argv[0]);
              return(1);
          }
      } else {
          fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
          return(1);
      }
  }
  printf("binding\n");
  // bindandlisten();  /* aborts on error */
  bind_and_listen();

  fd_set fdlist, rset;
  int maxfd = listenfd;
  FD_ZERO(&fdlist);
  FD_SET(listenfd, &fdlist);
  int rc, i;
  //sockfd = listenfd
  //allset = fdlist

  // using http://www.gnu.org/software/libc/manual/html_node/Server-Example.html

  /* the only way the server exits is by being killed */
  while (1) {

    rset = fdlist;
      // for (p = client_list; p; p = p->next) {
      //     FD_SET(p->fd, &rset);
      //     if (p->fd > maxfd)
      //         maxfd = p->fd;
      // }

    printf("selecting\n");

    if ((rc = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
        perror("select");
    }
    if (rc > 0) {
        printf("Num ready clients: %d\n", rc);
        for (i = 0; i <= maxfd; i++) { // service all ready clients
            if (FD_ISSET(i, &rset)) {
              printf("A client is ready\n");

              if (i == listenfd) {
              /* Connection request on original socket. */
                struct sockaddr_in r; //Client address
                int fd;
                if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
                perror("accept");

                } else { // successful connection
                  printf("Calserv: connection from %s\n", inet_ntoa(r.sin_addr));

                  add_client(listenfd, r.sin_addr); // add client to client list

                  FD_SET(fd, &fdlist);

                  printf("greeting client.\n");

                  // Ask for the client username
                  write(fd, greeting, sizeof greeting);
                FD_SET(fd, &fdlist);
                }

              } else { /* Data arriving on an already-connected socket. */
                printf("connection from existing client.\n");
              }
            }
          }
      }
  }
  return(0);
}