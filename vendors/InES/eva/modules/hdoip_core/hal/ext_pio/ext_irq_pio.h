
#ifndef EXT_IRQ_PIO_H
#define EXT_IRQ_PIO_H

#define EXT_IRQ_J2K_CODEC_0		(0x001)
#define EXT_IRQ_J2K_CODEC_1		(0x002)
#define EXT_IRQ_J2K_CODEC_2		(0x004)
#define EXT_IRQ_J2K_CODEC_3		(0x008)
#define EXT_IRQ_HDMI_RX_INT1		(0x010)
#define EXT_IRQ_HDMI_RX_INT2		(0x020)
#define EXT_IRQ_HDMI_TX_INT1		(0x040)
#define EXT_IRQ_ETHI			(0x080)
#define EXT_IRQ_USB_INT1		(0x100)
#define EXT_IRQ_USB_INT2		(0x200)

#define EXT_IRQ_POLARITY        (EXT_IRQ_HDMI_RX_INT1|EXT_IRQ_HDMI_RX_INT2)

/** Macros
 */
#define ext_irq_read32(p) HOI_RD32((p), 0)

#endif
