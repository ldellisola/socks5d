cmake_minimum_required(VERSION 3.16)
project(parsers C)

set(CMAKE_C_STANDARD 11)

set(
        SOURCES
        socks5/auth_parser.c
        socks5/hello_parser.c
        socks5/request_parser.c
        lulu/client_hello_parser.c
        lulu/client_goodbye_parser.c
        lulu/client_timeout_parser.c
        lulu/client_get_timeout_parser.c
        lulu/client_set_buffersize_parser.c
        lulu/client_get_buffersize_parser.c
        lulu/client_metrics_parser.c
        lulu/client_set_user_parser.c
        lulu/client_list_users_parser.c
        lulu/client_del_user_parser.c
        pop3/pop3_auth_parser.c)

add_library(parsers STATIC ${SOURCES})
target_link_libraries(parsers PUBLIC utils)