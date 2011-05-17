#ifndef HDCP_REG_H_
#define HDCP_REG_H_

#define HDCP_OFF_CONFIG_REG          (0x00)
#define HDCP_OFF_STATUS_REG          (0x04)
#define HDCP_OFF_START_TIME          (0x08)
#define HDCP_OFF_MASTER_KEY0         (0x0C)
#define HDCP_OFF_MASTER_KEY1         (0x10)
#define HDCP_OFF_MASTER_KEY2         (0x14)
#define HDCP_OFF_MASTER_KEY3         (0x18)

#define HDCP_BIT_CFG_EN              (1<<0)
#define HDCP_BIT_CFG_LOAD            (1<<1)

#define HDCP_BIT_STATUS_EN           (1<<0)
#define HDCP_BIT_STATUS_ZERO         (1<<1)

#endif /* HDCP_REG_H_ */
