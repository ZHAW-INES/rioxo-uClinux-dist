/*
 * rtsp_string.c
 *
 *  Created on: 19.11.2010
 *      Author: alda
 */
#include "rtsp_string.h"

int str_test(char* s, const char* fmt)
{
    int i;
    for (;*fmt;fmt++) {
        switch (*fmt) {
            case '%':
                fmt++;
                switch (*fmt) {
                    case 0: return 0;
                    case '%':
                        if (*s == '%') return 1;
                    break;
                    case '0':
                        if (*s == 0) return 1;
                    break;
                    case ':':
                        fmt++;
                        for (i=0;(*fmt!=';')&&(*fmt)&&(s[i]==*fmt);i++,fmt++);
                        if (*fmt==';') return i;
                        for (;(*fmt!=';')&&(*fmt);fmt++);
                        if (!*fmt) return 0;
                    break;
                }
            break;
            default:
                if (*s == *fmt) return 1;
        }
    }

    return 0;
}

char* str_next_token(char** line, const char* fmt)
{
    int b;
    char *s = *line;
    if (!*line) return 0;
    while (((b = str_test(*line, fmt)) == 0) && (**line)) (*line)++;
    if (b) {

        if (**line) {
            **line = 0;
            *line += b;
        }
        return s;
    }
    *line = s;
    return 0;
}

char* str_split_token(char* t1, const char* fmt)
{
    char* t2 = t1;
    if (!str_next_token(&t2, fmt)) {
        t2 = 0;
    }
    return t2;
}

bool str_starts_with(char** line, const char* str)
{
    char* s = *line;

    for (;(*str==*s) && (*str); s++, str++);

    if (*str == 0) {
        *line = s;
        return true;
    }

    return false;
}

bool str_split_uri(t_str_uri* uri, char* s)
{
    // scheme://server:port/name
    uri->scheme = str_next_token(&s, "%:://;");
    uri->server = str_next_token(&s, "/%0");
    uri->port = str_split_token(uri->server, ":");
    uri->name = str_next_token(&s, "%0");

    return (uri->scheme&&uri->server&&uri->name);
}


bool str_split_request_line(t_str_request_line* rl, char* line)
{
    rl->method = str_next_token(&line, " ");
    rl->uri = str_next_token(&line, " ");
    rl->version = str_next_token(&line, "%0");
    return (rl->method && rl->uri && rl->version);
}

bool str_split_response_line(t_str_response_line* rsp, char* s)
{
    rsp->version = str_next_token(&s, " ");
    rsp->code = str_next_token(&s, " ");
    rsp->reason = str_next_token(&s, "%0");
    return (rsp->version&&rsp->code&&rsp->reason);
}

//function to parse hdcp messages
//returns the string after the "=" as an integer
//TODO: error handling
int str_hdcp(char* str){
  int i, j;
  char status[9]="";
  char tempchar[2];
  char *hdcp_stat;
  hdcp_stat = strchr(str,'=');
  i=hdcp_stat-str;
  for (j=(i+1);j<(int)strlen(str);j++){
    sprintf(tempchar,"%c",str[j]);
    strcat(status,tempchar);
  }
  i = atoi(status);
  return i;
}
