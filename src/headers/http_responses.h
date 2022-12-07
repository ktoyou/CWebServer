#include <stdlib.h>
#include <string.h>

#define MAX_PAGE_SIZE 1024

char *configure_default_response_page(char *content) {
    char *page = (char*)calloc(MAX_PAGE_SIZE, sizeof(char));
    strcat(page, "<h1 style=\"display:flex;align-items:center;height: 200px;justify-content:center;\">");
    strcat(page, content);
    strcat(page, "</h1>");
    return page;
}

void put_default_page_to_buffer(char *page_buffer, char *buffer) {
    strcat(buffer, page_buffer);
    free(page_buffer);
}

char *get_forbidden_page() {
    return configure_default_response_page("403 Forbidden");
}

char *get_not_found_page() {
    return configure_default_response_page("404 Not Found");;
}

char *get_internal_server_error_page() {
    return configure_default_response_page("500 Internal Server Error");;
}
