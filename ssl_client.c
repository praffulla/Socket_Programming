#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>

int connectToServer () {

    struct sockaddr_in server_addr;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_addr, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
   
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) ;

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        close(socket);
        perror("Connect failed");
        return -1;
    }

    return sock;
}

void show_server_certs (SSL *ssl) {

    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       
        X509_free(cert);     
    }
    else
        printf("Info: No peer certificates provided.\n");
}

int main () {

    SSL_METHOD *method;
    SSL_CTX *ctx;
    int client_fd;
    SSL *ssl;
    char buff[1024] = {0};
    int bytes;

    /*Initialize SSL library*/
    SSL_library_init();
    SSL_load_error_strings();

    /*Create SSL factory (SSL_CTX)*/
    method = TLSv1_2_client_method();
    ctx = SSL_CTX_new(method);
    
    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    client_fd = connectToServer();
    if (client_fd == -1) {
        printf("TCP connect failed");
        return -1;
    }

    /*Point new SSL connection to SSL_CTX which store Certificates and Keying info*/
    ssl = SSL_new(ctx);

    /*In this cases, socket BIO is automatically created to interface ssl and client_fd */
    SSL_set_fd(ssl, client_fd);
    
    if (SSL_connect(ssl) < 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }

    show_server_certs(ssl);       
        
    sprintf(buff, "Hello from SSL client", strlen("Hello from SSL client")+1);
    SSL_write(ssl, buff, 1024);

    /*Here after a complete record received, decryption and check of integrity happens*/
    bytes = SSL_read(ssl, buff, sizeof(buff));
    buff[bytes] = 0;
    printf("Received: %s\n", buff);
    SSL_free(ssl);
    return 0;
}
