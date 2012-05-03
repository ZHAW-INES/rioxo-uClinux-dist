#ifndef CONFIG_H_
#define CONFIG_H_

#include <std.h>

// A/V reference frequency
#define AVFREQ              (1000000)
#define AVPERIOD            (1) /* in us */
// System frequency
#define SFREQ               (100000000)
#define SPERIOD             (10) /* in ns */
// Video pll base frequency
#define BFREQ               (75000000)
// ADV Frequency
#define MFREQ               (37500000)
// Oscillator tolerance
#define OSC_TOL             (50.0e-6)
// Working tolerance (min. 2*OSC_TOL)
#define SYS_TOL             (200.0e-6)
// maximum frame size
#define MAX_FRAME_LENGTH    (2048)

// Base address
#define BASE_TIMER          (na_acb_timer)
#define BASE_WATCHDOG       (na_acb_watchdog)
#define BASE_HDCP           (na_acb_hdcp)
#define BASE_VRP            (na_acb_vrp)
#define BASE_VIO            (na_acb_video_process)
#define BASE_ADV212	        (na_hdata_bus)
#define BASE_VIDEO_TX       (na_hdmi_tx_i2c)
#define BASE_VIDEO_RX       (na_hdmi_rx_i2c)
#define BASE_I2C_VID_TAG    (na_tag_vid_i2c)
#define BASE_I2C_AUD_TAG    (na_tag_aud_i2c)
#define BASE_VSO	        (na_acb_vid_st_out)
#define BASE_VSI	        (na_acb_vid_st_in)
#define BASE_ASO            (na_acb_aud_st_out)
#define BASE_ASI            (na_acb_aud_st_in)
#define BASE_ETI            (na_acb_eth_in)
#define BASE_ETI_IRQ        (na_acb_eth_in_irq)
#define BASE_ETO            (na_acb_eth_out)
#define BASE_ESC            (na_acb_eth_speed_chg)
#define BASE_EXT_IRQ        (na_ext_irq_pio)
#define BASE_TSE            (na_tse_mac_control_port)
#define BASE_VER            (na_acb_firmware_revision)
#define BASE_SYSID          (na_sysid)
#define BASE_LED            (na_acb_led)
#define BASE_VIDEO_MUX      (na_acb_video_mux)
#define BASE_SPI_TX         (na_spi_tx)
#define BASE_SPI_RX         (na_spi_rx)
#define BASE_SI598          (na_si598_i2c)

// Register area sizes
#define TSE_MAC_SIZE        0x400

#endif /*CONFIG_H_*/
