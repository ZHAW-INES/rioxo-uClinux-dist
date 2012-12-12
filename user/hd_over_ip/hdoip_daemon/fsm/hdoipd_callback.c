/*
 * hdoipd_callback.c
 *
 *  Created on: 20.01.2011
 *      Author: alda
 */

#include <string.h>
#include <time.h>

#include "hdoipd.h"
#include "hdoipd_callback.h"

#include "vrb_audio.h"
#include "vrb_video.h"

bool strscmp(char **p, char *s)
{
    int l2 = strlen(s);

    while ((**p) == ' ') (*p)++;

    int n = strncmp(*p, s, l2);

    if (n == 0) {
        *p += l2;
        return 1;
    }

    return 0;
}

/** Callback each required media
 *
 * @param media 0: choose from registry, 1: active, 2: all, ptr*: one specific
 */
int hdoipd_media_callback(t_rtsp_media* media, int (*f)(t_rtsp_media*, void*), void* d)
{
    char *s;
    // ret must be 1 because currently nothing has changed
    int ret = 0;
    if (media == (void*)0) {
        // Choose media based on registry
        s = reg_get("mode-media");
        //report("hdoipd_media_callback() media = 0");
        //report("mode-media: %s", reg_get("mode-media"));
        while (s && *s) {
            if (strscmp(&s, "video"))
              ret = f(&vrb_video, d);
            else if (strscmp(&s, "audio"))
              ret = f(&vrb_audio, d);
            else {
                report("unsupported media: %s", s);
                return 1;
            }

            // check if there was an error
            if (ret != 0)
                break;
        }
    }
    else if (media == (void*)1) {
    	report("hdoipd_media_callback() media = 1");
        // Choose all active media
        if (rtsp_media_active(&vrb_video))
            ret = f(&vrb_video, d);
        if (ret == 0 && rtsp_media_active(&vrb_audio))
            ret = f(&vrb_audio, d);
    }
    else if (media == (void*)2) {
    	report("hdoipd_media_callback() media = 2");
        // Choose all media
        if ((ret = f(&vrb_video, d)) != 0)
            return ret;
        if ((ret = f(&vrb_audio, d)) != 0)
            return ret;
    }
    else {
        // Choose media specified by *media
        ret = f(media, d);
    }

    return ret;
}
