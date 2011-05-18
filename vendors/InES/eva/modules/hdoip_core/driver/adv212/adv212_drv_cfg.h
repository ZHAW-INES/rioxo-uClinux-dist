#ifndef ADV212_DRV_CFG_H_
#define ADV212_DRV_CFG_H_

// restrictions
#define ADV212_MAX_SAMPLES          (65000000)
#define ADV212_MAX_VCLK             (75000000)
#define ADV212_MAX_SIZE             (1048000)
// max datarate interlaced = 80mbit/s (workaround)
#define ADV212_CFG_MAX_DATARATE_I   (174602)

// encoder configuration

#define ADV212_CFG_CODE             (ADV212_COD_FORMAT_J2C)
#define ADV212_CFG_TLEVEL           (ADV212_XFORMLEV_5)
#define ADV212_CFG_CBSIZE_P         (ADV212_CBSIZE_64x64)
#define ADV212_CFG_CBSIZE_I         (ADV212_CBSIZE_128x32)
#define ADV212_CFG_WAVLET           (ADV212_IRREVERSIBLE_9x7)
#define ADV212_CFG_IF               (ADV212_PICFG_VCLK_P | \
                                     ADV212_PICFG_VSYNC_P | \
                                     ADV212_PICFG_HSYNC_P | \
                                     ADV212_PICFG_FIELD_N | \
                                     ADV212_PICFG_HVF)


#endif /*ADV212_DRV_CFG_H_*/

// 64*64
