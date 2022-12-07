#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "http_headers.h"

#define MAX_FILE_SIZE       128000
#define MAX_BUFFER_SIZE     128000
#define MAX_PATH_SIZE       1024
#define MAX_ROUTE_FILE_SIZE 1024

struct http_server_config {
    int port;
    int max_queue;
    char *addr;
    char *content_folder;
};

struct http_route_node {
    char *route_name;
    char *file;
    struct http_route_node *next;
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

struct http_route_node *find_http_route_node_by_route_name(char *route_name, struct http_route_node *root) {
    struct http_route_node *current = root;
    while(current) {
        if(strcmp(route_name, current->route_name) == 0) {
            return current;
        }
        current = current->next;
    }
}

struct http_route_node *find_http_route_node_by_file(char *file, struct http_route_node *root) {
    struct http_route_node *current = root;
    while(current) {
        if(strcmp(file, current->file)) {
            return current;
        }
        current = current->next;
    }
}

int load_http_routes(char *path, struct http_route_node **root) {
    FILE *file;
    file = fopen(path, "r");

    char *buffer = (char*)calloc(MAX_ROUTE_FILE_SIZE, sizeof(char));
    struct http_route_node *current = NULL;

    if(file) {
        ssize_t bytes = fread(buffer, 1, MAX_ROUTE_FILE_SIZE, file);

        char *current_line = strtok(buffer, "\n");
        int offset = 0;

        while (current_line)
        {
            struct http_route_node *next = (struct http_route_node*)malloc(sizeof(struct http_route_node));
            offset += strlen(current_line);

            char *route = strtok(current_line, "::");
            next->route_name = route;

            char *path = strtok(NULL, "::");
            next->file = path;

            next->next = NULL;

            if(!current) *root = next;
            else current->next = next;

            offset++;
            current_line = strtok(buffer + offset, "\n");
            current = next;
        }
    }

    return 0;
}

int start_http_server(struct http_server_info server_info, struct http_server_config server_config, struct http_route_node *routes) {    
    int listen_status = listen(server_info.fd, server_config.max_queue);
    if(listen_status < 0) {
        perror("Error listen: \n");
        return -1;
    }

    printf("Listening on %s:%i\n", server_config.addr, server_config.port);

    while(1) {
        struct sockaddr accepted_addr;
        socklen_t accepted_addr_size;

        int accepted_fd = accept_connection(server_info.fd, &accepted_addr, &accepted_addr_size);

        if(accepted_fd > 0) {
            char *buffer = (char*)calloc(MAX_BUFFER_SIZE, sizeof(char));
            char *full_path = (char*)calloc(MAX_PATH_SIZE, sizeof(char));

            struct http_request_header request_header = read_http_request_header(accepted_fd);
            struct http_route_node *route = find_http_route_node_by_route_name(request_header.path, routes);
            printf("%s request %s\n", request_header.method, request_header.path);
            
            if(route) {
                strcat(full_path, server_config.content_folder);
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
                }
            } else {
                put_http_header_to_buffer(get_not_found_http_header(), buffer); 
            }

            ssize_t sended_bytes = send(accepted_fd, buffer, MAX_BUFFER_SIZE, 0);
            printf("Sended %zd bytes\n", sended_bytes);

            free(buffer);
            free(full_path);
            close(accepted_fd);
        }
    }

    return 0;
}