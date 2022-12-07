#include "headers/http_server.h"

int main() {
    struct http_server_config config;
    config.addr = "127.0.0.1";
    config.content_folder = "src/";
    config.port = 8080;

    struct http_route_node *routes = NULL;
    load_http_routes("routes", &routes);

    struct http_server_info http_info = configure_http_server(config);
    start_http_server(http_info, config, routes);
}