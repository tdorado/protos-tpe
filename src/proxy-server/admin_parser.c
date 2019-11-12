#include <stdio.h>
#include <string.h>

#include "include/admin_parser.h"

void print_msg_received(char msg_received[BUFFER_MAX], int len){
    if(len > 0){
        switch (*msg_received) {
            case LOGIN:
                printf("LOGIN %s\n", msg_received + 1);
                break;
            case LOGOUT:
                printf("LOGOUT\n");
                break;
            case GET_CONCURRENT_CONNECTIONS:
                printf("GET CONCURRENT CONNECTIONS\n");
                break;
            case GET_TOTAL_CONNECTIONS:
                printf("GET TOTAL CONNECTIONS\n");
                break;
            case GET_BYTES_TRANSFERED:
                printf("GET BYTES TRANSFERED\n");
                break;
            case GET_MTYPES:
                printf("GET MTYPES\n");
                break;
            case GET_CMD:
                printf("GET CMD\n");
                break;
            case SET_CMD:
                printf("SET CMD %s\n", msg_received + 1);
                break;
            case SET_MTYPE:
                printf("SET MTYPE %s\n", msg_received + 1);
                break;
            case RM_MTYPE:
                printf("RM MTYPE %s\n", msg_received + 1);
                break;
            case ENABLE_MTYPE_TRANSFORMATIONS:
                printf("ENABLE MTYPE TRANSFORMATION\n");
                break;
            case ENABLE_CMD_TRANSFORMATIONS:
                printf("ENABLE CMD TRANSFORMATION\n");
                break;
            case DISABLE_TRANSFORMATIONS:
                printf("DISABLE TRANSFORMATIONS\n");
                break;
            default:
                printf("NOT A COMMAND\n");
                break;
        }
    }
}

bool parse_msg_received(bool * logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings, metrics_t metrics) {
    bool ret = false;
    switch (*msg_received) {
        case LOGIN:
            parse_login(logged, msg_received, msg_received_len, msg_response, msg_response_len);
            break;
        case LOGOUT:
            parse_logout(msg_response, msg_response_len);
            ret = true;
            break;
        case GET_CONCURRENT_CONNECTIONS:
            parse_concurrent_connections(logged, msg_response, msg_response_len, metrics);
            break;
        case GET_TOTAL_CONNECTIONS:
            parse_total_connections(logged, msg_response, msg_response_len, metrics);
            break;
        case GET_BYTES_TRANSFERED:
            parse_bytes_transfered(logged, msg_response, msg_response_len, metrics);
            break;
        case GET_MTYPES:
            parse_get_mtypes(logged, msg_response, msg_response_len, settings);
            break;
        case GET_CMD:
            parse_get_cmd(logged, msg_response, msg_response_len, settings);
            break;
        case SET_CMD:
            parse_set_cmd(logged, msg_received, msg_received_len, msg_response, msg_response_len, settings);
            break;
        case SET_MTYPE:
            parse_set_mtype(logged, msg_received, msg_received_len, msg_response, msg_response_len, settings);
            break;
        case RM_MTYPE:
            parse_rm_mtype(logged, msg_received, msg_received_len, msg_response, msg_response_len, settings);
            break;
        case ENABLE_MTYPE_TRANSFORMATIONS:
            parse_enable_mtype_transformations(logged, msg_response, msg_response_len, settings);
            break;
        case ENABLE_CMD_TRANSFORMATIONS:
            parse_enable_cmd_transformations(logged, msg_response, msg_response_len, settings);
            break;
        case DISABLE_TRANSFORMATIONS:
            parse_disable_transformations(logged, msg_response, msg_response_len, settings);
            break;
        default:
            parse_err_response(msg_response, msg_response_len);
            break;
    }
    return ret;
}

void parse_ok_response(char msg_response[BUFFER_MAX], int * msg_response_len) {
    msg_response[0] = OK;
    *msg_response_len = 1;
}

void parse_err_response(char msg_response[BUFFER_MAX], int * msg_response_len) {
    msg_response[0] = ERR;
    *msg_response_len = 1;
}

void parse_login(bool * logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int *msg_response_len) {
    if(!(*logged) && msg_received_len == 9 && strncmp(msg_received + 1, "TOKENAZO", 8) == 0){
        parse_ok_response(msg_response, msg_response_len);
        *logged = true;
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_logout(char msg_response[BUFFER_MAX], int * msg_response_len) {
    parse_ok_response(msg_response, msg_response_len);
}

void parse_concurrent_connections(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, metrics_t metrics) {
    if(*logged) {
        parse_ok_response(msg_response, msg_response_len);
        parse_int_to_msg(metrics->concurrent_connections, msg_response, msg_response_len);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_total_connections(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, metrics_t metrics) {
    if(*logged){
        parse_ok_response(msg_response, msg_response_len);
        parse_int_to_msg(metrics->total_connections, msg_response, msg_response_len);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_bytes_transfered(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, metrics_t metrics) {
    if(*logged){
        parse_ok_response(msg_response, msg_response_len);
        parse_int_to_msg(metrics->bytes_transfered, msg_response, msg_response_len);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_get_mtypes(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings) {
    if(*logged) {
        parse_ok_response(msg_response, msg_response_len);
        *msg_response_len += strlen(settings->media_types) + 1;
        strcpy(msg_response + 1, settings->media_types);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_get_cmd(bool * logged, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings) {
    if(*logged) {
        parse_ok_response(msg_response, msg_response_len);
        *msg_response_len += strlen(settings->cmd) + 1;
        strcpy(msg_response + 1, settings->cmd);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_set_cmd(bool *logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings) {
    if(*logged) {
        strncpy(msg_received + 1, settings->cmd, msg_received_len);
        parse_ok_response(msg_response, msg_response_len);
        *msg_response_len += strlen(settings->cmd) + 1;
        strcpy(msg_response + 1, settings->cmd);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_set_mtype(bool *logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings) {
    if(*logged && valid_mtype_and_not_already_in(settings, msg_received + 1)) {
        parse_ok_response(msg_response, msg_response_len);
        *msg_response_len += strlen(settings->media_types) + 1;
        strcpy(msg_response + 1, settings->media_types);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_rm_mtype(bool *logged, char msg_received[BUFFER_MAX], int msg_received_len, char msg_response[BUFFER_MAX], int * msg_response_len, settings_t settings) {
    if(*logged && rm_mtype(settings, msg_received + 1)) {
        parse_ok_response(msg_response, msg_response_len);
        *msg_response_len += strlen(settings->media_types) + 1;
        strcpy(msg_response + 1, settings->media_types);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_enable_mtype_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings) {
    if(*logged && (!settings->cmd_or_mtype_transformations || !settings->transformations)) {
        settings->cmd_or_mtype_transformations = true;
        settings->transformations = true;
        parse_ok_response(msg_response, msg_response_len);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_enable_cmd_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings) {
    if(*logged && ( settings->cmd_or_mtype_transformations || !settings->transformations)) {
        settings->cmd_or_mtype_transformations = false;
        settings->transformations = true;
        parse_ok_response(msg_response, msg_response_len);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

void parse_disable_transformations(bool * logged, char msg_response[BUFFER_MAX], int *msg_response_len, settings_t settings) {
    if(*logged && settings->cmd_or_mtype_transformations) {
        settings->cmd_or_mtype_transformations = false;
        parse_ok_response(msg_response, msg_response_len);
    } else {
        parse_err_response(msg_response, msg_response_len);
    }
}

bool valid_mtype_and_not_already_in(settings_t settings, char * mtype) {
    if(strstr(settings->media_types, mtype) == NULL && valid_media_type(mtype)) {
        if(settings->mtypes > 0) {
            strcat(settings->media_types, ",");
            strcat(settings->media_types, mtype);
        } else {
            strcpy(settings->media_types, mtype);
        }
        settings->mtypes++;
        return true;
    }
    return false;
}

bool rm_mtype(settings_t settings, char * mtype) {
    char * ptr;
    if((ptr = strstr(settings->media_types, mtype)) != NULL) {
        if(settings->mtypes > 0) {
            int l_mtype = strlen(mtype);
            int len_ptr = strlen(ptr);
            memcpy(ptr - 1, ptr + l_mtype, len_ptr - l_mtype);
        } else {
            settings->media_types[0] = 0;
        }
        settings->media_types--;
        return true;
    }
    return false;
}

void parse_int_to_msg(int number, char msg_response[BUFFER_MAX], int * msg_response_len) {
    sprintf(msg_response + 1, "%d", number);
    *msg_response_len += strlen(msg_response + 1) + 1;
}
