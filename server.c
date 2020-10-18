#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 

#define PORT 8080

int main () {
    
    int listen_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in cli_addr;
    int rc;
    int client_array[FD_SETSIZE];
    int maxfd;
    int num_ready;
    int cli_len;
    int conn_fd;
    char buf[INET_ADDRSTRLEN];
    char read_buf[1024];
    int i = 0;

    fd_set    rset, allset;
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
    maxfd = listen_fd;
    FD_ZERO(&allset);
    FD_SET(listen_fd, &allset);

    for (i = 0; i < FD_SETSIZE; i++)
        client_array[i] = -1;

    int max_client = -1;
    for (; ;) {
        rset = allset;
        num_ready = select(maxfd+1, &rset, 0, 0, 0);
        cli_len = sizeof(cli_addr);
        if (FD_ISSET(listen_fd, &rset)) {
            /*A new connection arrived*/
            conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);
            if (conn_fd  < 0) {
                perror("accept failure");
                return -1;
            }
            
            inet_ntop(AF_INET, &cli_addr, buf, INET_ADDRSTRLEN);
            printf ("New connection from %s address and %d port:\n", buf, ntohs(cli_addr.sin_port));
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client_array[i] < 0) {
                    client_array[i] = conn_fd;
                    break;
                }
            }            
 
            FD_SET(conn_fd, &allset);
           
             if (i > max_client)                                                 
                max_client = i; 
 
            if (conn_fd > listen_fd)
                maxfd = conn_fd;
                
            if (i == FD_SETSIZE) {
                perror("Reached max fd size available");
                return -1;
            }
    
        }
       
        for (i = 0; i <= max_client; i++) {
            if (client_array[i] < 0) {
                continue;
            }
            if (FD_ISSET(client_array[i], &rset)) {
                int n = read(client_array[i], read_buf, 1024);
                if (n == 0) {
                    /*Got FIN from client*/
                    printf("Got FIN from client\n");
                    close(client_array[i]);
                    client_array[i] = -1;
                    FD_CLR(client_array[i], &allset);
                }else {
                    printf("Writing back to client: %s\n", read_buf);
                    write(client_array[i], read_buf, 1024);
                }
            }
        }
    }                
    return 0;
}
