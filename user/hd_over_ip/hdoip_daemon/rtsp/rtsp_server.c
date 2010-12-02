/*
 * rtsp_server.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "map.h"
#include "rtsp_net.h"
#include "rtsp_server.h"
#include "rtsp_string.h"

int rtsp_server_init(t_rtsp_server* handle, int fd, uint32_t addr)
{
    rtsp_coninit(&handle->con, fd, addr);
}


/** process incoming RTSP messages
 *
 * Format is the following:
 *  - Request Line [CRLF]
 *  - n * Header Line [CRLF]
 *  - [CRLF]
 *  - { Optional Body }
 *
 * if a format error is detected the function returns with an error code
 *
 */
int rtsp_server_thread(t_rtsp_server* handle)
{
    int n;
    char *line, *attr;
    char session[20];
    t_map_set* method;
    t_str_request_line rq;
    t_str_uri uri;
    t_rtsp_media *media, *smedia;

    printf("rtsp server started\n");

    // receive request line
    while ((n = rtsp_receive(&handle->con, &line)) >= 0) {

        if (!str_split_request_line(&rq, line)) return RTSP_CLIENT_ERROR;

        // test version
        if (strcmp(rq.version, RTSP_VERSION) != 0) return RTSP_SERVER_ERROR;
        // test uri
        if (!str_split_uri(&uri, rq.uri)) return RTSP_CLIENT_ERROR;
        if (strcmp(uri.scheme, RTSP_SCHEME) != 0) return RTSP_CLIENT_ERROR;

        // find media
        media = map_get_rec(srv_media, uri.name);
        // find method
        method = map_find_set(srv_method, rq.method);

        // TODO: special method when not supported (for response,sync)
        if (!media) {
            rtsp_ommit_header(&handle->con);
            rtsp_response_error(&handle->con, 462, "Destination unreachable");
            return RTSP_RESOURCE_ERROR;
        }
        if (!method) {
            rtsp_ommit_header(&handle->con);
            rtsp_response_error(&handle->con, 405, "Method not allowed");
            return RTSP_CLIENT_ERROR;
        }

        // process fields
        *session = 0;
        n = rtsp_parse_header(&handle->con, method->rec, &handle->req, session);
        if (n != RTSP_SUCCESS) {
            printf(" ? parse header error (%d)\n", n);
            return n;
        }

        // TODO: receive body


        // find appropriate session
        if (*session) {
            if ((smedia = rtsp_media_get(media, session))) {
                media = smedia;
            }
        }

        // process request (function responses for itself)
        n = ((frtspm*)method->fnc)(media, &handle->req, &handle->con);
        if (n != RTSP_SUCCESS) {
            printf(" ? execute method error (%d)\n", n);
            return n;
        }


    }

    return RTSP_SUCCESS;
}
