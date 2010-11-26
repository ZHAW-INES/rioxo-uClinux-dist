/*
 * err.h
 *
 *  Created on: 24.08.2010
 *      Author: alda
 */

#ifndef ERR_H_
#define ERR_H_

//
#define SUCCESS                                 (0)

// Error Codes
#define ERR_ADV212                              (0xe1000000)
#define ERR_ADV212_SUCCESS                      (0)
#define ERR_ADV212_BOOT_FAIL                    (ERR_ADV212+1)
#define ERR_ADV212_BOOT_TIMEOUT                 (ERR_ADV212+2)
#define ERR_ADV212_NULL_POINTER                 (ERR_ADV212+3)
#define ERR_ADV212_PIXEL_CLOCK_TOOHIGH          (ERR_ADV212+4)
#define ERR_ADV212_PIXEL_COUNT_TOOHIGH          (ERR_ADV212+5)
#define ERR_ADV212_SAMPLE_RATE_TOOHIGH          (ERR_ADV212+6)
#define ERR_ADV212_ADVCNT_OUT_OF_RANGE          (ERR_ADV212+7)

#define ERR_ADV7441A                            (0xe2000000)
#define ERR_ADV7441A_SUCCESS                    (0)

#define ERR_VIO                                 (0xe3000000)
#define ERR_VIO_SUCCESS                 	    (0)
#define ERR_VIO_NO_PLL_CONFIG           	    (ERR_VIO+1)
#define ERR_VIO_VCO_OUT_OF_RANGE        	    (ERR_VIO+2)
#define ERR_VIO_FORMAT_NOT_SUPPORTED            (ERR_VIO+3)
#define ERR_VIO_INVALID_VALUE                   (ERR_VIO+4)

#define ERR_I2C                                 (0xe4000000)
#define ERR_I2C_SUCCESS                         (0)
#define ERR_I2C_NO_DEVICE                       (ERR_I2C+1)
#define ERR_I2C_DATA_NACK                       (ERR_I2C+2)

#define ERR_VSI	                                (0xe5000000)
#define ERR_VSI_SUCCESS                         (0)
#define ERR_VSI_RUNNING                         (ERR_VSI+1)
#define ERR_VSI_PACKET_LENGTH_ERR               (ERR_VSI+2)
#define ERR_VSI_ETH_PARAMS_NOT_SET              (ERR_VSI+3)

#define ERR_VSO                                 (0xe6000000)
#define ERR_VSO_SUCCESS                         (0)
#define ERR_VSO_RUNNING                         (ERR_VSO+1)
#define ERR_VSO_TIMING_NOT_SET                  (ERR_VSO+2)

#define ERR_ASI                                 (0xe7000000)
#define ERR_ASI_SUCCESS                         (0)
#define ERR_ASI_RUNNING                         (ERR_ASI+1)
#define ERR_ASI_ETH_PARAMS_NOT_SET              (ERR_ASI+2)
#define ERR_ASI_AUD_PARAMS_NOT_SET              (ERR_ASI+3)
#define ERR_ASI_PACKET_LENGTH_ERR               (ERR_ASI+4)
#define ERR_ASI_AUD_PARAMS_ERR                  (ERR_ASI+5)

#define ERR_ASO                                 (0xe8000000)
#define ERR_ASO_SUCCESS                         (0)
#define ERR_ASO_RUNNING                         (ERR_ASO+1)
#define ERR_ASO_AUD_PARAMS_NOT_SET              (ERR_ASO+2)
#define ERR_ASO_AUD_DELAY_NOT_SET               (ERR_ASO+3)
#define ERR_ASO_SAMPLE_WIDTH_ERR                (ERR_ASO+4)
#define ERR_ASO_CHANNEL_CNT_ERR                 (ERR_ASO+5)

#define ERR_HOI                                 (0xe9000000)
#define ERR_HOI_CMD_NOT_SUPPORTED               (ERR_HOI+1)

#endif /* ERR_H_ */
