/*
 * box_sys_vtb.c
 *
 *  Created on: 06.12.2012
 *      Author: montesas
 */

#include "box_sys_vtb.h"
#include "box_sys.h"
#include "hdoipd.h"
#include "hdoipd_fsm.h"
#include "hdoipd_osd.h"
#include "multicast.h"
#include "rtsp_server.h"

#define TICK_TIMEOUT_MULTICAST          (hdoipd.eth_timeout * 2)
#define TICK_TIMEOUT_UNICAST            (hdoipd.eth_timeout)

static int box_sys_vtb_describe(t_rtsp_media *media, void *_data, t_rtsp_connection *con)
{
    t_rtsp_req_describe *data = _data;
    int ret;

    if (!data)
        return -1;

    ret = rtsp_handle_describe_generic(media, data, con);
    if (ret)
        return ret;

    /* Add media specific content */

    /* TODO: Implement dynamically */
    msgprintf(con, "m=video 0 RTP/AVP 96\r\n");
    msgprintf(con, "a=control:/device/1/video/1/\r\n");
    msgprintf(con, "m=audio 0 RTP/AVP 96\r\n");
    msgprintf(con, "a=control:/device/1/audio/1/\r\n");
    msgprintf(con, "m=usb 0 TCP/USB over IP\r\n");
    msgprintf(con, "a=control:/device/1/usb/1/\r\n");

    rtsp_send(con);

    return 0;
}

int box_sys_vtb_get_parameter(t_rtsp_media *media, void *data, t_rtsp_connection *rsp)
{
  if (media == NULL || media->creator == NULL)
    return RTSP_NULL_POINTER;

  ((t_rtsp_server*)media->creator)->timeout.timeout = 0;
  return box_sys_get_parameter(media, data, rsp);
}

static int box_sys_vtb_event(t_rtsp_media *media, uint32_t event)
{
  t_rtsp_server *server = NULL;
  uint32_t timeout;

  if (media == NULL || media->creator == NULL)
    return RTSP_NULL_POINTER;

  server = (t_rtsp_server*)media->creator;

  switch (event)
  {
    case EVENT_TICK:
    {
      if (multicast_get_enabled()) {
          timeout = TICK_TIMEOUT_MULTICAST;
      } else {
          timeout = TICK_TIMEOUT_UNICAST;
      }

      if (server->timeout.timeout <= timeout)
        server->timeout.timeout++;
      else {
        report(INFO "box_sys_vtb_event: timeout %d", timeout);
        // timeout -> kill connection
        // server cannot kill itself -> add to kill list
        // (will be executed after all events are processed)
        server->timeout.timeout = 0;
        rtsp_listener_add_kill(&hdoipd.listener, server);
        hdoipd_clr_rsc(RSC_VIDEO_OUT | RSC_OSD);
        osd_permanent(true);
        osd_printf("connection lost...\n");
      }
      break;
    }
  }

  return RTSP_SUCCESS;
}

t_rtsp_media box_sys_vtb = {
    .name           = "",
    .owner          = 0,
    .cookie         = NULL,
    .options        = box_sys_options,
    .describe       = box_sys_vtb_describe,
    .get_parameter  = box_sys_vtb_get_parameter,
    .set_parameter  = box_sys_set_parameter,
    .event          = box_sys_vtb_event
};
