/* For AllWinner Linux platform.
 *
 * mir3da.c - Linux kernel modules for 3-Axis Accelerometer
 *
 * Copyright (C) 2011-2013 MiraMEMS Sensing Technology Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/average.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input-polldev.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeirq.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sunxi-gpio.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>

#include <linux/kthread.h>
#include <linux/syscalls.h>

#include "mir3da_core.h"
#include "mir3da_cust.h"

#define MIR3DA_DRV_NAME "da380"
#define MIR3DA_INPUT_DEV_NAME MIR3DA_DRV_NAME
#define MIR3DA_MISC_NAME MIR3DA_DRV_NAME

static const unsigned short normal_i2c[] = {0x27,
					    I2C_CLIENT_END}; /* 0x4F  //0x27 */

#define POLL_INTERVAL_MAX 500 /* 800//500 */
#define POLL_INTERVAL 100     /* 500//50 */
/*#define POLL_INTERVAL_MAX 500 */
/*#define POLL_INTERVAL 50 */
#define INPUT_FUZZ 0
#define INPUT_FLAT 0

#define GSENSOR_THRES_TRIG_CNT 1

/*-----------gsensor data use mg ---------------------- */
/* same define for sdvcam gsensor_manager */
#define G_SENSOR_IMPACT_LEVEL_CLOSE 0
#define G_SENSOR_IMPACT_LEVEL_LOW 1
#define G_SENSOR_IMPACT_LEVEL_MIDDLE 2
#define G_SENSOR_IMPACT_LEVEL_HIGH 3

/* normal mode */
#define G_SENSOR_SENSI_H_VAL                                                   \
	1110 /* 600//1.11f   //(1.58f)    for test, normal is 1110 */
#define G_SENSOR_SENSI_M_VAL 1620 /* 1.32f   //(1.88f) */
#define G_SENSOR_SENSI_L_VAL 2010 /* 1.61f   //(2.28f) */

/* Parking Mode */
#define G_SENSOR_SENSI_PARKING_H_VAL 1200 /* 150//0.15f */
#define G_SENSOR_SENSI_PARKING_M_VAL 1600 /* 200//0.2f */
#define G_SENSOR_SENSI_PARKING_L_VAL 1800 /* 250//0.25f */

#define GSENSOR_K 8 /* 7.81(4G)  3.91(2g range)  15.625(8g range) */

static struct input_polled_dev *mir3da_idev;
static MIR_HANDLE mir_handle;
static unsigned int int_latch_mode = 0x83; /* 0011: temporary latched 1s */
static unsigned int slope_th;
static unsigned int delayMs = 50;

static int int2_enable;
static int int2_status;
static int impact_status;
static int impact_happen_level = G_SENSOR_IMPACT_LEVEL_HIGH;

static int g_target = G_SENSOR_SENSI_H_VAL;
static int parking_impact_sensitivity_close; /* 1=close,  0=open */

/* static int mGsensorOccurAxis = -1; */

static short axis_x_value;
static short axis_y_value;
static short axis_z_value;

struct gpio_desc *int_gpio;
int gsensor_irq;

extern int Log_level;

#define MI_DATA(format, ...)                                                   \
	do {								       \
		if (DEBUG_DATA & Log_level) {                                  \
			printk(KERN_ERR MI_TAG format "\n", ##__VA_ARGS__);    \
		}							       \
	} while (0)

#define MI_MSG(format, ...)                                                    \
	do {								       \
		if (DEBUG_MSG & Log_level) {                                   \
			printk(KERN_ERR MI_TAG format "\n", ##__VA_ARGS__);    \
		}							       \
	} while (0)

#define MI_ERR(format, ...)                                                    \
	do {								       \
		if (DEBUG_ERR & Log_level) {                                   \
			printk(KERN_ERR MI_TAG format "\n", ##__VA_ARGS__);    \
		}							       \
	} while (0)

#define MI_FUN                                                                 \
	do {								       \
		if (DEBUG_FUNC & Log_level) {                                  \
			printk(KERN_ERR MI_TAG "%s is called, line: %d\n",     \
			       __FUNCTION__, __LINE__);                        \
		}							       \
	} while (0)

#define MI_ASSERT(expr)                                                        \
	do {								       \
		if (!(expr)) {                                                 \
		printk(KERN_ERR "Assertion failed! %s,%d,%s,%s\n", __FILE__,   \
			       __LINE__, __func__, #expr);                     \
		}							       \
	} while (0)
/*----------------------------------------------------------------------------*/
#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
static char OffsetFileName[] = "/data/misc/miraGSensorOffset.txt";
#define OFFSET_STRING_LEN 26
struct work_info {
	char tst1[20];
	char tst2[20];
	char buffer[OFFSET_STRING_LEN];
	struct workqueue_struct *wq;
	struct delayed_work read_work;
	struct delayed_work write_work;
	struct completion completion;
	int len;
	int rst;
};

static struct work_info m_work_info = {{0} };
/*----------------------------------------------------------------------------*/
static void sensor_write_work(struct work_struct *work)
{
	struct work_info *pWorkInfo;
	struct file *filep;
	u32 orgfs;
	int ret;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	pWorkInfo = container_of((struct delayed_work *)work, struct work_info,
				 write_work);
	if (pWorkInfo == NULL) {
		MI_ERR("get pWorkInfo failed!");
		return;
	}

	filep = filp_open(OffsetFileName, O_RDWR | O_CREAT, 0600);
	if (IS_ERR(filep)) {
		MI_ERR("write, sys_open %s error!!.\n", OffsetFileName);
		ret = -1;
	} else {
		filep->f_op->write(filep, pWorkInfo->buffer, pWorkInfo->len,
				   &filep->f_pos);
		filp_close(filep, NULL);
		ret = 0;
	}

	set_fs(orgfs);
	pWorkInfo->rst = ret;
	complete(&pWorkInfo->completion);
}
/*----------------------------------------------------------------------------*/
static void sensor_read_work(struct work_struct *work)
{
	u32 orgfs;
	struct file *filep;
	int ret;
	struct work_info *pWorkInfo;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	pWorkInfo = container_of((struct delayed_work *)work, struct work_info,
				 read_work);
	if (pWorkInfo == NULL) {
		MI_ERR("get pWorkInfo failed!");
		return;
	}

	filep = filp_open(OffsetFileName, O_RDONLY, 0600);
	if (IS_ERR(filep)) {
		MI_MSG("read, sys_open %s error!!.\n", OffsetFileName);
		set_fs(orgfs);
		ret = -1;
	} else {
		filep->f_op->read(filep, pWorkInfo->buffer,
				  sizeof(pWorkInfo->buffer), &filep->f_pos);
		filp_close(filep, NULL);
		set_fs(orgfs);
		ret = 0;
	}

	pWorkInfo->rst = ret;
	complete(&(pWorkInfo->completion));
}
/*----------------------------------------------------------------------------*/
static int sensor_sync_read(u8 *offset)
{
	int err;
	int off[MIR3DA_OFFSET_LEN] = {0};
	struct work_info *pWorkInfo = &m_work_info;

	init_completion(&pWorkInfo->completion);
	queue_delayed_work(pWorkInfo->wq, &pWorkInfo->read_work,
			   msecs_to_jiffies(0));
	err = wait_for_completion_timeout(&pWorkInfo->completion,
					  msecs_to_jiffies(2000));
	if (err == 0) {
		MI_ERR("wait_for_completion_timeout TIMEOUT");
		return -1;
	}

	if (pWorkInfo->rst != 0) {
		MI_ERR("work_info.rst  not equal 0");
		return pWorkInfo->rst;
	}

	sscanf(m_work_info.buffer, "%x,%x,%x,%x,%x,%x,%x,%x,%x", &off[0],
	       &off[1], &off[2], &off[3], &off[4], &off[5], &off[6], &off[7],
	       &off[8]);

	offset[0] = (u8)off[0];
	offset[1] = (u8)off[1];
	offset[2] = (u8)off[2];
	offset[3] = (u8)off[3];
	offset[4] = (u8)off[4];
	offset[5] = (u8)off[5];
	offset[6] = (u8)off[6];
	offset[7] = (u8)off[7];
	offset[8] = (u8)off[8];

	return 0;
}
/*----------------------------------------------------------------------------*/
static int sensor_sync_write(u8 *off)
{

	int err = 0;
	struct work_info *pWorkInfo = &m_work_info;

	init_completion(&pWorkInfo->completion);

	sprintf(m_work_info.buffer, "%x,%x,%x,%x,%x,%x,%x,%x,%x\n", off[0],
		off[1], off[2], off[3], off[4], off[5], off[6], off[7], off[8]);

	pWorkInfo->len = sizeof(m_work_info.buffer);

	queue_delayed_work(pWorkInfo->wq, &pWorkInfo->write_work,
			   msecs_to_jiffies(0));
	err = wait_for_completion_timeout(&pWorkInfo->completion,
					  msecs_to_jiffies(2000));
	if (err == 0) {
		MI_ERR("wait_for_completion_timeout TIMEOUT");
		return -1;
	}

	if (pWorkInfo->rst != 0) {
		MI_ERR("work_info.rst  not equal 0");
		return pWorkInfo->rst;
	}

	return 0;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef MIR3DA_AUTO_CALIBRAE
static bool check_califile_exist(void)
{
	u32 orgfs = 0;
	struct file *filep;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	filep = filp_open(OffsetFileName, O_RDONLY, 0600);
	if (IS_ERR(filep)) {
		MI_MSG("%s read, sys_open %s error!!.\n", __func__,
		       OffsetFileName);
		set_fs(orgfs);
		return false;
	}

	filp_close(filep, NULL);
	set_fs(orgfs);

	return true;
}
#endif
/*----------------------------------------------------------------------------*/

/* deal gsensor data owl */
#if 0
static void calc_gvalue(unsigned short g, int *fg, int isPositive)
{
	int MaxSense = 8000.0f;      /*+-4g */
	int AdcResolution = 1 << 12; /* 12bit */

	/* int temp = 0.00f; */
	/* int tempRadio = MaxSense/AdcResolution; */
	/* temp = (g*tempRadio) / 1000; */

	*fg = (MaxSense * g) / AdcResolution;

	if (isPositive == 0) {
		*fg = 0 - *fg;
	}
}

static unsigned short abs_1(short n)
{
	if (n < 0) {
		return -n;
	} else {
		return n;
	}
}

static void Gfabs(int *a, int b)
{
	*a = (((b) < 0) ? -(b) : (b));
}

static void Max3(int *dtmax, int a, int b, int c)
{
	int temp = 0.0;
	if (temp < a) {
		mGsensorOccurAxis = 0x78;
		temp = a;
	}
	if (temp < b) {
		mGsensorOccurAxis = 0x79;
		temp = b;
	}
	if (temp < c) {
		mGsensorOccurAxis = 0x7A;
		temp = c;
	}
	*dtmax = temp;
}

static void CheckImpactEven(int x, int y, int z)
{
	static int g_data[5][3] = {0};
	static int gc;
	static int ThresTrigCnt;
	static int avg[3] = {0};
	static int dtmax, dtx, dty, dtz;
	int i = 0, j = 0;

	if (gc < 5) {
		g_data[gc][0] = x;
		g_data[gc][1] = y;
		g_data[gc][2] = z;
		gc++;
		avg[0] = (g_data[0][0] + g_data[1][0] + g_data[2][0] +
			  g_data[3][0] + g_data[4][0]) /
			 gc;
		avg[1] = (g_data[0][1] + g_data[1][1] + g_data[2][1] +
			  g_data[3][1] + g_data[4][1]) /
			 gc;
		avg[2] = (g_data[0][2] + g_data[1][2] + g_data[2][2] +
			  g_data[3][2] + g_data[4][2]) /
			 gc;
		return;
	} else {
		/* printk("\nx = %.2f  avg[0] = %.2f \n", x, avg[0]); */
		/* printk("y = %.2f  avg[1] = %.2f \n", y, avg[1]); */
		/* printk("z = %.2f  avg[2] = %.2f \n", z, avg[2]); */

		Gfabs(&dtx, (x - avg[0]));
		Gfabs(&dty, (y - avg[1]));
		Gfabs(&dtz, (z - avg[2]));
		Max3(&dtmax, dtx, dty, dtz);
		/* printk("G-sensor dtmax:%d g_target=<%d> gc:%d  axis=<%c> */
		/* impact_happen_level=%d  \n\n",dtmax, g_target, gc, */
		/* mGsensorOccurAxis, impact_happen_level); */

		if ((dtmax > g_target) &&
		    (impact_happen_level != G_SENSOR_IMPACT_LEVEL_CLOSE)) {
			ThresTrigCnt++;
			if (ThresTrigCnt >= GSENSOR_THRES_TRIG_CNT) {
				ThresTrigCnt = 0;
				/* owl fix ,impact record video */
				/* int2_status=1; */
				impact_status = 1;
				printk("G-sensor dtmax=%d g_target=<%d> gc=%d "
				       "axis=<%c> impact_status=1  !!!!!!  "
				       "\n\n",
				       dtmax, g_target, gc, mGsensorOccurAxis);

				gc = 0;
				return;
			}
		}

		for (i = 0; i < 4; i++) {
			for (j = 0; j < 3; j++) {
				g_data[i][j] = g_data[i + 1][j];
			}
		}
		g_data[4][0] = x;
		g_data[4][1] = y;
		g_data[4][2] = z;

		avg[0] = (g_data[0][0] + g_data[1][0] + g_data[2][0] +
			  g_data[3][0] + g_data[4][0]) /
			 5;
		avg[1] = (g_data[0][1] + g_data[1][1] + g_data[2][1] +
			  g_data[3][1] + g_data[4][1]) /
			 5;
		avg[2] = (g_data[0][2] + g_data[1][2] + g_data[2][2] +
			  g_data[3][2] + g_data[4][2]) /
			 5;
	}
}
#endif

static void report_abs(void)
{
	short x = 0, y = 0, z = 0;
	/* int gx = 0, gy = 0, gz = 0; */
	MIR_HANDLE handle = mir_handle;

	if (mir3da_read_data(handle, &x, &y, &z) != 0) {
		MI_ERR("MIR3DA data read failed!\n");
		return;
	}

	/*
	calc_gvalue(abs_1(x), &gx, x>0);
	calc_gvalue(abs_1(y), &gy, y>0);
	calc_gvalue(abs_1(z), &gz, z>0);
	CheckImpactEven(abs_1(gx), abs_1(gy), abs_1(gz));
	*/
	/* MI_ERR("mir3da_filt KK: x=%d, y=%d, z=%d\n",  x, y, z); */
	/* MI_ERR("mir3da_filt KK: gx=%d, gy=%d, gz=%d\n",  gx, gy, gz); */

	input_report_abs(mir3da_idev->input, ABS_X, x);
	input_report_abs(mir3da_idev->input, ABS_Y, y);
	input_report_abs(mir3da_idev->input, ABS_Z, z);
	/*
	    input_report_abs(mir3da_idev->input, ABS_X, gx);
	    input_report_abs(mir3da_idev->input, ABS_Y, gy);
	    input_report_abs(mir3da_idev->input, ABS_Z, gz);
	    */
	input_sync(mir3da_idev->input);
}
/*----------------------------------------------------------------------------*/
static void mir3da_dev_poll(struct input_polled_dev *dev)
{
	dev->poll_interval = delayMs;
	report_abs();
}
/*----------------------------------------------------------------------------*/
static long mir3da_misc_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int err = 0;
	int interval = 0;
	char bEnable = 0;
	/*    int             z_dir = 0; */
	/*    int             range = 0; */
	short xyz[3] = {0};
	MIR_HANDLE handle = mir_handle;

	if (_IOC_DIR(cmd) & _IOC_READ) {
		err = !access_ok(VERIFY_WRITE, (void __user *)arg,
				 _IOC_SIZE(cmd));
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		err =
		    !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if (err) {
		return -EFAULT;
	}

	switch (cmd) {
	case MIR3DA_ACC_IOCTL_GET_DELAY:
		interval = POLL_INTERVAL;
		if (copy_to_user(argp, &interval, sizeof(interval)))
			return -EFAULT;
		break;

	case MIR3DA_ACC_IOCTL_SET_DELAY:
		if (copy_from_user(&interval, argp, sizeof(interval)))
			return -EFAULT;
		if (interval < 0 || interval > 1000)
			return -EINVAL;
		if ((interval <= 30) && (interval > 10)) {
			interval = 10;
		}
		delayMs = interval;
		break;

	case MIR3DA_ACC_IOCTL_SET_ENABLE:
		if (copy_from_user(&bEnable, argp, sizeof(bEnable)))
			return -EFAULT;

		err = mir3da_set_enable(handle, bEnable);
		if (err < 0)
			return EINVAL;
		break;

	case MIR3DA_ACC_IOCTL_GET_ENABLE:
		err = mir3da_get_enable(handle, &bEnable);
		if (err < 0) {
			return -EINVAL;
		}

		if (copy_to_user(argp, &bEnable, sizeof(bEnable)))
			return -EINVAL;
		break;

#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
	case MIR3DA_ACC_IOCTL_CALIBRATION:
		if (copy_from_user(&z_dir, argp, sizeof(z_dir)))
			return -EFAULT;

		if (mir3da_calibrate(handle, z_dir)) {
			return -EFAULT;
		}

		if (copy_to_user(argp, &z_dir, sizeof(z_dir)))
			return -EFAULT;
		break;

	case MIR3DA_ACC_IOCTL_UPDATE_OFFSET:
		manual_load_cali_file(handle);
		break;
#endif

	case MIR3DA_ACC_IOCTL_GET_COOR_XYZ:

		if (mir3da_read_data(handle, &xyz[0], &xyz[1], &xyz[2]))
			return -EFAULT;

		if (copy_to_user((void __user *)arg, xyz, sizeof(xyz)))
			return -EFAULT;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
static const struct file_operations mir3da_misc_fops = {
    .owner = THIS_MODULE, .unlocked_ioctl = mir3da_misc_ioctl,
};

static struct miscdevice misc_mir3da = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = MIR3DA_MISC_NAME,
    .fops = &mir3da_misc_fops,
};
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_enable_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	int ret;
	char bEnable;
	MIR_HANDLE handle = mir_handle;

	ret = mir3da_get_enable(handle, &bEnable);
	if (ret < 0) {
		ret = -EINVAL;
	} else {
		ret = sprintf(buf, "%d\n", bEnable);
	}

	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_enable_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	int ret;
	char bEnable;
	unsigned long enable;
	MIR_HANDLE handle = mir_handle;

	if (buf == NULL) {
		return -1;
	}

	enable = simple_strtoul(buf, NULL, 10);
	bEnable = (enable > 0) ? 1 : 0;

	ret = mir3da_set_enable(handle, bEnable);
	if (ret < 0) {
		ret = -EINVAL;
	} else {
		ret = count;
	}

	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_delay_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", delayMs);
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_delay_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int interval = 0;

	interval = simple_strtoul(buf, NULL, 10);

	if (interval < 0 || interval > 1000)
		return -EINVAL;

	if ((interval <= 30) && (interval > 10)) {
		interval = 10;
	}

	delayMs = interval;

	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_axis_data_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int result;
	short x, y, z;
	int count = 0;
	MIR_HANDLE handle = mir_handle;

	result = mir3da_read_data(handle, &x, &y, &z);
	if (result == 0)
		count += sprintf(buf + count, "x= %d;y=%d;z=%d\n", x, y, z);
	else
		count += sprintf(buf + count, "reading failed!");

	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_reg_data_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	int addr, data;
	int result;
	MIR_HANDLE handle = mir_handle;

	sscanf(buf, "0x%x, 0x%x\n", &addr, &data);

	result = mir3da_register_write(handle, addr, data);

	MI_ASSERT(result == 0);

	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_reg_data_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	MIR_HANDLE handle = mir_handle;

	return mir3da_get_reg_data(handle, buf);
}
/*----------------------------------------------------------------------------*/
#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
static ssize_t mir3da_offset_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	ssize_t count = 0;
	int rst = 0;
	u8 off[9] = {0};
	MIR_HANDLE handle = mir_handle;

	rst = mir3da_read_offset(handle, off);
	if (!rst) {
		count = sprintf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", off[0],
				off[1], off[2], off[3], off[4], off[5], off[6],
				off[7], off[8]);
	}
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_offset_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	int off[9] = {0};
	u8 offset[9] = {0};
	int rst = 0;
	MIR_HANDLE handle = mir_handle;

	sscanf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", &off[0], &off[1], &off[2],
	       &off[3], &off[4], &off[5], &off[6], &off[7], &off[8]);

	offset[0] = (u8)off[0];
	offset[1] = (u8)off[1];
	offset[2] = (u8)off[2];
	offset[3] = (u8)off[3];
	offset[4] = (u8)off[4];
	offset[5] = (u8)off[5];
	offset[6] = (u8)off[6];
	offset[7] = (u8)off[7];
	offset[8] = (u8)off[8];

	rst = mir3da_write_offset(handle, offset);
	return count;
}
#endif

/*----------------------------------------------------------------------------*/
#if FILTER_AVERAGE_ENHANCE
static ssize_t mir3da_average_enhance_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	int ret = 0;
	struct mir3da_filter_param_s param = {0};

	ret = mir3da_get_filter_param(&param);
	ret |= sprintf(buf, "%d %d %d\n", param.filter_param_l,
		       param.filter_param_h, param.filter_threhold);

	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_average_enhance_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	int ret = 0;
	struct mir3da_filter_param_s param = {0};

	sscanf(buf, "%d %d %d\n", &param.filter_param_l, &param.filter_param_h,
	       &param.filter_threhold);

	ret = mir3da_set_filter_param(&param);

	return count;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
int bCaliResult = -1;
static ssize_t mir3da_calibrate_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int ret;

	ret = sprintf(buf, "%d\n", bCaliResult);
	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_calibrate_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	s8 z_dir = 0;
	MIR_HANDLE handle = mir_handle;

	z_dir = simple_strtol(buf, NULL, 10);
	bCaliResult = mir3da_calibrate(handle, z_dir);

	return count;
}
#endif
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_log_level_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int ret;

	ret = sprintf(buf, "%d\n", Log_level);

	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_log_level_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	Log_level = simple_strtoul(buf, NULL, 10);

	return count;
}

static ssize_t mir3da_int2_enable_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int on;
	int res = 0;
	MIR_HANDLE handle = mir_handle;

	int2_enable = simple_strtoul(buf, NULL, 10);
	on = int2_enable;

	if (parking_impact_sensitivity_close == 1) {
		int2_enable = 0;
		on = int2_enable;
		printk("mir3da_int2_enable_store "
		       "parking_impact_sensitivity_close close, not enable int "
		       "\n");
	}

	printk("mir3da_int2_enable_store on=%d slope_th=0x%x  "
	       "int_latch_mode=0x%x\n",
	       on, slope_th, int_latch_mode);

	if (on) {
		res |= mir3da_register_mask_write(handle, NSA_REG_G_RANGE, 0x0f,
						  0x05); /* 12-bit +/- 4g */
		/* res |= mir3da_register_mask_write(handle, */
		/* NSA_REG_POWERMODE_BW, 0xFF, 0x5E); */
		/* res |= mir3da_register_mask_write(handle, */
		/* NSA_REG_ODR_AXIS_DISABLE, 0xFF, 0x07); */
		res |=
		    mir3da_register_mask_write(handle, NSA_REG_INT_PIN_CONFIG,
					       0x0F, 0x01); /* set int_pin level */

		res |= mir3da_register_mask_write(
		    handle, NSA_REG_INTERRUPT_SETTINGS1, 0x07, 0x07); /* enable */
		res |= mir3da_register_mask_write(
		    handle, NSA_REG_ACTIVE_DURATION, 0x03, 0x03);
		res |= mir3da_register_write(handle, NSA_REG_ACTIVE_THRESHOLD,
					     slope_th);

		res |= mir3da_register_mask_write(handle,
						  NSA_REG_INTERRUPT_MAPPING1,
						  0x04, 0x04); /* enable int1 */

		res |= mir3da_register_mask_write(handle, NSA_REG_INT_LATCH,
						  0b00001100, 0b00001100);

		res |= mir3da_register_mask_write(
		    handle, NSA_REG_ENGINEERING_MODE, 0xFF, 0x83);
		res |= mir3da_register_mask_write(
		    handle, NSA_REG_ENGINEERING_MODE, 0xFF, 0x69);
		res |= mir3da_register_mask_write(
		    handle, NSA_REG_ENGINEERING_MODE, 0xFF, 0xBD);
	} else {
		res |= mir3da_register_write(handle,
					     NSA_REG_INTERRUPT_SETTINGS1, 0x00);
		res |= mir3da_register_write(handle, NSA_REG_INTERRUPT_MAPPING1,
					     0x00);
	}

	/* reset all impact status when switch impact mode */
	impact_status = 0;
	int2_status = 0;
	/*
	res |= mir3da_register_mask_write(handle, NSA_REG_INT_LATCH, 0x8F,
	int_latch_mode);
	// 83 1s 84 2s 85 4s  86 8s  8f yiz
	res |= mir3da_register_mask_write(handle, NSA_REG_ACTIVE_DURATION, 0xff,
	0x03);
	res |= mir3da_register_mask_write(handle, NSA_REG_ACTIVE_THRESHOLD,
	0xff, slope_th);//0x10
	//res |= mir3da_register_mask_write(handle, NSA_REG_ODR_AXIS_DISABLE,
	0xFF, 0x07);
	res |= mir3da_register_mask_write(handle, NSA_REG_INT_PIN_CONFIG, 0x0F,
	0x01); //0x03
	//res |= mir3da_register_mask_write(handle, NSA_REG_POWERMODE_BW, 0xFF,
	0x5E);
	if (on)
	{
	    res |= mir3da_register_mask_write(handle,
	NSA_REG_INTERRUPT_SETTINGS1, 0xff, 0x07); //0x03
	    res |= mir3da_register_mask_write(handle,
	NSA_REG_INTERRUPT_MAPPING1, 0xff, 0x04);

	    res |= mir3da_register_mask_write(handle, NSA_REG_ENGINEERING_MODE,
	0xFF, 0x83);
	    res |= mir3da_register_mask_write(handle, NSA_REG_ENGINEERING_MODE,
	0xFF, 0x69);
	    res |= mir3da_register_mask_write(handle, NSA_REG_ENGINEERING_MODE,
	0xFF, 0xBD);

	}
	else
	{
	    res |= mir3da_register_mask_write(handle,
	NSA_REG_INTERRUPT_SETTINGS1, 0xff, 0x00);
	    res |= mir3da_register_mask_write(handle,
	NSA_REG_INTERRUPT_MAPPING1, 0xff, 0x00);
	    res |= mir3da_register_mask_write(handle,
	NSA_REG_INTERRUPT_MAPPING3, 0xff, 0x00);
	}*/
	return count;
}

static ssize_t mir3da_int2_enable_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	int ret;

	ret = sprintf(buf, "%d\n", int2_enable);
	printk(" mir3da_int2_enable_show ret [ %d ]\n", ret);
	return ret;
}

static ssize_t mir3da_int2_clear_enable_store(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf, size_t count)
{

	MIR_HANDLE handle = mir_handle;

	printk(" mir3da_int2_clear_enable_store int2_clean \n");
	mir3da_clear_intterrupt(handle);

	return count;
}

static ssize_t mir3da_int2_clear_enable_show(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	int ret = 0;

	/*	ret = sprintf(buf, "%d\n", int2_enable); */
	/*	printk(" mir3da_int2_clear_enable_show ret [ %d ]\n",ret); */
	return ret;
}

static ssize_t mir3da_int2_start_statu_store(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	/*	MIR_HANDLE      handle = mir_handle; */
	int2_status = simple_strtoul(buf, NULL, 10);

	return count;
}

static ssize_t mir3da_int2_start_statu_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	int ret;
	/* MIR_HANDLE      handle = mir_handle; */
	/* int2_status =  mir3da_read_int_status( handle); */

	ret = sprintf(buf, "%d\n", int2_status);
	/* printk(" mir3da_int2_start_statu_show parking int int2_status=[ %d */
	/* ]\n\n", int2_status); */
	return ret;
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_primary_offset_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	MIR_HANDLE handle = mir_handle;
	int x = 0, y = 0, z = 0;

	mir3da_get_primary_offset(handle, &x, &y, &z);

	printk("mir3da_primary_offset_show x=%d ,y=%d ,z=%d   \n", x, y, z);

	return sprintf(buf, "x=%d ,y=%d ,z=%d\n", x, y, z);
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_version_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%s_%s\n", DRI_VER, CORE_VER);
}
/*----------------------------------------------------------------------------*/
static ssize_t mir3da_vendor_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", "MiraMEMS");
}

static ssize_t mir3da_slope_th_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%x\n", slope_th);
}

static ssize_t mir3da_slope_th_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	/* parking mode */
	unsigned long data;
	int error = 0;
	MIR_HANDLE handle = mir_handle;

	error = kstrtoul(buf, 16, &data);
	if (error)
		return error;
	/*
	parking_impact_sensitivity_close = 0;
    if(data == G_SENSOR_IMPACT_LEVEL_CLOSE)
    {
	//g_target = 10000;
		parking_impact_sensitivity_close = 1;
    }
    else if(data == G_SENSOR_IMPACT_LEVEL_HIGH)
    {
	slope_th = G_SENSOR_SENSI_PARKING_H_VAL/GSENSOR_K + 1;
    }
    else if(data == G_SENSOR_IMPACT_LEVEL_MIDDLE)
    {
	slope_th = G_SENSOR_SENSI_PARKING_M_VAL/GSENSOR_K + 1;
    }
    else if(data == G_SENSOR_IMPACT_LEVEL_LOW)
    {
	slope_th = G_SENSOR_SENSI_PARKING_L_VAL/GSENSOR_K + 1;
    }
    else
    {
	printk("mir3da_impact_happen_level_store impact_happen_level error!!!
    \n");
    }*/
	slope_th = data;

	error = mir3da_register_mask_write(handle, NSA_REG_ACTIVE_THRESHOLD,
					   0xff, slope_th);

	printk("set data=%ld  slope=0x%x  result=%d \n", data, slope_th, error);

	return count;
}

static ssize_t mir3da_impact_status_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	/*	MIR_HANDLE      handle = mir_handle; */
	impact_status = simple_strtoul(buf, NULL, 10);

	return count;
}

static ssize_t mir3da_impact_status_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	int ret;
	ret = sprintf(buf, "%d\n", impact_status);
	/* printk(" mir3da_impact_status_show  normal impact_status = [ %d ]\n", */
	/* impact_status); */
	return ret;
}

static ssize_t mir3da_impact_happen_level_store(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	/* normal mode */
	/*	MIR_HANDLE      handle = mir_handle; */
	impact_happen_level = simple_strtoul(buf, NULL, 10);
	if (impact_happen_level == G_SENSOR_IMPACT_LEVEL_CLOSE) {
		/* g_target = 10000; */
	} else if (impact_happen_level == G_SENSOR_IMPACT_LEVEL_HIGH) {
		g_target = G_SENSOR_SENSI_H_VAL;
	} else if (impact_happen_level == G_SENSOR_IMPACT_LEVEL_MIDDLE) {
		g_target = G_SENSOR_SENSI_M_VAL;
	} else if (impact_happen_level == G_SENSOR_IMPACT_LEVEL_LOW) {
		g_target = G_SENSOR_SENSI_L_VAL;
	} else {
		printk("mir3da_impact_happen_level_store impact_happen_level "
		       "error!!! \n");
	}

	printk("mir3da_impact_happen_level_store impact_happen_level=%d "
	       "g_target=%d \n",
	       impact_happen_level, g_target);

	return count;
}

static ssize_t mir3da_impact_happen_level_show(struct device *dev,
					       struct device_attribute *attr,
					       char *buf)
{
	int ret;
	ret = sprintf(buf, "%d\n", impact_happen_level);
	/* printk("mir3da_impact_happen_level_show impact_happen_level=%d */
	/* g_target=%d \n", impact_happen_level, g_target); */
	return ret;
}

static ssize_t axis_x_value_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret = -1;
	MIR_HANDLE handle = mir_handle;
	if (mir3da_read_data(handle, &axis_x_value, &axis_y_value,
			     &axis_z_value) != 0) {
		MI_ERR("MIR3DA data read failed!\n");
		return ret;
	}

	ret = sprintf(buf, "%d\n", axis_x_value);
	/* printk("axis_x_value_show axis_x_value=%d \n", axis_x_value); */
	return ret;
}

static ssize_t axis_y_value_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret;
	ret = sprintf(buf, "%d\n", axis_y_value);
	/* printk("axis_x_value_show axis_x_value=%d \n", axis_x_value); */
	return ret;
}

static ssize_t axis_z_value_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret;
	ret = sprintf(buf, "%d\n", axis_z_value);
	/* printk("axis_x_value_show axis_x_value=%d \n", axis_x_value); */
	return ret;
}

/*----------------------------------------------------------------------------*/
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR, mir3da_enable_show,
		   mir3da_enable_store);
static DEVICE_ATTR(delay, S_IRUGO | S_IWUSR, mir3da_delay_show,
		   mir3da_delay_store);
static DEVICE_ATTR(axis_data, S_IRUGO | S_IWUSR, mir3da_axis_data_show, NULL);
static DEVICE_ATTR(reg_data, S_IWUSR | S_IRUGO, mir3da_reg_data_show,
		   mir3da_reg_data_store);
static DEVICE_ATTR(log_level, S_IWUSR | S_IRUGO, mir3da_log_level_show,
		   mir3da_log_level_store);
#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
static DEVICE_ATTR(offset, S_IWUSR | S_IRUGO, mir3da_offset_show,
		   mir3da_offset_store);
static DEVICE_ATTR(calibrate_miraGSensor, S_IWUSR | S_IRUGO,
		   mir3da_calibrate_show, mir3da_calibrate_store);
#endif
#ifdef FILTER_AVERAGE_ENHANCE
static DEVICE_ATTR(average_enhance, S_IWUSR | S_IRUGO,
		   mir3da_average_enhance_show, mir3da_average_enhance_store);
#endif
/* aad cz */
static DEVICE_ATTR(int2_enable, S_IRUGO | S_IWUSR, mir3da_int2_enable_show,
		   mir3da_int2_enable_store);
static DEVICE_ATTR(int2_clear, S_IRUGO | S_IWUSR, mir3da_int2_clear_enable_show,
		   mir3da_int2_clear_enable_store);
static DEVICE_ATTR(int2_start_status, S_IRUGO | S_IWUSR,
		   mir3da_int2_start_statu_show, mir3da_int2_start_statu_store);

static DEVICE_ATTR(primary_offset, S_IWUSR, mir3da_primary_offset_show, NULL);
static DEVICE_ATTR(version, S_IRUGO, mir3da_version_show, NULL);
static DEVICE_ATTR(vendor, S_IRUGO, mir3da_vendor_show, NULL);
/* add owl */
static DEVICE_ATTR(slope_th, S_IRUGO | S_IWUSR, mir3da_slope_th_show,
		   mir3da_slope_th_store);
static DEVICE_ATTR(impact_status, S_IRUGO | S_IWUSR, mir3da_impact_status_show,
		   mir3da_impact_status_store);
static DEVICE_ATTR(impact_happen_level, S_IRUGO | S_IWUSR,
		   mir3da_impact_happen_level_show,
		   mir3da_impact_happen_level_store);
static DEVICE_ATTR(axis_x_value, S_IRUGO | S_IWUSR, axis_x_value_show, NULL);
static DEVICE_ATTR(axis_y_value, S_IRUGO | S_IWUSR, axis_y_value_show, NULL);
static DEVICE_ATTR(axis_z_value, S_IRUGO | S_IWUSR, axis_z_value_show, NULL);

/*----------------------------------------------------------------------------*/
static struct attribute *mir3da_attributes[] = {
    &dev_attr_enable.attr,
    &dev_attr_delay.attr,
    &dev_attr_axis_data.attr,
    &dev_attr_reg_data.attr,
    &dev_attr_log_level.attr,
#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
    &dev_attr_offset.attr,
    &dev_attr_calibrate_miraGSensor.attr,
/*	&dev_attr_primary_offset.attr, */
#endif
#ifdef FILTER_AVERAGE_ENHANCE
    &dev_attr_average_enhance.attr,
#endif /* ! FILTER_AVERAGE_ENHANCE */
    &dev_attr_int2_enable.attr,
    &dev_attr_int2_clear.attr,
    &dev_attr_int2_start_status.attr,
    &dev_attr_primary_offset.attr,
    &dev_attr_version.attr,
    &dev_attr_vendor.attr,
    &dev_attr_slope_th.attr,
    &dev_attr_impact_status.attr,
    &dev_attr_impact_happen_level.attr,
    &dev_attr_axis_x_value.attr,
    &dev_attr_axis_y_value.attr,
    &dev_attr_axis_z_value.attr,
    NULL};

static const struct attribute_group mir3da_attr_group = {
    .attrs = mir3da_attributes,
};
/*----------------------------------------------------------------------------*/
int i2c_smbus_read(PLAT_HANDLE handle, u8 addr, u8 *data)
{
	int res = 0;
	struct i2c_client *client = (struct i2c_client *)handle;

	*data = i2c_smbus_read_byte_data(client, addr);

	return res;
}
/*----------------------------------------------------------------------------*/
int i2c_smbus_read_block(PLAT_HANDLE handle, u8 addr, u8 count, u8 *data)
{
	int res = 0;
	struct i2c_client *client = (struct i2c_client *)handle;

	res = i2c_smbus_read_i2c_block_data(client, addr, count, data);

	/* printk("read_block  data=%d %d %d %d  \n", data[0], data[1], data[2], */
	/* data[3]); */

	return res;
}
/*----------------------------------------------------------------------------*/
int i2c_smbus_write(PLAT_HANDLE handle, u8 addr, u8 data)
{
	int res = 0;
	struct i2c_client *client = (struct i2c_client *)handle;

	res = i2c_smbus_write_byte_data(client, addr, data);

	return res;
}
/*----------------------------------------------------------------------------*/
void msdelay(int ms)
{
	mdelay(ms);
}

static void initDA380Reg(void)
{
	/* unsigned char tmp_data; */
	MIR_HANDLE handle = mir_handle;
	int res = 0;

	/* set chip rang +-4g */
	if (mir3da_register_mask_write(handle, NSA_REG_G_RANGE, 0x0f, 0x05)) {
		MI_ERR("[OWL-da380]i2c mask write NSA_REG_G_RANGE failed !\n");
		return;
	}

	/* res = mir3da_register_read(handle, NSA_REG_G_RANGE, &tmp_data); */
	/* printk("[OWL-da380] initDA380Reg  NSA_REG_G_RANGE=%d  res=%d  \n", */
	/* tmp_data, res); */

	/* res = mir3da_register_read(handle, NSA_REG_INT_PIN_CONFIG, */
	/* &tmp_data); */
	/* printk("[OWL-da380] initDA380Reg  NSA_REG_INT_PIN_CONFIG=%d  res=%d */
	/* \n", tmp_data, res); */

	/* mir3da_register_mask_write(handle, NSA_REG_INT_PIN_CONFIG, 0x83, */
	/* 0x01); */

	/*
	 *
	 * Latch interrupt mode:
	 *       0b000 0000:    no-latched
	 *       0b000 0001:    250ms
	 *       0b000 0010:    500ms
	 *       0b000 0011:    1s
	 *       0b000 0100:    2s
	 *       0b000 0101:    4s
	 *       0b000 0110:    8s
	 *       0b000 1001:    1ms
	 *       0b000 1010:    1ms
	 *       0b000 1011:    2ms
	 *       0b000 1100:    25ms
	 *       0b000 1101:    50ms
	 *       0b000 1110:    100ms
	 *
	 */
	res |=
	    mir3da_register_mask_write(handle, NSA_REG_INT_LATCH, 0b00001100, 0b00001100);

	/* 83 1s 84 2s 85 4s  86 8s  8f yiz */
	res |= mir3da_register_mask_write(handle, NSA_REG_ACTIVE_DURATION, 0xff,
					  0x03);
	res |= mir3da_register_mask_write(handle, NSA_REG_ACTIVE_THRESHOLD,
					  0xff, slope_th); /* 0x10 */
	/* res |= mir3da_register_mask_write(handle, NSA_REG_ODR_AXIS_DISABLE, */
	/* 0xFF, 0x07); */
	res |= mir3da_register_mask_write(handle, NSA_REG_INT_PIN_CONFIG, 0x0F,
					  0x01); /* 0x03 */
	/* res |= mir3da_register_mask_write(handle, NSA_REG_POWERMODE_BW, 0xFF, */
	/* 0x5E); */
}

static irqreturn_t gsensor_irq_func(int irq, void *priv)
{
	int2_status = 1;

	printk("[OWL-da380] *** gsensor_irq_func  irq=%d int2_status=1  \n",
	       irq);
	return IRQ_HANDLED;
}

#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
MIR_GENERAL_OPS_DECLARE(ops_handle, i2c_smbus_read, i2c_smbus_read_block,
			i2c_smbus_write, sensor_sync_write, sensor_sync_read,
			msdelay, printk, sprintf);
#else
MIR_GENERAL_OPS_DECLARE(ops_handle, i2c_smbus_read, i2c_smbus_read_block,
			i2c_smbus_write, NULL, NULL, msdelay, printk, sprintf);
#endif
/*----------------------------------------------------------------------------*/
static int mir3da_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int result = 0;
	struct input_dev *idev;
	struct device_node *np = NULL;

	if (mir3da_install_general_ops(&ops_handle)) {
		MI_ERR("Install ops failed !\n");
		goto err_detach_client;
	}

#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
	m_work_info.wq = create_singlethread_workqueue("oo");
	if (NULL == m_work_info.wq) {
		MI_ERR("Failed to create workqueue !");
		goto err_detach_client;
	}

	INIT_DELAYED_WORK(&m_work_info.read_work, sensor_read_work);
	INIT_DELAYED_WORK(&m_work_info.write_work, sensor_write_work);
#endif

	int2_status = mir3da_read_int_status((PLAT_HANDLE)client);
	printk("ParkMonitor* powerOn status is %d\n", int2_status);

	/* Initialize the MIR3DA chip */
	mir_handle = mir3da_core_init((PLAT_HANDLE)client);
	if (NULL == mir_handle) {
		MI_ERR("chip init failed !\n");
		goto err_detach_client;
	}

	/* input poll device register */
	mir3da_idev = input_allocate_polled_device();
	if (!mir3da_idev) {
		MI_ERR("alloc poll device failed!\n");
		result = -ENOMEM;
		goto err_detach_client;
	}
	mir3da_idev->poll = mir3da_dev_poll;
	mir3da_idev->poll_interval = POLL_INTERVAL;
	delayMs = POLL_INTERVAL;
	mir3da_idev->poll_interval_max = POLL_INTERVAL_MAX;
	idev = mir3da_idev->input;

	idev->name = MIR3DA_INPUT_DEV_NAME;
	idev->id.bustype = BUS_I2C;
	idev->evbit[0] = BIT_MASK(EV_ABS);

	printk("ParkMonitor input_set_abs_params  \n");

	input_set_abs_params(idev, ABS_X, -16384, 16383, INPUT_FUZZ,
			     INPUT_FLAT);
	input_set_abs_params(idev, ABS_Y, -16384, 16383, INPUT_FUZZ,
			     INPUT_FLAT);
	input_set_abs_params(idev, ABS_Z, -16384, 16383, INPUT_FUZZ,
			     INPUT_FLAT);

	printk("ParkMonitor input_register_polled_device  \n");
	result = input_register_polled_device(mir3da_idev);
	if (result) {
		MI_ERR("register poll device failed!\n");
		goto err_free_polled_device;
	}

	printk("ParkMonitor sysfs_create_group  \n");
	/* Sys Attribute Register */
	result = sysfs_create_group(&idev->dev.kobj, &mir3da_attr_group);
	if (result) {
		MI_ERR("create device file failed!\n");
		result = -EINVAL;
		goto err_unregister_polled_device;
	}

	printk("ParkMonitor misc_register  \n");
	/* Misc device interface Register */
	result = misc_register(&misc_mir3da);
	if (result) {
		MI_ERR("%s: mir3da_dev register failed", __func__);
		goto err_remove_sysfs_group;
	}
	/* slope_th = 0x10; */
	slope_th = G_SENSOR_SENSI_PARKING_H_VAL / GSENSOR_K + 1;

	/* gpio request */
	np = of_find_node_by_name(NULL, "gsensor_para");

	int_gpio = devm_gpiod_get_optional(&client->dev, "int", GPIOD_IN);


	if (IS_ERR_OR_NULL(int_gpio)) {
		MI_ERR("%s: int_gpio is invalid.\n", __func__);
	} else {
		printk("ParkMonitor int_gpio=%d  \n", desc_to_gpio(int_gpio));

		gpiod_direction_input(int_gpio);
		gsensor_irq = gpiod_to_irq(int_gpio);
		result = request_irq(gsensor_irq, gsensor_irq_func,
				     IRQF_SHARED |
					 /*IRQF_TRIGGER_FALLING |*/ IRQF_TRIGGER_RISING,
				     "int_gpio1", client);
		if (result) {
			MI_ERR("request_irq failed with error %d\n", result);
		} else {
#if 0 /* this method for linux-4.4*/
			enable_wakeup_src(CPUS_GPIO_SRC, int_gpio);
#else
			result = device_init_wakeup(&client->dev, 1);
			if (result < 0) {
				MI_ERR("device init wakeup failed\n");
				goto err_remove_sysfs_group;
			}

			result = dev_pm_set_wake_irq(&client->dev, gsensor_irq);
			printk("ParkMonitor enable_irq_wake result=%d \n", result);

			printk("ParkMonitor int_gpio=%d  gsensor_irq=%d  \n",
					 desc_to_gpio(int_gpio), gsensor_irq);
#endif
		}


	}
	initDA380Reg();


	printk("ParkMonitor init finish !!!  \n");

	return 0;

err_remove_sysfs_group:
	sysfs_remove_group(&idev->dev.kobj, &mir3da_attr_group);
err_unregister_polled_device:
	input_unregister_polled_device(mir3da_idev);
err_free_polled_device:
	input_free_polled_device(mir3da_idev);
err_detach_client:
	return result;
}
/*----------------------------------------------------------------------------*/
static int mir3da_remove(struct i2c_client *client)
{

/* if you want to wakeup chip by g-sensor, so dont need to disable here*/
	/*
	MIR_HANDLE handle = mir_handle;
	mir3da_set_enable(handle, 0);
	*/

	misc_deregister(&misc_mir3da);

	sysfs_remove_group(&mir3da_idev->input->dev.kobj, &mir3da_attr_group);

	input_unregister_polled_device(mir3da_idev);

	input_free_polled_device(mir3da_idev);

#ifdef MIR3DA_OFFSET_TEMP_SOLUTION
	flush_workqueue(m_work_info.wq);
	destroy_workqueue(m_work_info.wq);
#endif

	return 0;
}
/*----------------------------------------------------------------------------*/

#if 0
#ifdef CONFIG_PM
static int mir3da_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int result = 0;
	printk("mir3da_cust mir3da_suspend ************\n");
	/* MIR_HANDLE      handle = mir_handle; */
	/* MI_FUN; */
	/*
	result = mir3da_set_enable(handle, 0);
	if(result)
	{
		    MI_ERR("%s:set disable fail!!\n",__func__);
		    return result;
	}
	mir3da_idev->input->close(mir3da_idev->input);
	*/
	return result;
}

/*----------------------------------------------------------------------------*/
static int mir3da_resume(struct i2c_client *client)
{
	int result = 0;
	MIR_HANDLE handle = mir_handle;
	MI_FUN;

	printk("mir3da_cust mir3da_resume ************\n");

	result = mir3da_chip_resume(handle);
	if (result) {
		MI_ERR("%s:chip resume fail!!\n", __func__);
		return result;
	}

	result = mir3da_set_enable(handle, 1);
	if (result) {
		MI_ERR("%s:set enable fail!!\n", __func__);
		return result;
	}

	mir3da_idev->input->open(mir3da_idev->input);

	return result;
}
#endif
#endif

/*----------------------------------------------------------------------------*/
static int mir3da_detect(struct i2c_client *new_client,
			 struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = new_client->adapter;

	/* printk(" richard mir3da_detect: %s:bus[%d] addr[0x%x]\n", __func__, */
	/* adapter->nr, new_client->addr); */

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

		if (mir3da_install_general_ops(&ops_handle)) {
			MI_ERR("Install ops failed !\n");
			return -ENODEV;
		}
		if (mir3da_module_detect((PLAT_HANDLE)new_client)) {
			/*				printk("%s,%d\n",__FUNCTION__,__FILE__); */
			MI_ERR("Can't find Mir3da gsensor!!");
		} else {
			MI_ERR("'Find Mir3da gsensor!!");
			strlcpy(info->type, MIR3DA_DRV_NAME, I2C_NAME_SIZE);
			return 0;
		}

	return -ENODEV;
}
/*----------------------------------------------------------------------------*/
/* SIMPLE_DEV_PM_OPS(da380_gsensor_pm_ops, mir3da_suspend, mir3da_resume); */


/*----------------------------------------------------------------------------*/
static const struct i2c_device_id mir3da_id[] = {{MIR3DA_DRV_NAME, 0}, {} };

MODULE_DEVICE_TABLE(i2c, mir3da_id);
/*----------------------------------------------------------------------------*/
static struct i2c_driver mir3da_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
	    .name = MIR3DA_DRV_NAME, .owner = THIS_MODULE,
#ifdef CONFIG_PM
	    /* .pm = &da380_gsensor_pm_ops, */
#endif
	},

    .probe = mir3da_probe,
#ifdef CONFIG_PM
/*    .suspend = mir3da_suspend,
      .resume = mir3da_resume, */
#endif
    .detect = mir3da_detect,
    .remove = mir3da_remove,
    .id_table = mir3da_id,
    .address_list = normal_i2c,
};
/*----------------------------------------------------------------------------*/
static int __init mir3da_init(void)
{
	int ret;

	MI_FUN;

	ret = i2c_add_driver(&mir3da_driver);
	if (ret < 0) {
		MI_ERR("add mir3da i2c driver failed\n");
		return -ENODEV;
	}

	printk("%s i2c i2c_add_driver is %d \n", __func__, ret);

	return ret;
}
/*----------------------------------------------------------------------------*/
static void __exit mir3da_exit(void)
{
	MI_FUN;

	i2c_del_driver(&mir3da_driver);
}
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("MiraMEMS <lschen@miramems.com>");
MODULE_DESCRIPTION("MIR3DA 3-Axis Accelerometer driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

module_init(mir3da_init);
module_exit(mir3da_exit);
