#ifndef ESO_REG_H_
#define ESO_REG_H_

/** register byte offset
 * 
 */
#define ESO_OFF_CFG                 (  0)
#define ESO_OFF_STATUS              (  4)
#define ESO_OFF_CPU_START           ( 16)
#define ESO_OFF_CPU_STOP            ( 20)
#define ESO_OFF_CPU_WRITE           ( 24)
#define ESO_OFF_CPU_READ            ( 28)
#define ESO_OFF_VID_START           ( 32)
#define ESO_OFF_VID_STOP            ( 36)
#define ESO_OFF_VID_WRITE           ( 40)
#define ESO_OFF_VID_READ            ( 44)
#define ESO_OFF_AUD_START           ( 48)
#define ESO_OFF_AUD_STOP            ( 52)
#define ESO_OFF_AUD_WRITE           ( 56)
#define ESO_OFF_AUD_READ            ( 60)
#define ESO_OFF_CPU_PRIORITY        ( 64)
#define ESO_OFF_VID_PRIORITY        ( 68)
#define ESO_OFF_AUD_PRIORITY        ( 72)
#define ESO_OFF_CFG_PRIORITY        ( 76)
#define ESO_OFF_CFG_DMA             ( 80)
#define ESO_OFF_CPU_PACKET_CNT      ( 84)
#define ESO_OFF_CPU_DATA_CNT        ( 88)
#define ESO_OFF_CPU_SIZE_ERR_CNT    ( 92)
#define ESO_OFF_AUD_PACKET_CNT      ( 96)
#define ESO_OFF_AUD_DATA_CNT        (100)
#define ESO_OFF_AUD_SIZE_ERR_CNT    (104)
#define ESO_OFF_VID_PACKET_CNT      (108)
#define ESO_OFF_VID_DATA_CNT        (112)
#define ESO_OFF_VID_SIZE_ERR_CNT    (116)

/** register value
 * 
 */
#define ESO_CFG_RUN                 (0x00000001)
#define ESO_CFG_PRIO_EN             (0x00000002)
#define ESO_CFG_STAT_RESET          (0x00000004)
#define ESO_STATUS_IDLE             (0x00000001)
#define ESO_STATUS_PRIO_EN          (0x00000002)
#define ESO_STATUS_ERR              (0x00000004)
#define ESO_STATUS_SIZE_ERR1        (0x00000008)    // frame size = 0
#define ESO_STATUS_SIZE_ERR2        (0x00000010)    // frame size > MTU
#define ESO_DSC_MASK                (0x3FFFFFFF)
#define ESO_DMA_FIFO_TRESH_MASK     (0x3FFC0000)
#define ESO_DMA_FIFO_TRESH_INDEX    (18)
#define ESO_DMA_BURST_SIZE_MASK     (0x0003FC00)
#define ESO_DMA_BURST_SIZE_INDEX    (10)
#define ESO_CFG_PRIO_SHIFT_MASK     (0x70000000)
#define ESO_CFG_PRIO_SHIFT_INDEX    (28)
#define ESO_CFG_PRIO_DIV_MASK       (0x0FFFFFFF)
#define ESO_PRIO_SCALING_MASK       (0xFF000000)
#define ESO_PRIO_SCALING_INDEX      (24)
#define ESO_PRIO_LOAD_MASK          (0x00FFFFFF)


#endif /*ESO_REG_H_*/

