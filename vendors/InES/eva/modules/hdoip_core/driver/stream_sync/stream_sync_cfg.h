#ifndef STREAM_SYNC_CFG_H_
#define STREAM_SYNC_CFG_H_


#define SIZE_MEANS			        (0x000001F4)
#define SIZE_RISES                  (0x000001F4)
#define DEAD_TIME                   (0x000001F3)
#define P_GAIN                      (0x00005000)
#define I_GAIN                      (0x00000500)
#define INC_PPM                     (0x00000064)

#define STATISTIC_REG_OFFSET_VID    (0x00000080)
#define STATISTIC_REG_OFFSET_AUD    (0x00000060)

#define SYNC_SOURCE_AUDIO           (0x00050000)
#define SYNC_SOURCE_VIDEO           (0x00060000)

#endif /* STREAM_SYNC_CFG_H_ */
