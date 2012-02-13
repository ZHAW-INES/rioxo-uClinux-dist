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
#include "rscp_include.h"
#include "vrb_video.h"
#include "update.h"
#include "edid.h"

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
    hdoipd_goto_vrb();
    alive_check_start_vrb_alive();
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
    char *buff_ptr = &buff;

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

#define hdoipdreq(x, y) case x: y((void*)cmd); break
#define hdoipdreq_rsp(x, y) case x: y((void*)cmd, rsp); break
void hdoipd_request(uint32_t* cmd, int rsp)
{
    switch (cmd[0]) {
        hdoipdreq(HOIC_CANVAS, hdoipd_cmd_canvas);
        hdoipdreq(HOIC_LOAD, hdoipd_load);
        hdoipdreq(HOIC_CAPTURE, hdoipd_capture);
        hdoipdreq(HOIC_LOOP, hdoipd_loop);
        hdoipdreq(HOIC_OSD_ON, hdoipd_osd_on);
        hdoipdreq(HOIC_OSD_OFF, hdoipd_osd_off);
        hdoipdreq(HOIC_HPD_ON, hdoipd_hpd_on);
        hdoipdreq(HOIC_HPD_OFF, hdoipd_hpd_off);
        hdoipdreq(HOIC_FMT_IN, hdoipd_fmt_in);
        hdoipdreq(HOIC_FMT_OUT, hdoipd_fmt_out);
        hdoipdreq(HOIC_FMT_PROC, hdoipd_fmt_proc);
        hdoipdreq(HOIC_VTB, hdoipd_vtb);
        hdoipdreq(HOIC_VRB_SETUP, hdoipd_cmd_vrb_setup);
        hdoipdreq(HOIC_VRB_PLAY, hdoipd_cmd_play);
        hdoipdreq(HOIC_READY, hdoipd_ready);
        hdoipdreq(HOIC_REBOOT, hdoipd_reboot);
        hdoipdreq(HOIC_REPAIR, hdoipd_repair);
        hdoipdreq(HOIC_STORE_CFG, hdoipd_store_cfg);
        hdoipdreq(HOIC_PARAM_SET, hdoipd_set_param);
        hdoipdreq(HOIC_REMOTE_UPDATE, hdoipd_remote_update);
        hdoipdreq(HOIC_GET_EDID, hdoipd_get_edid);
        hdoipdreq(HOIC_FACTORY_DEFAULT, hdoipd_factory_default);
        hdoipdreq(HOIC_DEBUG, hdoipd_debug);
        hdoipdreq_rsp(HOIC_GETVERSION, hdoipd_get_version);
        hdoipdreq_rsp(HOIC_GETUSB, hdoipd_get_usb);
        hdoipdreq_rsp(HOIC_READ, hdoipd_read);
        hdoipdreq_rsp(HOIC_PARAM_GET, hdoipd_get_param);
        hdoipdreq_rsp(HOIC_GET_HDCP_STATE, hdoipd_get_hdcp_state);
        default: report("command not supported <%08x>", cmd[0]);
    }
}
