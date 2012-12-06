/*
 * box_sys_vrb.c
 *
 *  Created on: 06.12.2012
 *      Author: montesas
 */

#include "box_sys_vrb.h"
#include "box_sys.h"
#include "hdoipd.h"
#include "rtsp_client.h"

#define TICK_SEND_ALIVE                 (hdoipd.eth_alive)

static int box_sys_vrb_event(t_rtsp_media *media, uint32_t event)
{
  t_rtsp_client *client = NULL;

  if (media == NULL || media->creator == NULL) {
    return RTSP_NULL_POINTER;
  }

  client = (t_rtsp_client*)media->creator;

  switch (event)
  {
    case EVENT_TICK:
      if (client->timeout.alive_ping > 0)
        client->timeout.alive_ping--;
      else {
        client->timeout.alive_ping = TICK_SEND_ALIVE;
        return rtsp_client_get_parameter(client);
      }
      break;
  }

  return RTSP_SUCCESS;
}

t_rtsp_media box_sys_vrb = {
    .name           = "",
    .owner          = 0,
    .cookie         = NULL,
    .hello          = box_sys_hello,
    .options        = box_sys_options,
    .get_parameter  = box_sys_get_parameter,
    .event          = box_sys_vrb_event
};
