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
#define PROC_NAME       "hdoip_core"
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
} t_hoi_dev;

t_hoi_dev *hdev;

//------------------------------------------------------------------------------
// character device driver i/o

static int hdoip_core_open(struct inode *inop, struct file *filp)
{
    t_hoi_dev *hdev;
    int ret;

    hdev = container_of(inop->i_cdev, t_hoi_dev, cdev);

    ret = mutex_lock_interruptible(&hdev->sem);
    if (ret)
        return ret;

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
    t_hoi_dev *hdev;

    hdev = container_of(inop->i_cdev, t_hoi_dev, cdev);

    mutex_lock(&hdev->sem);

    hdev->is_open = false;

    mutex_unlock(&hdev->sem);
    return 0;
}

static ssize_t hdoip_core_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    t_hoi_dev *hdev = filp->private_data;
	ssize_t ret;

	ret = mutex_lock_interruptible(&hdev->sem);
	if (ret)
		return ret;

	vio_drv_osd_write(&hdev->hoi.vio, buf, count);
	vio_drv_set_osd(&hdev->hoi.vio, true);

	ret = count;

	mutex_unlock(&hdev->sem);
	return ret;
}

static int hdoip_core_ioctl(struct inode *inop, struct file *filp,
                            unsigned int cmd, unsigned long arg)
{
    t_hoi_dev *hdev = filp->private_data;
    t_hoi_msg msg;
    uint32_t tmp[256]; // 1KiB maximum size
    int ret = 0;

    if (cmd == HDOIP_IOCPARAM) {
        // the message is copied to an local buffer,
        // processed and then copied back to userspace
        if (copy_from_user(&msg, (void __user *)arg, sizeof(t_hoi_msg)) != 0) {
            REPORT(WARNING, "copy_from_user: header failed");
            return -EFAULT;
        }

        REPORT(INFO, "[%08x:%d] received", msg.id, msg.size);

        if (msg.size > 1024) {
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

        REPORT(INFO, "[%08x:%d] finished", msg.id, msg.size);
    } else {
        //... TODO
    }

    return ret;
}

static struct file_operations hdoip_core_fops = {
    .open       = hdoip_core_open,
    .release    = hdoip_core_release,
    .ioctl      = hdoip_core_ioctl,
	.write		= hdoip_core_write
};


//------------------------------------------------------------------------------
// /proc read function
int hdoip_proc_read (char *buf, char **start, off_t offset, int count, int *eof, void *data) 
{
    int len = 0;
    struct hdoip_eth_params eth_params;

    // VSI
    vsi_drv_get_eth_params(&hdev->hoi.vsi, &eth_params);
    len += sprintf(buf + len, "vsi config:%08x\n", vsi_get_ctrl(hdev->hoi.vsi.p_vsi,0xFFFFFFFF));
    len += sprintf(buf + len, "vsi status:%08x\n", vsi_get_status(hdev->hoi.vsi.p_vsi,0xFFFFFFFF));
    len += sprintf(buf + len, "vsi dst ip:%08x\n", eth_params.ipv4_dst_ip);
    len += sprintf(buf + len, "vsi dst port:%04x\n", eth_params.udp_dst_port);

    // VSO
    len += sprintf(buf + len, "vso config:%08x\n",vso_get_ctrl(hdev->hoi.vso.p_vso,0xFFFFFFFF));
    len += sprintf(buf + len, "vso status:%08x\n",vso_get_status(hdev->hoi.vso.p_vso,0xFFFFFFFF));

    // ASI
    asi_drv_get_eth_params(&hdev->hoi.asi, &eth_params);

    len += sprintf(buf + len, "asi config:%08x\n", asi_get_ctrl(hdev->hoi.asi.p_asi,0xFFFFFFFF));
    len += sprintf(buf + len, "asi status:%08x\n", asi_get_status(hdev->hoi.asi.p_asi,0xFFFFFFFF));
    len += sprintf(buf + len, "asi dst ip:%08x\n", eth_params.ipv4_dst_ip);
    len += sprintf(buf + len, "asi dst port:%04x\n", eth_params.udp_dst_port);

    // ASO
    len += sprintf(buf + len, "aso config:%08x\n", aso_get_ctrl(hdev->hoi.aso.p_aso,0xFFFFFFFF));
    len += sprintf(buf + len, "aso status:%08x\n", aso_get_status(hdev->hoi.aso.p_aso,0xFFFFFFFF));

    *eof = 1;

    return len;    
}


//------------------------------------------------------------------------------
// driver init

static int hdoip_init(void)
{
    int ret;
    dev_t devno;

	printk(KERN_ALERT "hdoip driver is starting...\n");

	/*
	if (alloc_chrdev_region(&devno, 0, 1, "hdoip_osd")) {
		printk(KERN_ALERT "could not allocate dev_t\n");
		return -1;
	}*/

    devno = MKDEV(HDOIP_CORE_MAJOR, HDOIP_CORE_MINOR);
    ret = register_chrdev_region(devno, HDOIP_CORE_NUM_DEVICES, DRV_NAME);

	REPORT(INFO, "Debug output enabled");

	// char device
	hdev = kzalloc(sizeof(t_hoi_dev), GFP_KERNEL);
	if (!hdev) {
		pr_err( "Failed to allocate device private data\n");
		return -ENOMEM;
	}
	hdev->devno = devno;
	mutex_init(&hdev->sem);
	cdev_init(&hdev->cdev, &hdoip_core_fops);
	hdev->cdev.owner = THIS_MODULE;
	ret = cdev_add(&hdev->cdev, devno, 1);
	if (ret) {
		pr_err("Failed to add character device\n");
		return -1;
	}

	hoi_drv_init(&hdev->hoi);

    // init proc device
    create_proc_read_entry(PROC_NAME, 0, NULL, hdoip_proc_read, NULL);

	return 0;
}

static void hdoip_exit(void)
{
	unregister_chrdev_region(hdev->devno, 1);
    remove_proc_entry(PROC_NAME, NULL);

	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hdoip_init);
module_exit(hdoip_exit);
