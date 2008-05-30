/*
 *	altuart.c -- Altera UART driver
 *
 *	Based on mcf.c -- Freescale ColdFire UART driver
 *
 *	(C) Copyright 2003-2007, Greg Ungerer <gerg@snapgear.com>
 *	(C) Copyright 2008, Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/altuart.h>

/*
 *	Altera UART reg defs
 */

#define ALTERA_UART_SIZE                  32

#define ALTERA_UART_RXDATA_REG            0
#define ALTERA_UART_TXDATA_REG            4
#define ALTERA_UART_STATUS_REG            8
#define ALTERA_UART_CONTROL_REG           12
#define ALTERA_UART_DIVISOR_REG           16
#define ALTERA_UART_EOP_REG               20

#define ALTERA_UART_STATUS_PE_MSK         (0x1)
#define ALTERA_UART_STATUS_FE_MSK         (0x2)
#define ALTERA_UART_STATUS_BRK_MSK        (0x4)
#define ALTERA_UART_STATUS_ROE_MSK        (0x8)
#define ALTERA_UART_STATUS_TOE_MSK        (0x10)
#define ALTERA_UART_STATUS_TMT_MSK        (0x20)
#define ALTERA_UART_STATUS_TRDY_MSK       (0x40)
#define ALTERA_UART_STATUS_RRDY_MSK       (0x80)
#define ALTERA_UART_STATUS_E_MSK          (0x100)
#define ALTERA_UART_STATUS_DCTS_MSK       (0x400)
#define ALTERA_UART_STATUS_CTS_MSK        (0x800)
#define ALTERA_UART_STATUS_EOP_MSK        (0x1000)

#define ALTERA_UART_CONTROL_PE_MSK        (0x1)
#define ALTERA_UART_CONTROL_FE_MSK        (0x2)
#define ALTERA_UART_CONTROL_BRK_MSK       (0x4)
#define ALTERA_UART_CONTROL_ROE_MSK       (0x8)
#define ALTERA_UART_CONTROL_TOE_MSK       (0x10)
#define ALTERA_UART_CONTROL_TMT_MSK       (0x20)
#define ALTERA_UART_CONTROL_TRDY_MSK      (0x40)
#define ALTERA_UART_CONTROL_RRDY_MSK      (0x80)
#define ALTERA_UART_CONTROL_E_MSK         (0x100)
#define ALTERA_UART_CONTROL_TRBK_MSK      (0x200)
#define ALTERA_UART_CONTROL_DCTS_MSK      (0x400)
#define ALTERA_UART_CONTROL_RTS_MSK       (0x800)
#define ALTERA_UART_CONTROL_EOP_MSK       (0x1000)

#define ALTERA_UART_EOP_MSK               (0xFF)
#define ALTERA_UART_EOP_OFST              (0)

/*
 *	Some boards implement the DTR/DCD lines using GPIO lines, most
 *	don't. Dummy out the access macros for those that don't. Those
 *	that do should define these macros somewhere in there board
 *	specific inlude files.
 */
#if !defined(altera_uart_getppdcd)
#define	altera_uart_getppdcd(p)		(1)
#endif
#if !defined(altera_uart_getppdtr)
#define	altera_uart_getppdtr(p)		(1)
#endif
#if !defined(altera_uart_setppdtr)
#define	altera_uart_setppdtr(p, v)	do { } while (0)
#endif

/*
 *	Local per-uart structure.
 */
struct altera_uart {
	struct uart_port port;
	unsigned int sigs;	/* Local copy of line sigs */
	unsigned short imr;	/* Local IMR mirror */
};

static unsigned int altera_uart_tx_empty(struct uart_port *port)
{
	return (readw(port->membase + ALTERA_UART_STATUS_REG) &
		ALTERA_UART_STATUS_TMT_MSK) ? TIOCSER_TEMT : 0;
}

static unsigned int altera_uart_get_mctrl(struct uart_port *port)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;
	unsigned int sigs;

	spin_lock_irqsave(&port->lock, flags);
	sigs =
	    (readw(port->membase + ALTERA_UART_STATUS_REG) &
	     ALTERA_UART_STATUS_CTS_MSK) ? TIOCM_CTS : 0;
	sigs |= (pp->sigs & TIOCM_RTS);
	sigs |= (altera_uart_getppdcd(port->line) ? TIOCM_CD : 0);
	sigs |= (altera_uart_getppdtr(port->line) ? TIOCM_DTR : 0);
	spin_unlock_irqrestore(&port->lock, flags);
	return sigs;
}

static void altera_uart_set_mctrl(struct uart_port *port, unsigned int sigs)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	pp->sigs = sigs;
	altera_uart_setppdtr(port->line, (sigs & TIOCM_DTR));
	if (sigs & TIOCM_RTS)
		pp->imr |= ALTERA_UART_CONTROL_RTS_MSK;
	else
		pp->imr &= ~ALTERA_UART_CONTROL_RTS_MSK;
	writew(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_start_tx(struct uart_port *port)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	pp->imr |= ALTERA_UART_CONTROL_TRDY_MSK;
	writew(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_stop_tx(struct uart_port *port)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	pp->imr &= ~ALTERA_UART_CONTROL_TRDY_MSK;
	writeb(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_stop_rx(struct uart_port *port)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	pp->imr &= ~ALTERA_UART_CONTROL_RRDY_MSK;
	writew(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_break_ctl(struct uart_port *port, int break_state)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);
	if (break_state == -1)
		pp->imr |= ALTERA_UART_CONTROL_TRBK_MSK;
	else
		pp->imr &= ~ALTERA_UART_CONTROL_TRBK_MSK;
	writew(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_enable_ms(struct uart_port *port)
{
}

static int altera_uart_startup(struct uart_port *port)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);

	/* Enable RX interrupts now */
	pp->imr = ALTERA_UART_CONTROL_RRDY_MSK;
	writew(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);

	spin_unlock_irqrestore(&port->lock, flags);

	return 0;
}

static void altera_uart_shutdown(struct uart_port *port)
{
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);

	/* Disable all interrupts now */
	pp->imr = 0;
	writew(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);

	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_set_termios(struct uart_port *port,
				    struct ktermios *termios,
				    struct ktermios *old)
{
	unsigned long flags;
	unsigned int baud, baudclk;

	baud = uart_get_baud_rate(port, termios, old, 0, 4000000);
	baudclk = port->uartclk / baud;

	spin_lock_irqsave(&port->lock, flags);
	writew(baudclk, port->membase + ALTERA_UART_DIVISOR_REG);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void altera_uart_rx_chars(struct altera_uart *pp)
{
	struct uart_port *port = &pp->port;
	unsigned char ch, flag;
	unsigned short status;

	while ((status = readw(port->membase + ALTERA_UART_STATUS_REG)) &
	       ALTERA_UART_STATUS_RRDY_MSK) {
		ch = readb(port->membase + ALTERA_UART_RXDATA_REG);
		flag = TTY_NORMAL;
		port->icount.rx++;

		if (status & ALTERA_UART_STATUS_E_MSK) {
			writew(status, port->membase + ALTERA_UART_STATUS_REG);

			if (status & ALTERA_UART_STATUS_BRK_MSK) {
				port->icount.brk++;
				if (uart_handle_break(port))
					continue;
			} else if (status & ALTERA_UART_STATUS_PE_MSK) {
				port->icount.parity++;
			} else if (status & ALTERA_UART_STATUS_ROE_MSK) {
				port->icount.overrun++;
			} else if (status & ALTERA_UART_STATUS_FE_MSK) {
				port->icount.frame++;
			}

			status &= port->read_status_mask;

			if (status & ALTERA_UART_STATUS_BRK_MSK)
				flag = TTY_BREAK;
			else if (status & ALTERA_UART_STATUS_PE_MSK)
				flag = TTY_PARITY;
			else if (status & ALTERA_UART_STATUS_FE_MSK)
				flag = TTY_FRAME;
		}

		if (uart_handle_sysrq_char(port, ch))
			continue;
		uart_insert_char(port, status, ALTERA_UART_STATUS_ROE_MSK, ch,
				 flag);
	}

	tty_flip_buffer_push(port->info->tty);
}

static void altera_uart_tx_chars(struct altera_uart *pp)
{
	struct uart_port *port = &pp->port;
	struct circ_buf *xmit = &port->info->xmit;

	if (port->x_char) {
		/* Send special char - probably flow control */
		writeb(port->x_char, port->membase + ALTERA_UART_TXDATA_REG);
		port->x_char = 0;
		port->icount.tx++;
		return;
	}

	while (readw(port->membase + ALTERA_UART_STATUS_REG) &
	       ALTERA_UART_STATUS_TRDY_MSK) {
		if (xmit->head == xmit->tail)
			break;
		writeb(xmit->buf[xmit->tail],
		       port->membase + ALTERA_UART_TXDATA_REG);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (xmit->head == xmit->tail) {
		pp->imr &= ~ALTERA_UART_CONTROL_TRDY_MSK;
		writeb(pp->imr, port->membase + ALTERA_UART_CONTROL_REG);
	}
}

static irqreturn_t altera_uart_interrupt(int irq, void *data)
{
	struct uart_port *port = data;
	struct altera_uart *pp = container_of(port, struct altera_uart, port);
	unsigned int isr;

	isr = readb(port->membase + ALTERA_UART_STATUS_REG) & pp->imr;
	if (isr & ALTERA_UART_STATUS_RRDY_MSK)
		altera_uart_rx_chars(pp);
	if (isr & ALTERA_UART_STATUS_TRDY_MSK)
		altera_uart_tx_chars(pp);
	return IRQ_RETVAL(isr);
}

static void altera_uart_config_port(struct uart_port *port, int flags)
{
	port->type = PORT_ALTERA_UART;

	/* Clear mask, so no surprise interrupts. */
	writeb(0, port->membase + ALTERA_UART_CONTROL_REG);

	if (request_irq
	    (port->irq, altera_uart_interrupt, IRQF_DISABLED | IRQF_SHARED,
	     "UART", port))
		printk(KERN_ERR "ALTERA_UART: unable to attach Altera UART %d "
		       "interrupt vector=%d\n", port->line, port->irq);
}

static const char *altera_uart_type(struct uart_port *port)
{
	return (port->type == PORT_ALTERA_UART) ? "Altera UART" : NULL;
}

static int altera_uart_request_port(struct uart_port *port)
{
	/* UARTs always present */
	return 0;
}

static void altera_uart_release_port(struct uart_port *port)
{
	/* Nothing to release... */
}

static int altera_uart_verify_port(struct uart_port *port,
				   struct serial_struct *ser)
{
	if ((ser->type != PORT_UNKNOWN) && (ser->type != PORT_ALTERA_UART))
		return -EINVAL;
	return 0;
}

/*
 *	Define the basic serial functions we support.
 */
static struct uart_ops altera_uart_ops = {
	.tx_empty = altera_uart_tx_empty,
	.get_mctrl = altera_uart_get_mctrl,
	.set_mctrl = altera_uart_set_mctrl,
	.start_tx = altera_uart_start_tx,
	.stop_tx = altera_uart_stop_tx,
	.stop_rx = altera_uart_stop_rx,
	.enable_ms = altera_uart_enable_ms,
	.break_ctl = altera_uart_break_ctl,
	.startup = altera_uart_startup,
	.shutdown = altera_uart_shutdown,
	.set_termios = altera_uart_set_termios,
	.type = altera_uart_type,
	.request_port = altera_uart_request_port,
	.release_port = altera_uart_release_port,
	.config_port = altera_uart_config_port,
	.verify_port = altera_uart_verify_port,
};

static struct altera_uart altera_uart_ports[CONFIG_SERIAL_ALTERA_UART_MAXPORTS];

#if defined(CONFIG_SERIAL_ALTERA_UART_CONSOLE)

int __init early_altera_uart_setup(struct altera_uart_platform_uart *platp)
{
	struct uart_port *port;
	int i;

	for (i = 0;
	     ((i < CONFIG_SERIAL_ALTERA_UART_MAXPORTS) && (platp[i].mapbase));
	     i++) {
		port = &altera_uart_ports[i].port;

		port->line = i;
		port->type = PORT_ALTERA_UART;
		port->mapbase = platp[i].mapbase;
		port->membase = ioremap(port->mapbase, ALTERA_UART_SIZE);
		port->iotype = SERIAL_IO_MEM;
		port->irq = platp[i].irq;
		port->uartclk = platp[i].uartclk;
		port->flags = ASYNC_BOOT_AUTOCONF;
		port->ops = &altera_uart_ops;
	}

	return 0;
}

static void altera_uart_console_putc(struct console *co, const char c)
{
	struct uart_port *port = &(altera_uart_ports + co->index)->port;
	int i;

	for (i = 0; (i < 0x10000); i++) {
		if (readw(port->membase + ALTERA_UART_STATUS_REG) &
		    ALTERA_UART_STATUS_TRDY_MSK)
			break;
	}
	writeb(c, port->membase + ALTERA_UART_TXDATA_REG);
	for (i = 0; (i < 0x10000); i++) {
		if (readw(port->membase + ALTERA_UART_STATUS_REG) &
		    ALTERA_UART_STATUS_TRDY_MSK)
			break;
	}
}

static void altera_uart_console_write(struct console *co, const char *s,
				      unsigned int count)
{
	for (; (count); count--, s++) {
		altera_uart_console_putc(co, *s);
		if (*s == '\n')
			altera_uart_console_putc(co, '\r');
	}
}

static int __init altera_uart_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = CONFIG_SERIAL_ALTERA_UART_BAUDRATE;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if ((co->index >= 0)
	    && (co->index <= CONFIG_SERIAL_ALTERA_UART_MAXPORTS))
		co->index = 0;
	port = &altera_uart_ports[co->index].port;
	if (port->membase == 0)
		return -ENODEV;

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

static struct uart_driver altera_uart_driver;

static struct console altera_uart_console = {
	.name = "ttyS",
	.write = altera_uart_console_write,
	.device = uart_console_device,
	.setup = altera_uart_console_setup,
	.flags = CON_PRINTBUFFER,
	.index = -1,
	.data = &altera_uart_driver,
};

static int __init altera_uart_console_init(void)
{
	register_console(&altera_uart_console);
	return 0;
}

console_initcall(altera_uart_console_init);

#define	ALTERA_UART_CONSOLE	&altera_uart_console

#else

#define	ALTERA_UART_CONSOLE	NULL

#endif /* CONFIG_ALTERA_UART_CONSOLE */

/*
 *	Define the altera_uart UART driver structure.
 */
static struct uart_driver altera_uart_driver = {
	.owner = THIS_MODULE,
	.driver_name = "altera_uart",
	.dev_name = "ttyS",
	.major = TTY_MAJOR,
	.minor = 64,
	.nr = CONFIG_SERIAL_ALTERA_UART_MAXPORTS,
	.cons = ALTERA_UART_CONSOLE,
};

static int __devinit altera_uart_probe(struct platform_device *pdev)
{
	struct altera_uart_platform_uart *platp = pdev->dev.platform_data;
	struct uart_port *port;
	int i;

	for (i = 0;
	     ((i < CONFIG_SERIAL_ALTERA_UART_MAXPORTS) && (platp[i].mapbase));
	     i++) {
		port = &altera_uart_ports[i].port;

		port->line = i;
		port->type = PORT_ALTERA_UART;
		port->mapbase = platp[i].mapbase;
		port->membase = ioremap(port->mapbase, ALTERA_UART_SIZE);
		port->iotype = SERIAL_IO_MEM;
		port->irq = platp[i].irq;
		port->uartclk = platp[i].uartclk;
		port->ops = &altera_uart_ops;
		port->flags = ASYNC_BOOT_AUTOCONF;

		uart_add_one_port(&altera_uart_driver, port);
	}

	return 0;
}

static int altera_uart_remove(struct platform_device *pdev)
{
	struct uart_port *port;
	int i;

	for (i = 0; (i < CONFIG_SERIAL_ALTERA_UART_MAXPORTS); i++) {
		port = &altera_uart_ports[i].port;
		if (port)
			uart_remove_one_port(&altera_uart_driver, port);
	}

	return 0;
}

static struct platform_driver altera_uart_platform_driver = {
	.probe = altera_uart_probe,
	.remove = __devexit_p(altera_uart_remove),
	.driver = {
		   .name = "altera_uart",
		   .owner = THIS_MODULE,
		   },
};

static int __init altera_uart_init(void)
{
	int rc;

	rc = uart_register_driver(&altera_uart_driver);
	if (rc)
		return rc;
	rc = platform_driver_register(&altera_uart_platform_driver);
	if (rc)
		return rc;
	return 0;
}

static void __exit altera_uart_exit(void)
{
	platform_driver_unregister(&altera_uart_platform_driver);
	uart_unregister_driver(&altera_uart_driver);
}

module_init(altera_uart_init);
module_exit(altera_uart_exit);

MODULE_LICENSE("GPL");
