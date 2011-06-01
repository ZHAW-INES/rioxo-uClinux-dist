/*
 * hdoipd_default.c
 *
 *  Created on: 07.12.2010
 *      Author: alda
 */
#include "hdoipd.h"
#include "hoi_cfg.h"

#define CFGTAG "config-version"

void hdoipd_set_default()
{
    reg_set("system-ifname", "eth0");
    reg_set("system-name", "rioxo_0");
    reg_set("system-ip", "192.168.1.200");
    reg_set("system-subnet", "255.255.255.0");
    reg_set("system-gateway", "192.168.1.1");
    reg_set("system-mac", "00:15:12:00:00:42");
    reg_set("system-cmd", "");

    reg_set("auto-stream", "true");
    reg_set("mode-start", "none");
    reg_set("mode-media", "video");
    reg_set("mode-sync", "streamsync");
    reg_set("sync-target", "video");
    reg_set("remote-uri", "rscp://192.168.1.201");
    reg_set("hello-uri", "rscp://192.168.1.201");
    reg_set("compress", "jp2k");
    reg_set("bandwidth", "10000000");
    reg_set("advcnt-min", "0");
    reg_set("network-delay", "20");
    reg_set("network-alive", "1");
    reg_set("network-timeout", "3");
    reg_set("video-port", "3400");
    reg_set("audio-port", "3402");
    reg_set("rscp-server-port", "554");

    reg_set("hdcp-force", "false");

    reg_set("web-lang", "EN");
    reg_set("web-auth-en", "false");
    reg_set("web-user", "admin");
    reg_set("web-pass", "admin");

    reg_set("amx-en", "true");
    reg_set("amx-hello-ip", "192.168.1.255");
    reg_set("amx-hello-port", "1000");
    reg_set("amx-hello-msg", "HELLO");
    reg_set("amx-hello-interval", "5");

    reg_set("multicast_en", "false");
    reg_set("multicast_group", "224.0.1.0");

    reg_set("alive-check", "true");
    reg_set("alive-check-interval", "5");
    reg_set("alive-check-port", "2002");
}

static void update_0_0_to_0_1()
{
    char *p;
    p = reg_get("rtsp-server-port");
    if (p) {
        reg_set("rscp-server-port", p);
        reg_del("rtsp-server-port");
    }
    p = reg_get("remote-uri");
    if (p && (strncmp(p, "rtsp", 4) == 0)) {
        memcpy(p, "rscp", 4);
    }
    p = reg_get("hello-uri");
    if (p && (strncmp(p, "rtsp", 4) == 0)) {
        memcpy(p, "rscp", 4);
    }

    reg_set(CFGTAG, "v0.1");

    report(INFO "updated registry from version 0.0 to 0.1");
}

static void update_0_1_to_0_2()
{
    char *p;
    p = reg_get("amx-en");
    if (p) {
        if(strcmp(p, "1") == 0) {
            reg_set("amx-en", "true");
        } else {
            reg_set("amx-en", "false");
        }
    }

    p = reg_get("web-auth-en");
    if (p) {
        if(strcmp(p, "1") == 0) {
            reg_set("web-auth-en", "true");
        } else {
            reg_set("web-auth-en", "false");
        }
    }

    reg_set(CFGTAG, "v0.2");

    report(INFO "updated registry from version 0.1 to 0.2");
}

static void update_0_2_to_0_3()
{
    char *p;
    p = reg_get("multicast_group_video");
    if (p) {
        reg_del("multicast_group_video");
    }

    p = reg_get("multicast_group_audio");
    if (p) {
        reg_del("multicast_group_audio");
    }

    reg_set(CFGTAG, "v0.3");
}

/** upgrade config
 *
 * upgrades from one version to the next until the newest version is
 * reached
 *
 */
void hdoipd_registry_update()
{
    bool update = false;

    if (!reg_get(CFGTAG)) {
        // version 0.0
        update_0_0_to_0_1();
        update = true;
    }

    if (reg_test(CFGTAG, "v0.1")) {
        update_0_1_to_0_2();
        update = true;
    }

    if (reg_test(CFGTAG, "v0.2")) {
        update_0_2_to_0_3();
        update = true;
    }

    // ... further version upgrades
    // if (reg_test(CFGTAG, "v0.1")) -> upgrade to 0.2 etc..

    // when update store the result
    if (update) {
        report(INFO "store updated config");
        hoi_cfg_write(CFG_FILE);
    }

    if (!reg_test(CFGTAG, "v0.3")) {
        report(INFO "unknown config version");
    }
}

