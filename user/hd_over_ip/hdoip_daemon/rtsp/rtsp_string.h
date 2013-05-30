/*
 * rtsp_string.h
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */

#ifndef RTSP_STRING_H_
#define RTSP_STRING_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char*   method;
    char*   uri;
    char*   version;
} t_str_request_line;

typedef struct {
    char*   version;
    char*   code;
    char*   reason;
} t_str_response_line;

typedef struct {
    char*   scheme;
    char*   server;
    char*   port;
    char*   name;
    int32_t control;
} t_str_uri;


int str_test(char* s, const char* fmt);
char* str_next_token(char** line, const char* fmt);
bool str_starts_with(char** line, const char* str);

bool str_split_uri(t_str_uri* uri, char* s);
bool str_split_request_line(t_str_request_line* rl, char* line);
bool str_split_response_line(t_str_response_line* rsp, char* s);
int str_hdcp(char* str);


#endif /* RTSP_STRING_H_ */
