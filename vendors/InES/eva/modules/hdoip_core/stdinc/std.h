// types
#include <linux/types.h>
#include <linux/string.h>

// linux driver module
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/timer.h>

// platform specific
#include <asm/io.h>

// NIOS2 sopc specific
#include <asm-nios2/nios2.h>

// project specific
#include "stdhal.h"
#include "err.h"
#include "event.h"
#include "config.h"
#include "debug.h"
#include "stdfnc.h"
#include "queue.h"
#include "ext_irq_pio.h"
#include "ext_reset_pio.h"
#include "stdrbf.h"

