/*
 * rtsp_client.c
 *
 *  Created on: 22.11.2010
 *      Author: alda
 */

#include "rtsp.h"
#include "rtsp_net.h"
#include "rtsp_string.h"
#include "rtsp_client.h"


int rtsp_client_open(t_rtsp_client* client, t_rtsp_media *media, char* address)
{
    char buf[200];
    int fd, port;
    t_str_uri uri;
    struct hostent* host;
    struct in_addr addr;
    struct sockaddr_in dest_addr;

    printf("rtsp client open: %s\n", address);

    client->media = media;

    memcpy(client->uri, address, strlen(address)+1);
    memcpy(buf, address, strlen(address)+1);

    if (!str_split_uri(&uri, buf)) {
        printf("uri error: scheme://server[:port]/name\n");
        return -1;
    }

    host = gethostbyname(uri.server);

    if (!host) {
        printf("gethostbyname failed\n");
        return -1;
    }

    addr.s_addr = *((uint32_t*)host->h_addr_list[0]);

    // default port 554
    if (uri.port) {
        port = htons(atoi(uri.port));
    } else {
        port = htons(554);
    }

    printf("Name: %s - %s:%d\n", host->h_name, inet_ntoa(addr), ntohs(port));

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("server socket error: %s\n", strerror(errno));
        return -1;
    }

    // setup own address
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = port;
    dest_addr.sin_addr.s_addr = addr.s_addr;

    printf("rtsp client connecting... ");
    if (connect(fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr)) == -1) {
        close(fd);
        printf("connection refused\n");
        return -1;
    }

    rtsp_coninit(&client->con, fd, addr.s_addr);

    printf("[DONE]\n");

    return RTSP_SUCCESS;
}

int rtsp_client_close(t_rtsp_client* client)
{
    close(client->con.fd);

    return RTSP_SUCCESS;
}

// returns response code
int rtsp_client_setup(t_rtsp_client* client, t_rtsp_transport* transport)
{
    int n;

    rtsp_request_setup(&client->con, client->uri, transport);

    rtsp_default_response_setup(&client->rsp.setup);

    // response
    n = rtsp_parse_response(&client->con, tab_response_setup, &client->rsp, 0);

    if (n == RTSP_SUCCESS) {
        // TODO:
        memcpy(client->media->sessionid, client->rsp.setup.session, strlen(client->rsp.setup.session));
        rmc_setup(client->media, &client->rsp, &client->con);
    } else if (n == RTSP_RESPONSE_ERROR) {
        client->media->error(client->media, n, &client->con);
    } else {
        printf("internal failure (%d)\n", n);
    }

    return RTSP_SUCCESS;
}

int rtsp_client_play(t_rtsp_client* client, t_rtsp_rtp_format* fmt)
{
    int n;

    rtsp_request_play(&client->con, client->uri, client->media->sessionid, fmt);

    rtsp_default_response_play(&client->rsp.play);

    // response
    n = rtsp_parse_response(&client->con, tab_response_play, &client->rsp, 0);
    if (n == RTSP_SUCCESS) {
        rmc_play(client->media, &client->rsp, &client->con);
    } else if (n == RTSP_RESPONSE_ERROR) {
        client->media->error(client->media, n, &client->con);
    } else {
        printf("internal failure (%d)\n", n);
    }

    return RTSP_SUCCESS;
}

int rtsp_client_teardown(t_rtsp_client* client)
{

    // response
    rtsp_parse_header(&client->con, tab_response_teardown, client->rsp, client->media->sessionid);
    rmc_teardown(client->media, &client->rsp, &client->con);

    rtsp_client_close(client);

    return RTSP_SUCCESS;
}
