#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "hoi_drv.h"
#include "hoi_msg.h"

MODULE_LICENSE("abc");

#define DRV_NAME        "hdoip_core"
#define DRV_VERSION     "0.0"

/* Major and minor number for the device file */
#define HDOIP_CORE_MAJOR    123
#define HDOIP_CORE_MINOR    0
/* Number of devices supported */
#define HDOIP_CORE_NUM_DEVICES  1

typedef struct {
	struct cdev cdev;
	dev_t devno;
	bool is_open;
    t_hoi hoi;
	struct mutex sem;
	struct timer_list timer;
} t_hoi_dev;

t_hoi_dev *hdev;

//------------------------------------------------------------------------------
// character device driver i/o

static int hdoip_core_open(struct inode *inop, struct file *filp)
{
    t_hoi_dev *hdev;
    int ret;

    hdev = container_of(inop->i_cdev, t_hoi_dev, cdev);

    if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;

    if (hdev->is_open) {
        ret = -EBUSY;
        goto err_out;
    }

    filp->private_data = hdev;
    hdev->is_open = true;

err_out:
    mutex_unlock(&hdev->sem);
    return ret;
}

static int hdoip_core_release(struct inode *inop, struct file *filp)
{
    int ret;
    t_hoi_dev *hdev;

    hdev = container_of(inop->i_cdev, t_hoi_dev, cdev);

    if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;

    hdev->is_open = false;

    mutex_unlock(&hdev->sem);
    return 0;
}

static ssize_t hdoip_core_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    t_hoi_dev *hdev = filp->private_data;
	ssize_t ret;

	if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;

	vio_drv_osd_write(&hdev->hoi.vio, (uint8_t*)buf, count);

	ret = count;

	mutex_unlock(&hdev->sem);
	return ret;
}

static ssize_t hdoip_core_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    t_hoi_dev *hdev = filp->private_data;
    ssize_t ret;
    uint32_t event;

    if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;

    if (count != 4) {
        return -EFAULT;
    }

    while (queue_empty(hdev->hoi.event)) {
        // no data -> wait
        mutex_unlock(&hdev->sem);
        wait_event_interruptible(hdev->hoi.eq, !queue_empty(hdev->hoi.event));
        if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;
    }

    event = queue_get(hdev->hoi.event);
    copy_to_user(buf, &event, 4);

    mutex_unlock(&hdev->sem);

    return 4;
}

static int hdoip_core_ioctl(struct inode *inop, struct file *filp,
                            unsigned int cmd, unsigned long arg)
{
    t_hoi_dev *hdev = filp->private_data;
    t_hoi_msg msg;
    uint32_t tmp[256]; // 1KiB maximum size
    int ret = 0;
    //uint32_t t;

    if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;

    if (cmd == HDOIP_IOCPARAM) {
        //t = jiffies;

        // the message is copied to an local buffer,
        // processed and then copied back to userspace
        if (copy_from_user(&msg, (void __user *)arg, sizeof(t_hoi_msg)) != 0) {
            REPORT(WARNING, "copy_from_user: header failed");
            return -EFAULT;
        }

        if (msg.size > 1024) {
            REPORT(WARNING, "size too big");
            return -EFAULT;
        }

        if (copy_from_user(tmp, (void __user *)arg, msg.size) != 0) {
            REPORT(WARNING, "copy_from_user: message failed");
            return -EFAULT;
        }

        hoi_drv_message(&hdev->hoi, (t_hoi_msg*)tmp);

        if (copy_to_user((void __user *)arg, tmp, msg.size) != 0) {
            REPORT(WARNING, "copy_to_user failed");
            return -EFAULT;
        }

        //t = (jiffies - t) * 1000 / HZ;
        //REPORT(INFO, "execution time: %d ms", t);
    } else {
        //... TODO
    }

    mutex_unlock(&hdev->sem);
    return ret;
}

static struct file_operations hdoip_core_fops = {
    .open       = hdoip_core_open,
    .release    = hdoip_core_release,
    .ioctl      = hdoip_core_ioctl,
	.write		= hdoip_core_write,
	.read       = hdoip_core_read
};


//------------------------------------------------------------------------------
// driver init

static int hdoip_init(void)
{
    int ret;
    dev_t devno;

	printk(KERN_ALERT "hdoip driver is starting...\n");

    devno = MKDEV(HDOIP_CORE_MAJOR, HDOIP_CORE_MINOR);
    ret = register_chrdev_region(devno, HDOIP_CORE_NUM_DEVICES, DRV_NAME);

	REPORT(INFO, "Debug output enabled");

	// char device
	hdev = kzalloc(sizeof(t_hoi_dev), GFP_KERNEL);
	if (!hdev) {
		pr_err( "Failed to allocate device private data\n");
		return -ENOMEM;
	}
	init_waitqueue_head(&hdev->hoi.eq);
	hdev->devno = devno;
	mutex_init(&hdev->sem);
	cdev_init(&hdev->cdev, &hdoip_core_fops);
	hdev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&hdev->cdev, devno, 1);
	if (ret) {
		pr_err("Failed to add character device\n");
		return -1;
	}

	if ((ret = mutex_lock_interruptible(&hdev->sem))) return ret;

	hoi_drv_init(&hdev->hoi);

	mutex_unlock(&hdev->sem);

	return 0;
}

static void hdoip_exit(void)
{
	unregister_chrdev_region(hdev->devno, 1);

	printk(KERN_ALERT "Goodbye, cruel world\n");
}


module_init(hdoip_init);
module_exit(hdoip_exit);
