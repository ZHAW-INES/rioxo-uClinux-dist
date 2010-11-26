/*
 * hdoipd.h
 *
 *  Created on: 03.11.2010
 *      Author: alda
 */

#ifndef HDOIPD_H_
#define HDOIPD_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

enum {
    HOID_IDLE,
    HOID_READY,
    HOID_SHOW_CANVAS,
    HOID_LOOP,
    HOID_VRB,
    HOID_VTB
};

typedef struct {
    int                 drv;
    int                 state;
    pthread_mutex_t     mutex;

    void*               canvas;
    uint32_t            drivers;
} t_hdoipd;


#endif /* HDOIPD_H_ */
