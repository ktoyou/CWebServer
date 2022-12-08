#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ROUTE_FILE_SIZE 1024

struct http_route_node {
    char *route_name;
    char *file;
    struct http_route_node *next;
};


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