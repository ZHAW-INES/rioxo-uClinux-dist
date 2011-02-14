/*
 * hdoipd_default.c
 *
 *  Created on: 07.12.2010
 *      Author: alda
 */
#include "hdoipd.h"


void hdoipd_set_default()
{
    reg_set("system-ifname", "eth0");
    reg_set("system-name", "rioxo_0");
    reg_set("system-ip", "192.168.1.200");
    reg_set("system-subnet", "255.255.255.0");
    reg_set("system-gateway", "192.168.1.1");
    reg_set("system-mac", "00:15:12:00:00:42");
    reg_set("system-cmd", "");

    reg_set("mode-start", "none");
    reg_set("mode-media", "video");
    reg_set("mode-sync", "streamsync");
    reg_set("sync-target", "video");
    reg_set("remote-uri", "rscp://192.168.1.201");
    reg_set("hello-uri", "rscp://192.168.1.201");
    reg_set("compress", "jp2k");
    reg_set("bandwidth", "9830400");
    reg_set("advcnt-min", "0");
    reg_set("network-delay", "20");
    reg_set("video-port", "3400");
    reg_set("audio-port", "3402");
    reg_set("rscp-server-port", "554");

    reg_set("web-lang", "EN");
    reg_set("web-auth-en", "1");
    reg_set("web-user", "admin");
    reg_set("web-pass", "admin");
}
