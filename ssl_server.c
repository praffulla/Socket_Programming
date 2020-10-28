#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

int openListener(int port) {
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "Bind failed");
        return -1;
    }

    listen(sock, 0);

    return sock;
}

int main () {

    SSL_METHOD *method;
    SSL_CTX *ctx;


    printf("HERe");
    SSL_library_init();
    SSL_load_error_strings();
    int server;
    char buff[1024];
    int bytes;

    printf("here 1");   
    /*Prepare the SSL factory*/
    method = TLSv1_2_server_method();
    ctx = SSL_CTX_new(method);

    /*Load PrivateKey and Certificate in ctx*/
    if (SSL_CTX_use_certificate_file(ctx, "mycert.pem", SSL_FILETYPE_PEM) <= 0) {
       ERR_print_errors_fp(stderr);
       return -1;
    } 
    fprintf(stderr, "here 2"); 

    if (SSL_CTX_use_PrivateKey_file(ctx, "mycert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }       
        
    /*Check private key and Certificate pair i.e, verify private key*/
   if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "PrivateKey doesn't match with public certificate\n");
        return -1;
    }

    server = openListener(8080);
    if (server == -1) {
        fprintf(stderr, "Failed to open Socket on server side");
        return -1;
    }
    fprintf(stderr, "Connected to client\n");
    while (1) {
        struct sockaddr_in addr;
        int len = sizeof(addr);
        SSL *ssl;
        char s_reply[1024];

        int client = accept(server, (struct sockaddr*)&addr, &len);
        //printf ("Connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);
        
        SSL_accept(ssl);

        /*Read from client*/
        bytes =SSL_read(ssl, buff, 1024);      
        if (bytes == -1) {
            fprintf(stderr, "Read error\n");
            int err = SSL_get_error(ssl, bytes);
            fprintf (stderr, "Error is %d\n", err);
        } 
        fprintf(stderr, "Read bytes from client is %d\n", bytes); 
        buff[bytes] = '\0';
       
        fprintf(stderr, "Message from client is %s\n", buff); 
        sprintf(s_reply, "Got your message: %s", buff);       
        SSL_write(ssl, s_reply, 1024);

        SSL_free(ssl);
        close(client);
    }


    return 0;
}
