#ifndef ADMIN_PARSER_H
#define ADMIN_PARSER_H

#include "admin_socket.h"

void print_msg_received(char msg_received[BUFFER_MAX], int len);
bool parse_msg_received(bool * logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings, metrics_t metrics);
void parse_ok_response(char msg_response[BUFFER_MAX], int *msg_response_len);
void parse_err_response(char msg_response[BUFFER_MAX], int *msg_response_len);
void parse_login(bool * logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int *msg_response_len);
void parse_logout(char msg_response[BUFFER_MAX], int * msg_response_len);
void parse_concurrent_connections(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, metrics_t metrics);
void parse_total_connections(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, metrics_t metrics);
void parse_bytes_transfered(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, metrics_t metrics);
void parse_get_mtypes(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings);
void parse_get_cmd(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings);
void parse_set_cmd(bool *logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings);
void parse_set_mtypes(bool *logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings);
void parse_enable_mtype_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings);
void parse_enable_cmd_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings);
void parse_disable_mtype_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings);
void parse_disable_cmd_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings);
void parse_int_to_msg(int number, char msg_response[BUFFER_MAX], int * msg_response_len);

#endif