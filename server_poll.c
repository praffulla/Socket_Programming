#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <poll.h>
#include <errno.h>

#define PORT 8080

int main () {
    
    int listen_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in cli_addr;
    int rc;
    int num_ready;
    int cli_len;
    int conn_fd;
    char buf[INET_ADDRSTRLEN];
    char read_buf[1024];
    int i = 0;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Socket create failed");
        return -1;
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (rc < 0) {
        perror("Bind failed");
        return -1;
    }
    
    listen(listen_fd, 10);
 
    /*Initialize data structure for select*/
    /*
        struct pollfd
        {
            int fd;                        
            short int events;              
            short int revents;             
        };

    */

    struct pollfd client[100];
    
    client[0].fd = listen_fd;
    /*POLLRDNORM: Normal data may be read without blocking*/
    client[0].events = POLLRDNORM;
    for (i =1; i < 100; i++) {
        client[i].fd = -1;
    }

    int maxi = 0;
    int clilen = 0;
    int connfd;

    for (; ;) {
        num_ready = poll(client, maxi+1, -1);
        if (client[0].revents & POLLRDNORM) {
            /*a new connection arrives on server*/
            clilen = sizeof(cli_addr);
            connfd = accept(listen_fd, (struct sockaddr*)&cli_addr, &clilen);
            printf("a new connection arrived from client\n");

            for (i = 1; i < 100; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }    
             
            client[i].events = POLLRDNORM;
    
            if (i > maxi)
                maxi = i;

        }

        /*There might be data ready read socket*/

        for (i = 1; i < 100; i++) {

            if (client[i].fd < 0) {
                continue;
            }

            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                /* now three cases can be there
                1. read < 0 i.e, either RST was sent or read itsef failed
                2. read == 0, i.e, FIN was sent
                3. read was fine
                */

                int n = read(client[i].fd, read_buf, 1024);
                           
                if (n < 0) {
                    if (errno == ECONNRESET) {
                        /*Connection reset*/
                        printf("Connection aborted by client\n");
                        close(client[i].fd);            
                        client[i].fd = -1;
                    }else {
                        printf("read error");
                        exit(1); 
                    }
                } else if (n == 0) {
                    /*FIN arrived*/
                    printf ("Connection closed by client\n");
                    close(client[i].fd);
                    client[i].fd = -1;    
                }else {
                    printf("Client send %s\n", read_buf);
                    write(client[i].fd, read_buf, 1024);
                }
            }
        }    
    }
    return 0;
}
