#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "http_headers.h"
#include "http_responses.h"
#include "http_routes.h"

#define MAX_FILE_SIZE       128000
#define MAX_BUFFER_SIZE     128000
#define MAX_PATH_SIZE       1024

struct http_request {
    int request_fd;
    struct http_request_header header;
};

struct http_server_config {
    int port;
    int max_queue;
    char *addr;
    char *content_folder;
};

struct http_server_info {
    int fd;
};

struct sockaddr_in configure_addr(char *ip, int port) {
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    return addr;
}

struct http_server_info configure_http_server(struct http_server_config config) {
    struct http_server_info info;
    info.fd = -1;

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0) {
        perror("Error: ");
    }

    struct sockaddr_in addr = configure_addr(config.addr, config.port);
    socklen_t addr_size = sizeof(addr);

    int bind_status = bind(fd, (const struct sockaddr*)&addr, addr_size);
    if(bind_status < 0) {
        perror("Error bind: ");
        exit(-1);
    }

    info.fd = fd;
    return info;
}

int accept_connection(int fd, struct sockaddr *accepted_addr, socklen_t *accepted_addr_size) {
    int accepted_fd = accept(fd, accepted_addr, accepted_addr_size);
    if(accepted_fd < 0) {
        perror("Error accept connection: \n");
    } else {
        printf("Accepted connection\n");
    }
    return accepted_fd;
}

struct http_request_header read_http_request_header(int request_fd) {
    char buf[MAX_BUFFER_SIZE];
    read(request_fd, buf, MAX_BUFFER_SIZE);
    return convert_buffer_to_http_request_header(buf);
}

struct http_request accept_http_request(int fd, struct sockaddr *accepted_addr, socklen_t *accepted_addr_size) {
    int client_fd = accept_connection(fd, accepted_addr, accepted_addr_size);
    
    struct http_request request;
    request.request_fd = client_fd;
    request.header = read_http_request_header(client_fd);

    return request;
}

int start_listening(struct http_server_info server_info, struct http_server_config server_config) {
    int listen_status = listen(server_info.fd, server_config.max_queue);
    if(listen_status < 0) {
        perror("Error listen: \n");
        exit(-1);
        return -1;
    }

    return listen_status;
}

int start_http_server(struct http_server_info *server_info, struct http_server_config *server_config, struct http_route_node *routes) {    
    start_listening(*server_info, *server_config);

    printf("Listening on %s:%i\n", server_config->addr, server_config->port);

    while(1) {
        struct sockaddr accepted_addr;
        socklen_t accepted_addr_size;

        struct http_request http_request = accept_http_request(server_info->fd, &accepted_addr, &accepted_addr_size);

        if(http_request.request_fd > 0) {
            struct http_route_node *route = find_http_route_node_by_route_name(http_request.header.path, routes);
            printf("%s request %s\n", http_request.header.method, http_request.header.path);

            char *buffer = (char*)calloc(MAX_BUFFER_SIZE, sizeof(char));
            char *full_path = (char*)calloc(MAX_PATH_SIZE, sizeof(char));

            if(route) {
                strcat(full_path, server_config->content_folder);
                strcat(full_path, route->file);

                FILE *file = NULL;
                file = fopen(full_path, "r");
                
                if(file) {
                    put_http_header_to_buffer(get_successful_http_header(), buffer);

                    char content[MAX_FILE_SIZE];
                    ssize_t readed = fread(content, 1, MAX_FILE_SIZE, file);
                    memcpy(buffer + HTTP_HEADER_SIZE * sizeof(char), content, readed);

                    fclose(file);
                } else {
                    put_http_header_to_buffer(get_internal_server_error_http_header(), buffer);
                    put_default_page_to_buffer(get_internal_server_error_page(), buffer);
                }
            } else {
                put_http_header_to_buffer(get_not_found_http_header(), buffer);
                put_default_page_to_buffer(get_not_found_page(), buffer);
            }   

            ssize_t sended_bytes = send(http_request.request_fd, buffer, MAX_BUFFER_SIZE, 0);
            printf("Sended %zd bytes\n", sended_bytes);

            free(buffer);
            free(full_path);
            close(http_request.request_fd);
        }
    }

    return 0;
}