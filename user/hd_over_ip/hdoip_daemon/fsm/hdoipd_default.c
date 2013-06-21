/*
 * hdoipd_default.c
 *
 *  Created on: 07.12.2010
 *      Author: alda
 */
#include "hdoipd.h"
#include "hoi_cfg.h"

#define CFGTAG      "config-version"
#define CFGVERSION  "v0.6"

void hdoipd_set_default()
{
    reg_set("system-ifname", "eth0");
    reg_set("system-hostname", "device_0");
    reg_set("system-dev-caption", "e.q. blu-ray player");
    reg_set("system-ip", "192.168.1.200");
    reg_set("system-subnet", "255.255.255.0");
    reg_set("system-gateway", "192.168.1.1");
    reg_set("system-mac", "00:15:12:00:00:42");
    reg_set("system-dns1", "");
    reg_set("system-dns2", "");
    reg_set("system-dhcp", "false");
    reg_set("system-cmd", "");
    reg_set("eth-ttl", "30");

    reg_set("auto-stream", "true");
    reg_set("mode-start", "none");
    reg_set("mode-media", "video audio_emb");
    reg_set("audio-source", "linein");
    reg_set("audio-hpgain", "50");
    reg_set("audio-linegain", "50");
    reg_set("audio-mic-boost","0");
    reg_set("mode-sync", "streamsync");
    reg_set("sync-target", "video");
    reg_set("remote-uri", RTSP_SCHEME "://192.168.1.201");
    reg_set("compress", "jp2k");
    reg_set("bandwidth", "10485760");
    reg_set("chroma-bandwidth", "100");
    reg_set("fps_divide", "0");
    reg_set("advcnt-min", "0");
    reg_set("network-delay", "20");
    reg_set("av-delay", "0");
    reg_set("network-alive", "1");
    reg_set("network-timeout", "3");
    reg_set("video-port", "3400");
    reg_set("audio-port", "3406");
    reg_set("audio-port-board", "3412");
    reg_set("rtsp-server-port", "554");
    reg_set("rtsp-timeout", "5");

    reg_set("hdcp-force", "false");

    reg_set("web-lang", "EN");
    reg_set("web-auth-en", "false");
    reg_set("web-user", "admin");
    reg_set("web-pass", "admin");
    reg_set("amx-en", "false");
    reg_set("amx-hello-ip", "192.168.1.255");
    reg_set("amx-hello-port", "1000");
    reg_set("amx-hello-msg", "HELLO");
    reg_set("amx-hello-interval", "5");

    reg_set("multicast_en", "false");
    reg_set("multicast_group", "224.0.1.0");

    reg_set("led_instruction", "0");
    reg_set("osd-time", "5");
    reg_set("serial-number", "00000000000000");

    reg_set("usb-mode", "off");
    reg_set("edid-mode", "receiver");
    reg_set("traffic-shaping", "true");
    reg_set("fec_setting", "1441014410");     

    reg_set(CFGTAG, "origin");
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

    p = reg_get("system-name");
    reg_del("system-name");

    reg_set("system-hostname", p);
    reg_set("system-dev-caption", "e.q. blu-ray player");

    p = reg_get("multicast_group_video");
    if (p) {
        reg_del("multicast_group_video");
    }

    p = reg_get("multicast_group_audio");
    if (p) {
        reg_del("multicast_group_audio");
    }

    reg_set(CFGTAG, "v0.3");

    report(INFO "updated registry from version 0.2 to 0.3");
}

static void update_0_3_to_0_4()
{
    char *p;

    // because fec, one port must be at least 6 bigger than the other port
    reg_set("video-port", "3400");
    reg_set("audio-port", "3406");

    reg_set(CFGTAG, "v0.4");

    report(INFO "updated registry from version 0.3 to 0.4");
}

static void update_0_4_to_0_5()
{
    size_t length = 0;
    char *p, *tmp;

    // update remote-uri
    p = reg_get("remote-uri");
    if (p && strstr(p, "rscp://") == p)
    {
        length = strlen(p);
        tmp = (char*)malloc(length + 1);
        if (tmp)
        {
          length = sprintf(tmp, "%s://%s", RTSP_SCHEME, p + 7);
          tmp[length] = '\0';
          reg_set("remote-uri", tmp);

          free(tmp);
        }
    }

    // update hello-uri
    p = reg_get("hello-uri");
    if (p && strstr(p, "rscp://") == p)
    {
        length = strlen(p);
        tmp = (char*)malloc(length + 1);
        if (tmp)
        {
          length = sprintf(tmp, "%s://%s", RTSP_SCHEME, p + 7);
          tmp[length] = '\0';
          reg_set("hello-uri", tmp);

          free(tmp);
        }
    }

    // update rscp-server-port to rtsp-server-port
    p = reg_get("rscp-server-port");
    if (p)
    {
        reg_set("rtsp-server-port", p);
        reg_del("rscp-server-port");
    }

    reg_set(CFGTAG, "v0.5");
}

static void update_0_5_to_0_6()
{
    if (reg_get("alive-check") != NULL)
        reg_del("alive-check");
    if (reg_get("alive-check-interval") != NULL)
        reg_del("alive-check-interval");
    if (reg_get("alive-check-port") != NULL)
        reg_del("alive-check-port");

    reg_set(CFGTAG, "v0.6");
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

    if (reg_test(CFGTAG, "origin")) {
        reg_set(CFGTAG, CFGVERSION);
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

    if (reg_test(CFGTAG, "v0.3")) {
        update_0_3_to_0_4();
        update = true;
    }

    if (reg_test(CFGTAG, "v0.4")) {
        update_0_4_to_0_5();
        update = true;
    }

    if (reg_test(CFGTAG, "v0.5")) {
        update_0_5_to_0_6();
        update = true;
    }

    // when update store the result
    if (update) {
        report(INFO "store updated config");
        hoi_cfg_write(CFG_FILE);
    }

    if (!reg_test(CFGTAG, CFGVERSION)) {
        report(INFO "unknown config version");
    }
}
