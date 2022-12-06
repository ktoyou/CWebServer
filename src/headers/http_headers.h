#include <stdlib.h>
#include <string.h>

#define CONTENT_TYPE_TEXT_HTML     "Content-Type: text/html\r\n"

#define CONNECTION_CLOSE           "Connection: close\r\n";
#define CONNECTION_KEEP_ALIVE      "Connection: keep-alive\r\n";

#define HTTP_NOT_FOUND             "HTTP/1.1 404 NOT FOUND\r\n"
#define HTTP_SUCCESSFUL            "HTTP/1.1 200 OK\r\n"
#define HTTP_FORBIDDEN             "HTTP/1.1 403 FORBIDDEN\r\n"
#define HTTP_INTERNAL_SERVER_ERROR "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n"

#define HTTP_HEADER_SIZE           256

struct http_header {
    char *http_status;
    char *content_type;
    char *connection;
};

char *convert_http_header_to_buffer(struct http_header header) {
    char *buffer = (char*)calloc(HTTP_HEADER_SIZE, sizeof(char));

    strcat(buffer, header.http_status);
    strcat(buffer, header.connection);
    strcat(buffer, header.content_type);
    strcat(buffer, "\r\n");

    return buffer;
}

void put_http_header_to_buffer(struct http_header header, char *buffer) {
    strcat(buffer, header.http_status);
    strcat(buffer, header.connection);
    strcat(buffer, header.content_type);
    strcat(buffer, "\r\n");
}

struct http_header get_successful_http_header() {
    struct http_header header;
    header.http_status = HTTP_SUCCESSFUL;
    header.connection = CONNECTION_CLOSE;
    header.content_type = CONTENT_TYPE_TEXT_HTML;

    return header;
}

struct http_header get_not_found_http_header() {
    struct http_header header;
    header.http_status = HTTP_NOT_FOUND;
    header.connection = CONNECTION_CLOSE;
    header.content_type = CONTENT_TYPE_TEXT_HTML;

    return header;
}

struct http_header get_forbidden_http_header() {
    struct http_header header;
    header.http_status = HTTP_FORBIDDEN;
    header.connection = CONNECTION_CLOSE;
    header.content_type = CONTENT_TYPE_TEXT_HTML;

    return header;
}

struct http_header get_internal_server_error_http_header() {
    struct http_header header;
    header.http_status = HTTP_INTERNAL_SERVER_ERROR;
    header.connection = CONNECTION_CLOSE;
    header.content_type = CONTENT_TYPE_TEXT_HTML;

    return header;
}
