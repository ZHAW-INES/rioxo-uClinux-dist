/*
 * hdoipd_cmd.c
 *
 *  Created on: 13.12.2010
 *      Author: alda
 */

#include <stdio.h>
#include <stdlib.h>

#include "hdoipd.h"
#include "hoi_drv_user.h"
#include "hoi_image.h"
#include "hoi_res.h"
#include "hoi_cfg.h"
#include "hdoipd_msg.h"
#include "hdoipd_osd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_task.h"
#include "rtsp_client.h"
#include "rtsp_error.h"
#include "rtsp_parse_header.h"
#include "update.h"
#include "edid.h"
#include "vrb_audio_emb.h"
#include "vrb_audio_board.h"
#include "vrb_video.h"

void hdoipd_cmd_canvas(t_hoic_canvas* cmd)
{
    hdoipd_canvas(cmd->width, cmd->height, cmd->fps);
}

void hdoipd_load(t_hoic_load* cmd)
{
    FILE* f;

    if (hdoipd_goto_ready()) {

        if ((f = fopen(cmd->filename, "r")) != NULL) {
            hdoipd.canvas = hoi_image_load(f);
            fclose(f);
        } else {
            report("open file <%s> failed\n", cmd->filename);
        }
        if (hdoipd.canvas) {
            hdoipd.rsc_state |= RSC_VIDEO_OUT;
            hdoipd_set_state(HOID_SHOW_CANVAS);
        } else {
            hdoipd_set_state(HOID_READY);
        }

    }
}

void hdoipd_capture(t_hoic_capture* cmd)
{
    FILE* f;

    if (hdoipd_goto_ready()) {

        if ((f = fopen(cmd->filename, "w")) != NULL) {
            if (cmd->compress) {
                hoi_image_capture_jpeg2000(f, cmd->size);
            } else {
                hoi_image_capture(f);
            }
            fclose(f);
        } else {
            report("open file <%s> failed\n", cmd->filename);
        }

        hdoipd_set_state(HOID_READY);

    }
}

void hdoipd_fmt_in(t_hoic_param* cmd)
{
    hoi_drv_ifmt(cmd->value);
}

void hdoipd_fmt_out(t_hoic_param* cmd)
{
    hoi_drv_ofmt(cmd->value);
}

void hdoipd_fmt_proc(t_hoic_param* cmd)
{
    hoi_drv_pfmt(cmd->value);
}

void hdoipd_loop(t_hoic_cmd UNUSED *cmd)
{
    if (hdoipd_goto_ready()) {
        hoi_drv_loop();
        hdoipd_set_state(HOID_LOOP);
    }
}

void hdoipd_osd_on(t_hoic_cmd UNUSED *cmd)
{
    hoi_drv_osdon();
    hdoipd.rsc_state |= RSC_OSD;
}

void hdoipd_osd_off(t_hoic_cmd UNUSED *cmd)
{
    hoi_drv_osdoff();
    hdoipd.rsc_state &= ~RSC_OSD;
}

void hdoipd_hpd_on(t_hoic_cmd UNUSED *cmd)
{
    hoi_drv_hpdon();
}

void hdoipd_hpd_off(t_hoic_cmd UNUSED *cmd)
{
    hoi_drv_hpdoff();
}

void hdoipd_vtb(t_hoic_cmd UNUSED *cmd)
{
    hdoipd_goto_vtb();
}

void hdoipd_cmd_vrb_setup(t_hoic_load* cmd)
{
    reg_set("remote-uri", cmd->filename);
    hdoipd_goto_vrb();
}

void hdoipd_cmd_play(t_hoic_cmd UNUSED *cmd)
{
    if (!hdoipd_goto_vrb())
		hdoipd_set_state(HOID_VRB);
        hdoipd_start_vrb(true);
}

void hdoipd_cmd_pause(t_hoic_cmd UNUSED *cmd)
{
    t_rtsp_media* media = &vrb_audio_board;           // audio only for debug purposes
    t_rtsp_client *client = media->creator;
    u_rtsp_header buf;
    int ret;

    if (!client)                        { report(ERROR "no client");      return; }
    if (media->state != RTSP_STATE_PLAYING)   { report(ERROR "wrong state");    return; }

    rtsp_client_pause(client, NULL);

    // response
    rtsp_default_response_setup((void*)&buf);
    ret = rtsp_parse_response(&client->con, tab_response_pause, (void*)&buf, 0, CFG_RSP_TIMEOUT);
    if (ret == RTSP_SUCCESS) {
        rmsr_pause(media, 0);
    }
}

void hdoipd_cmd_pause_play(t_hoic_cmd UNUSED *cmd)
{
    t_rtsp_media* media = &vrb_audio_board;           // audio only for debug purposes
    t_rtsp_client *client = media->creator;
    t_rtsp_rtp_format fmt;
    char *s;

    if (!client)                        { report(ERROR "no client");      return; }
    if (media->state != RTSP_STATE_READY)     { report(ERROR "wrong state");    return; }

    s = reg_get("compress");
    if (strcmp(s, "jp2k") == 0) {
        fmt.compress = FORMAT_JPEG2000;
    } else {
        fmt.compress = FORMAT_PLAIN;
    }
    fmt.rtptime = 0;
    fmt.value = reg_get_int("advcnt-min");

    rtsp_client_play(client, &fmt, NULL);
}

void hdoipd_ready(t_hoic_cmd UNUSED *cmd)
{
    if (hdoipd_goto_ready()) {
        report(INFO "hdoip_ready() done");
    }
}

void hdoipd_reboot(t_hoic_cmd UNUSED *cmd)
{
    sleep(2);
    if (altremote_trigger_reconfig()) {
        report(" ? reboot failed");
    }
    report("should have rebooted by now.");
}

void hdoipd_repair(t_hoic_cmd UNUSED *cmd)
{
    hoi_drv_repair();
}

void hdoipd_store_cfg(t_hoic_cmd UNUSED *cmd)
{
    hoi_cfg_write(CFG_FILE);
}

void hdoipd_set_param(t_hoic_kvparam* cmd)
{
    set_call(cmd->str, &cmd->str[cmd->offset]);
}

void hdoipd_get_param(t_hoic_kvparam* cmd, int rsp)
{
    char* s;
    get_call(cmd->str, &s);
    if (!s) s = "";
    hoic_ret_param(rsp, s);
}

void hdoipd_read(t_hoic_param* cmd, int rsp)
{
    uint32_t *p = (void*)(cmd->value|0x80000000);

    hoic_ret_read(rsp, *p);
}

void hdoipd_remote_update(t_hoic_kvparam *cmd)
{
    if(update_flash(cmd->str) != 0) {
        report("update_flash() failed\n");
    }
}

void hdoipd_free_buffer(t_hoic_cmd UNUSED *cmd)
{
    // stop streaming, so that video and audio buffer can be freed
    hdoipd_goto_ready();

    // TODO: wait until ethernet input is stopped
    //while ((hdoipd_rsc(RSC_EABI) || hdoipd_rsc(RSC_EAEI) || hdoipd_rsc(RSC_EVI)));

    // free buffer
    if (hdoipd.img_buff) {
        free(hdoipd.img_buff);
        hdoipd.img_buff = 0;
    }

    if (hdoipd.vid_tx_buff) {
        free(hdoipd.vid_tx_buff);
        hdoipd.vid_tx_buff = 0;
    }

    if (hdoipd.aud_tx_buff) {
        free(hdoipd.aud_tx_buff);
        hdoipd.aud_tx_buff = 0;
    }

    // we need free memory!!!
    system("killall snmpd");
    system("killall dhcpc");

    report("free video and audio buffer\n");
}

void hdoipd_get_edid(t_hoic_kvparam *cmd)
{
    t_edid edid;

    if(hdoipd.rsc_state & RSC_VIDEO_SINK) {
        hoi_drv_rdedid(&edid);
        edid_write_file(&edid, cmd->str);
        edid_report(&edid);
    } else {
        report("no sink connected");
    }
}

void hdoipd_get_version(t_hoic_getversion* cmd, int rsp)
{
    hoi_drv_getversion(cmd);
    write(rsp, cmd, sizeof(t_hoic_getversion));
}

void hdoipd_get_usb(t_hoic_getusb* cmd, int rsp)
{
    hoi_drv_getusb(cmd);
    write(rsp, cmd, sizeof(t_hoic_getusb));
}

void hdoipd_get_hdcp_state(t_hoic_gethdcpstate* cmd, int rsp)
{
    char *buff;
    char **buff_ptr = &buff;

    task_get_hdcp_status(buff_ptr);
    if (strlen(buff) < 100) {
        strcpy(cmd->hdcp_state, buff);
    }
    write(rsp, cmd, sizeof(t_hoic_gethdcpstate));
}

void hdoipd_factory_default(t_hoic_cmd UNUSED *cmd)
{
    char mac[18];
    char serial[15];
    memcpy(mac, reg_get("system-mac"), sizeof(mac));        	/* MAC backup */
    memcpy(serial, reg_get("serial-number"), sizeof(serial));	/* Serial number backup */
    hdoipd_set_default();
    reg_set("system-mac", mac);                           		/* MAC restore */
    reg_set("serial-number", serial);                          	/* Serial number restore */
}

void hdoipd_debug(t_hoic_cmd UNUSED *cmd)
{
    hoi_drv_debug();
}

void hdoipd_reload_edid(t_hoic_cmd UNUSED *cmd)
{
    int edid_length = 256;
    uint8_t edid_table[edid_length];

    if (reg_test("edid-mode", "default") || multicast_get_enabled()) {
        if (!edid_read_file(&edid_table, "/mnt/config/edid.bin")) {
            edid_write_function((t_edid *)edid_table, multicast_get_enabled() ? "unicast edid changed" : "unicast edid changed");
        }
    }
}

void hdoipd_read_ram(t_hoic_kvparam* cmd, int UNUSED rsp)
{
	hoi_drv_read_ram(atoi(cmd->str));
}

#define hdoipdreq(x, y) case x: y((void*)cmd); break
#define hdoipdreq_rsp(x, y) case x: y((void*)cmd, rsp); break
void hdoipd_request(uint32_t* cmd, int rsp)
{
    switch (cmd[0]) {
        hdoipdreq(HOIC_CANVAS               , hdoipd_cmd_canvas);
        hdoipdreq(HOIC_LOAD                 , hdoipd_load);
        hdoipdreq(HOIC_CAPTURE              , hdoipd_capture);
        hdoipdreq(HOIC_LOOP                 , hdoipd_loop);
        hdoipdreq(HOIC_OSD_ON               , hdoipd_osd_on);
        hdoipdreq(HOIC_OSD_OFF              , hdoipd_osd_off);
        hdoipdreq(HOIC_HPD_ON               , hdoipd_hpd_on);
        hdoipdreq(HOIC_HPD_OFF              , hdoipd_hpd_off);
        hdoipdreq(HOIC_FMT_IN               , hdoipd_fmt_in);
        hdoipdreq(HOIC_FMT_OUT              , hdoipd_fmt_out);
        hdoipdreq(HOIC_FMT_PROC             , hdoipd_fmt_proc);
        hdoipdreq(HOIC_VTB                  , hdoipd_vtb);
        hdoipdreq(HOIC_VRB_SETUP            , hdoipd_cmd_vrb_setup);
        hdoipdreq(HOIC_VRB_PLAY             , hdoipd_cmd_play);
        hdoipdreq(HOIC_VRB_PAUSE            , hdoipd_cmd_pause);
        hdoipdreq(HOIC_VRB_PAUSE_PLAY       , hdoipd_cmd_pause_play);
        hdoipdreq(HOIC_READY                , hdoipd_ready);
        hdoipdreq(HOIC_REBOOT               , hdoipd_reboot);
        hdoipdreq(HOIC_REPAIR               , hdoipd_repair);
        hdoipdreq(HOIC_STORE_CFG            , hdoipd_store_cfg);
        hdoipdreq(HOIC_PARAM_SET            , hdoipd_set_param);
        hdoipdreq(HOIC_REMOTE_UPDATE        , hdoipd_remote_update);
        hdoipdreq(HOIC_FREE_BUFFER          , hdoipd_free_buffer);
        hdoipdreq(HOIC_GET_EDID             , hdoipd_get_edid);
        hdoipdreq(HOIC_FACTORY_DEFAULT      , hdoipd_factory_default);
        hdoipdreq(HOIC_DEBUG                , hdoipd_debug);
        hdoipdreq(HOIC_RELOAD_EDID          , hdoipd_reload_edid);
        hdoipdreq_rsp(HOIC_READ_RAM         , hdoipd_read_ram);
        hdoipdreq_rsp(HOIC_GETVERSION       , hdoipd_get_version);
        hdoipdreq_rsp(HOIC_GETUSB           , hdoipd_get_usb);
        hdoipdreq_rsp(HOIC_READ             , hdoipd_read);
        hdoipdreq_rsp(HOIC_PARAM_GET        , hdoipd_get_param);
        hdoipdreq_rsp(HOIC_GET_HDCP_STATE   , hdoipd_get_hdcp_state);
        default: report("command not supported <%08x>", cmd[0]);
    }
}
