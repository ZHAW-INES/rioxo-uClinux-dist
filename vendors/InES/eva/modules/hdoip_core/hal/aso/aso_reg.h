/*
 * aso_reg.h
 *
 * Created on: 22.10.2010
 *     Author: stth
 */

#ifndef ASO_REG_H_
#define ASO_REG_H_

/** register byte offset
 *
 */
#define ASO_BASE_CONTROL                (0x00000000)
#define ASO_BASE_I2S_CLK_GEN            (0x00000060)

#define ASO_OFF_CONFIG                  (ASO_BASE_CONTROL + 0x00000000)
#define ASO_OFF_STATUS                  (ASO_BASE_CONTROL + 0x00000004)
#define ASO_OFF_AUD_DELAY               (ASO_BASE_CONTROL + 0x00000008)
#define ASO_OFF_CH_CNT_LEFT             (ASO_BASE_CONTROL + 0x00000010)
#define ASO_OFF_CH_CNT_RIGHT            (ASO_BASE_CONTROL + 0x00000014)
#define ASO_OFF_FS_CODE                 (ASO_BASE_CONTROL + 0x00000018)
#define ASO_OFF_CONTAINER               (ASO_BASE_CONTROL + 0x0000001C)
#define ASO_OFF_CLK_CTRL_DIFF_RO        (ASO_BASE_CONTROL + 0x00000020)
#define ASO_OFF_DENOMINATOR             (ASO_BASE_CONTROL + 0x00000024)
#define ASO_OFF_AUD_ID_RO               (ASO_BASE_CONTROL + 0x00000028)
#define ASO_OFF_CLK_DIV_BASE            (ASO_BASE_I2S_CLK_GEN + 0x00000000)
#define ASO_OFF_CLK_DIV_KP              (ASO_BASE_I2S_CLK_GEN + 0x00000004)
#define ASO_OFF_CLK_DIV_KI              (ASO_BASE_I2S_CLK_GEN + 0x00000008)
#define ASO_OFF_MCLK_TO_BCLK_RATIO      (ASO_BASE_I2S_CLK_GEN + 0x0000000C)
#define ASO_OFF_BCLK_TO_LRCLK_RATIO     (ASO_BASE_I2S_CLK_GEN + 0x00000010)
#define ASO_OFF_CLK_DIV_DIFF		    (ASO_BASE_I2S_CLK_GEN + 0x00000014)
#define ASO_OFF_CLK_DIV_ACT			    (ASO_BASE_I2S_CLK_GEN + 0x00000018)
#define ASO_OFF_CLK_CONFIG        	    (ASO_BASE_I2S_CLK_GEN + 0x0000001C)


/** bit indexes 
 *
 */
#define ASO_CFG_RUN                     (1<<0)
#define ASO_CFG_OE                      (1<<1)
#define ASO_STATUS_IDLE                 (1<<0)
#define ASO_STATUS_FRAME_SIZE_ERROR     (1<<1)
#define ASO_STATUS_FIFO_EMPTY           (1<<2)
#define ASO_STATUS_FIFO_FULL            (1<<3)
#define ASO_STATUS_ALL                  0x0F
#define ASO_REORDERING_STATUS_RAM_FULL   (1<<0)
#define ASO_REORDERING_STATUS_ALL       0x01

#endif /* ASO_REG_H_ */
