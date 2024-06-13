/*
 * sound\soc\sunxi\sunxi-mad.c
 * (C) Copyright 2018-2023
 * AllWinner Technology Co., Ltd. <www.allwinnertech.com>
 * wolfgang <qinzhenying@allwinnertech.com>
 * yumingfeng <yumingfeng@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regmap.h>
#include <linux/input.h>
#include <sound/soc.h>
#include <linux/dma/sunxi-dma.h>
#include "sunxi-mad.h"
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/sock.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>

#include <linux/pm_wakeirq.h>
#include <linux/pm_domain.h>

#define	DRV_NAME	"sunxi-mad"

/****************************************************************************/

#ifdef SUNXI_MAD_NETLINK_USE
enum sunxi_mad_netlink_status {
	SNDRV_MAD_NETLINK_NULL = -1,
	SNDRV_MAD_NETLINK_CLOSE = 0,
	SNDRV_MAD_NETLINK_START = 1,
	SNDRV_MAD_NETLINK_PAUSE = 2,
	SNDRV_MAD_NETLINK_RESUME = 3,
	SNDRV_MAD_NETLINK_SUSPEND = 4,
};

struct mad_netlink_status {
	int pid;
	int status;
	struct mutex mutex_lock;
};

struct sunxi_mad_event {
	struct sock *sock;
	struct mad_netlink_status mad_netlink;
};

static struct sunxi_mad_event mad_event;

void sunxi_mad_netlink_send(struct sunxi_mad_event *mad_event, const char *fmt, ...);
#endif

/****************************************************************************/

/*memory mapping*/
#define MAD_SRAM_DMA_SRC_ADDR 0x05480000
/* 128k bytes */
#define MAD_SRAM_SIZE_VALUE 0x80

struct sunxi_mad_sram_size {
	unsigned int src_chan_num;
	unsigned int standby_chan_num; /* no used */
	unsigned int size;
	unsigned int sram_store_th;
	/* align dma block size */
	unsigned int ahb1_rx_th;
	unsigned int ahb1_tx_th;
};

/*
 * for alignment data when overflow.
 * Tips: sram dma width is 32bit.
 */
static const struct sunxi_mad_sram_size mad_sram_size[] = {
	{1, 1, 128, 64, 64, 64},
	{2, 2, 128, 64, 64, 64},
	{3, 3, 120, 60, 60, 60},	/* 120 % (3 * 2 * 2) == 0 */
	{4, 4, 128, 64, 64, 64},	/* 128 % (4 * 2) == 0 */
	{5, 5, 120, 60, 60, 60},	/* 120 % (5 * 2 * 2) == 0 */
	{6, 6, 120, 60, 60, 60},	/* 120 % (6 * 2) == 0 */
	{7, 7, 112, 56, 56, 56},	/* 112 % (7 * 2 * 2) == 0 */
	{8, 8, 128, 64, 64, 64},	/* 128 % (8 * 2) == 0 */
};

struct sunxi_mad_info *sunxi_mad;

/****************************************************************************/
/*
 * UNIT: audio frames
 * defalut_val:[0x5]
 */
static int sram_wake_back_da_val = 0x5;
module_param(sram_wake_back_da_val, int, 0644);
MODULE_PARM_DESC(sram_wake_back_da_val, "sunxi mad sram wakeup back debug");

/****************************************************************************/
/* lpsd ad sync frame */
static int lpsd_ad_sync = 0x20;
module_param(lpsd_ad_sync, int, 0644);
MODULE_PARM_DESC(lpsd_ad_sync, "sunxi mad lpsd ad sync debug");

/* lpsd th */
static int lpsd_th = 0x4b0;
module_param(lpsd_th, int, 0644);
MODULE_PARM_DESC(lpsd_th, "sunxi mad lpsd th debug(0x0-0xFFFF)");

/* speed deep on lpsd clk */
static int lpsd_rrun = 0x91;
module_param(lpsd_rrun, int, 0644);
MODULE_PARM_DESC(lpsd_rrun, "sunxi mad lpsd start run debug(0x0-0xFF)");

/* speed deep on lpsd clk */
static int lpsd_rstop = 0xaa;
module_param(lpsd_rstop, int, 0644);
MODULE_PARM_DESC(lpsd_rstop, "sunxi mad about lpsd stop run debug(0x0-0xFF)");

/* frames */
static int lpsd_ecnt = 2;//default: 0x32;
module_param(lpsd_ecnt, int, 0644);
MODULE_PARM_DESC(lpsd_ecnt, "sunxi mad about lpsd end count debug(0x0-0xFFFF)");

/****************************************************************************/
void sunxi_mad_lpsd_init(void)
{
	regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_WAKE_BACK_DATA,
		sram_wake_back_da_val);

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_LPSD_AD_SYNC_FC, lpsd_ad_sync);

	/*enable lpsd DC offset*/
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_LPSD_CH_MASK,
			0x1 << MAD_LPSD_DCBLOCK_EN,
			0x1 << MAD_LPSD_DCBLOCK_EN);

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_LPSD_TH, lpsd_th);

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_LPSD_RRUN, lpsd_rrun);

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_LPSD_RSTOP, lpsd_rstop);

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_LPSD_ECNT, lpsd_ecnt);
}
EXPORT_SYMBOL_GPL(sunxi_mad_lpsd_init);

void sunxi_sram_ahb1_threshole_init(void)
{
	int i = 0;

	/*config sunxi_mad_ahb1_rx_th_reg*/
	for (i = 0; i < ARRAY_SIZE(mad_sram_size); i++) {
		if (mad_sram_size[i].src_chan_num ==
				sunxi_mad->audio_src_chan_num) {
			regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_AHB1_RX_TH,
				mad_sram_size[i].ahb1_rx_th);
			pr_debug("[%s] MAD_SRAM_AHB1_RX_TH:%dkB\n",
					__func__, mad_sram_size[i].ahb1_rx_th);

			regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_AHB1_TX_TH,
				mad_sram_size[i].ahb1_tx_th);
			pr_debug("[%s] MAD_SRAM_AHB1_TX_TH:%dkB\n",
					__func__, mad_sram_size[i].ahb1_tx_th);
			break;
		}
	}
}
EXPORT_SYMBOL_GPL(sunxi_sram_ahb1_threshole_init);

void sunxi_mad_sram_set_reset_bit(void)
{
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			1 << SRAM_RST, 1 << SRAM_RST);
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			1 << SRAM_RST, 0 << SRAM_RST);
}
EXPORT_SYMBOL_GPL(sunxi_mad_sram_set_reset_bit);

int sunxi_mad_sram_set_reset_flag(enum sunxi_mad_sram_reset_flag reset_flag)
{
	sunxi_mad->sram_reset_flag = reset_flag;

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_sram_set_reset_flag);

enum sunxi_mad_sram_reset_flag sunxi_mad_sram_get_reset_flag(void)
{
	return sunxi_mad->sram_reset_flag;
}
EXPORT_SYMBOL_GPL(sunxi_mad_sram_get_reset_flag);

int sunxi_mad_sram_wait_reset_flag(enum sunxi_mad_sram_reset_flag reset_flag,
			unsigned int time_out_msecond)
{
	struct timeval old_tv;
	struct timeval new_tv;
	unsigned int new_msecond = 0;
	unsigned int old_msecond = 0;

	if (time_out_msecond < 1)
		time_out_msecond = 1;

	do_gettimeofday(&old_tv);
	old_msecond = old_tv.tv_usec/1000 + old_tv.tv_sec * 1000;

	while (sunxi_mad->sram_reset_flag != reset_flag) {
		do_gettimeofday(&new_tv);
		new_msecond = new_tv.tv_usec/1000 + new_tv.tv_sec * 1000;
		if (abs(new_msecond - old_msecond) > time_out_msecond)
			break;
		usleep_range(1000, 5000);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_sram_wait_reset_flag);

void sunxi_mad_sram_init(void)
{
	int i = 0;

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_POINT, 0x00);

	for (i = 0; i < ARRAY_SIZE(mad_sram_size); i++) {
		if (mad_sram_size[i].src_chan_num ==
				sunxi_mad->audio_src_chan_num) {
			regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_SIZE,
				mad_sram_size[i].size);
			pr_debug("[%s] MAD_SRAM_SIZE:%dkB\n",
					__func__, mad_sram_size[i].size);

			regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_STORE_TH,
				mad_sram_size[i].sram_store_th);
			pr_debug("[%s] MAD_SRAM_STORE_TH:%dkB\n",
				__func__, mad_sram_size[i].sram_store_th);
			break;
		}
	}

	/*config sunxi_mad_sram_sec_region_reg, non-sec*/
	regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_SEC_REGION_REG, 0x0);

	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			1 << SRAM_RST, 1 << SRAM_RST);
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			1 << SRAM_RST, 0 << SRAM_RST);

	sunxi_mad_sram_set_reset_flag(SUNXI_MAD_SRAM_RESET_IDLE);
}
EXPORT_SYMBOL_GPL(sunxi_mad_sram_init);

struct sunxi_mad_info *sunxi_mad_get_mad_info(void)
{
	return sunxi_mad;
}
EXPORT_SYMBOL_GPL(sunxi_mad_get_mad_info);

void sunxi_mad_clk_enable(bool val)
{
	if (val)
		clk_prepare_enable(sunxi_mad->mad_clk);
	else
		clk_disable_unprepare(sunxi_mad->mad_clk);
}
EXPORT_SYMBOL_GPL(sunxi_mad_clk_enable);

void sunxi_mad_ad_clk_enable(bool val)
{
	if (val)
		clk_prepare_enable(sunxi_mad->mad_ad_clk);
	else
		clk_disable_unprepare(sunxi_mad->mad_ad_clk);
}
EXPORT_SYMBOL_GPL(sunxi_mad_ad_clk_enable);

void sunxi_mad_cfg_clk_enable(bool val)
{
	if (val)
		clk_prepare_enable(sunxi_mad->mad_cfg_clk);
	else
		clk_disable_unprepare(sunxi_mad->mad_cfg_clk);
}
EXPORT_SYMBOL_GPL(sunxi_mad_cfg_clk_enable);

void sunxi_lpsd_clk_enable(bool val)
{
	if (val)
		clk_prepare_enable(sunxi_mad->lpsd_clk);
	else
		clk_disable_unprepare(sunxi_mad->lpsd_clk);
}
EXPORT_SYMBOL_GPL(sunxi_lpsd_clk_enable);

void sunxi_mad_module_clk_enable(bool val)
{
	if (val) {
		clk_prepare_enable(sunxi_mad->mad_clk);
		clk_prepare_enable(sunxi_mad->mad_ad_clk);
		clk_prepare_enable(sunxi_mad->mad_cfg_clk);
	} else {
		clk_disable_unprepare(sunxi_mad->mad_cfg_clk);
		clk_disable_unprepare(sunxi_mad->mad_ad_clk);
		clk_disable_unprepare(sunxi_mad->mad_clk);
	}
}
EXPORT_SYMBOL_GPL(sunxi_mad_module_clk_enable);

void sunxi_mad_standby_chan_sel(unsigned int num)
{
	pr_debug("[%s] standby_chan_sel: %d\n", __func__, num);
	sunxi_mad->standby_chan_sel = num;
}
EXPORT_SYMBOL_GPL(sunxi_mad_standby_chan_sel);

void sunxi_lpsd_chan_sel(unsigned int num)
{
	pr_debug("[%s] lpsd_chan_sel: %d\n", __func__, num);
	sunxi_mad->lpsd_chan_sel = num;
}
EXPORT_SYMBOL_GPL(sunxi_lpsd_chan_sel);

/*
 * should be called before the sunxi_mad_sram_init.
 */
void sunxi_mad_audio_src_chan_num(unsigned int num)
{
	pr_debug("[%s] audio_src_chan_num:%d\n", __func__, num);
	sunxi_mad->audio_src_chan_num = num;
}
EXPORT_SYMBOL_GPL(sunxi_mad_audio_src_chan_num);

#if 0
static void sunxi_mad_int_info_show(void)
{
	unsigned int val = 0;

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_CTRL, &val);
	pr_err("[%s] --> SUNXI_MAD_CTRL:0x%x\n", __func__, val);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_SRAM_CH_MASK, &val);
	pr_err("[%s] --> SUNXI_MAD_SRAM_CH_MASK:0x%x\n", __func__, val);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_LPSD_CH_MASK, &val);
	pr_err("[%s] --> SUNXI_MAD_LPSD_CH_MASK:0x%x\n", __func__, val);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, &val);
	pr_err("[%s] --> SUNXI_MAD_INT_ST_CLR:0x%x\n", __func__, val);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_INT_MASK, &val);
	pr_err("[%s] --> SUNXI_MAD_INT_MASK:0x%x\n", __func__, val);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_STA, &val);
	pr_err("[%s] SUNXI_MAD_STA:0x%x\n", __func__, val);
	if (((val >> MAD_STATE) & 0xF) == 0)
		pr_alert("[%s] MAD_STATE: ---> IDLE\n", __func__);
	else if ((val >> MAD_STATE) & 0x1)
		pr_alert("[%s] MAD_STATE: ---> WAIT\n", __func__);
	else if ((val >> (MAD_STATE + 1)) & 0x1)
		pr_alert("[%s] MAD_STATE: ---> RUN\n", __func__);
	else if ((val >> (MAD_STATE + 2)) & 0x1)
		pr_alert("[%s] MAD_STATE: ---> NORMAL\n", __func__);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_DEBUG, &val);
	pr_alert("[%s] SUNXI_MAD_DEBUG:0x%x\n", __func__, val);
	regmap_write(sunxi_mad->regmap, SUNXI_MAD_DEBUG, 0x7F);
}
#endif

void sunxi_mad_set_lpsdreq(bool enable)
{
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
		0x1 << MAD_REQ_INT_MASK, enable << MAD_REQ_INT_MASK);
}
EXPORT_SYMBOL_GPL(sunxi_mad_set_lpsdreq);

void sunxi_mad_set_datareq(bool enable)
{
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
		0x1 << DATA_REQ_INT_MASK, enable << DATA_REQ_INT_MASK);
}
EXPORT_SYMBOL_GPL(sunxi_mad_set_datareq);

#ifdef SUNXI_MAD_NETLINK_USE
static enum sunxi_mad_netlink_status sunxi_mad_set_netlink_status(
				struct sunxi_mad_event *mad_event,
				enum sunxi_mad_netlink_status netlink_status)
{
	if (!mad_event)
		return SNDRV_MAD_NETLINK_NULL;

	mad_event->mad_netlink.status = netlink_status;
	return mad_event->mad_netlink.status;
}

static enum sunxi_mad_netlink_status sunxi_mad_get_netlink_status(
				struct sunxi_mad_event *mad_event)
{
	if (!mad_event)
		return SNDRV_MAD_NETLINK_NULL;

	return mad_event->mad_netlink.status;
}
#endif

void sunxi_lpsd_int_stat_clr(void)
{
	unsigned int val = 0;
	/* must clear the wake req flag */
	regmap_read(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, &val);
	if (val & (0x1 << WAKE_INT)) {
		val &= ~(0x1 << DATA_REQ_INT);
		val |= (0x1 << WAKE_INT);
		regmap_write(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, val);
	}
}
EXPORT_SYMBOL_GPL(sunxi_lpsd_int_stat_clr);

void sunxi_mad_sram_chan_params(unsigned int mad_channels)
{
	unsigned int reg_val = 0;

	pr_debug("[%s]: mad_channels:%d\n", __func__, mad_channels);
	regmap_read(sunxi_mad->regmap, SUNXI_MAD_SRAM_CH_MASK, &reg_val);
	/*config mad sram receive audio channel num*/
	reg_val &= ~(0x1f << MAD_SRAM_CH_NUM);
	reg_val |= mad_channels << MAD_SRAM_CH_NUM;

	/* open mad sram receive channels */
	reg_val &= ~(0xffff << MAD_SRAM_CH_MASK);
	reg_val |= ((1 << mad_channels) - 1) << MAD_SRAM_CH_MASK;

	regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_CH_MASK, reg_val);
}
EXPORT_SYMBOL_GPL(sunxi_mad_sram_chan_params);

static int sunxi_mad_sram_chan_com_config(unsigned int audio_src_chan_num,
				unsigned int mad_standby_chan_sel, bool enable)
{
	unsigned int chan_ch = 0;
	unsigned int mad_sram_chan_num = 0;
	unsigned int temp_val = 0;

	/*transfer to mad_standby channels*/
	switch (mad_standby_chan_sel) {
	case 0:
		mad_sram_chan_num = audio_src_chan_num;
		break;
	case 1:
		mad_sram_chan_num = 2;
		break;
	case 2:
		mad_sram_chan_num = 4;
		break;
	default:
		mad_sram_chan_num = 2;
		break;
	}

	/* Read data at start */
	regmap_read(sunxi_mad->regmap, SUNXI_MAD_SRAM_CH_MASK, &temp_val);
	/* mad sram receive channels */
	temp_val &= ~(0x1F << MAD_SRAM_CH_NUM);
	temp_val &= ~(0xFFFF << MAD_SRAM_CH_MASK);
	if (enable) {
		temp_val |= audio_src_chan_num << MAD_SRAM_CH_NUM;
		temp_val |= ((1 << audio_src_chan_num) - 1) << MAD_SRAM_CH_MASK;
	} else {
		temp_val |= mad_sram_chan_num << MAD_SRAM_CH_NUM;
		temp_val |= ((1 << mad_sram_chan_num) - 1) << MAD_SRAM_CH_MASK;
	}

	/* config mad_sram channel change */
	if (mad_standby_chan_sel == 0) {
		chan_ch = MAD_CH_COM_NON;
	} else if (mad_standby_chan_sel == 1) {
		switch (audio_src_chan_num) {
		case 2:
			chan_ch = MAD_CH_COM_NON;
			break;
		case 4:
			chan_ch = MAD_CH_COM_2CH_TO_4CH;
			break;
		case 6:
			chan_ch = MAD_CH_COM_2CH_TO_6CH;
			break;
		case 8:
			chan_ch = MAD_CH_COM_2CH_TO_8CH;
			break;
		default:
			pr_err("unsupported mad_sram channels!\n");
			return -EINVAL;
		}
	} else if (mad_standby_chan_sel == 2) {
		switch (audio_src_chan_num) {
		case 4:
			chan_ch = MAD_CH_COM_NON;
			break;
		case 6:
			chan_ch = MAD_CH_COM_4CH_TO_6CH;
			break;
		case 8:
			chan_ch = MAD_CH_COM_4CH_TO_8CH;
			break;
		default:
			pr_err("unsupported mad_sram channels!\n");
			return -EINVAL;
		}
	} else {
		pr_err("mad_standby channels isn't set up!\n");
		return -EINVAL;
	}
	temp_val &= ~(0xF << MAD_CH_COM_NUM);
	temp_val |= chan_ch << MAD_CH_COM_NUM;

	/*
	 * Enable mad_sram channel CHANGE_EN
	 * when DMA interpolation process finish, the CHANGE_EN bit will be set
	 * to 0 automaticallly.
	 */
	if (enable && (chan_ch != MAD_CH_COM_NON))
		temp_val |= 0x1 << MAD_CH_CHANGE_EN;
	else
		temp_val &= ~(0x1 << MAD_CH_CHANGE_EN);

	/* Write the value Once! */
	if (chan_ch != MAD_CH_COM_NON)
		regmap_write(sunxi_mad->regmap, SUNXI_MAD_SRAM_CH_MASK, temp_val);

#ifdef CONFIG_SUNXI_AUDIO_DEBUG
	temp_val = 0;
	regmap_read(sunxi_mad->regmap, SUNXI_MAD_SRAM_PRE_DSIZE, &temp_val);
	snd_printd("SUNXI_MAD_SRAM_PRE_DSIZE:0x%x\n", temp_val);
#endif

	return 0;
}

static void sunxi_mad_lpsd_chan_enable(unsigned int lpsd_chan_sel, bool enable)
{
	unsigned int reg_val = 0;

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_LPSD_CH_MASK, &reg_val);

	reg_val &= ~(0xFFFF << MAD_LPSD_CH_MASK);
	reg_val &= ~(0x1 << MAD_LPSD_CH_NUM);
	if (enable) {
		/*transfer to mad_standby lpsd sel channels*/
		reg_val |= 1 << lpsd_chan_sel;
		/*config LPSD receive audio channel num: 1 channel*/
		reg_val |= 0x1 << MAD_LPSD_CH_NUM;
	}

	/*
	 * Tips:
	 *      The lpsd chan num and the channel mask should be setup
	 * at the same time.
	 */
	regmap_write(sunxi_mad->regmap, SUNXI_MAD_LPSD_CH_MASK, reg_val);
}

static void sunxi_mad_interrupt_status_clear(struct sunxi_mad_info *sunxi_mad)
{
	unsigned int reg_val = 0;

	for (;;) {
		regmap_read(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, &reg_val);
		if (reg_val & (0x1 << DATA_REQ_INT)) {
			/* clear data int state */
			reg_val &= ~(0x1 << WAKE_INT);
			reg_val |= (0x1 << DATA_REQ_INT);
			regmap_write(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, reg_val);
		} else
			break;
	}
}

static void sunxi_lpsd_interrupt_status_clear(struct sunxi_mad_info *sunxi_mad)
{
	unsigned int reg_val = 0;

	for (;;) {
		regmap_read(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, &reg_val);
		if (reg_val & (0x1 << WAKE_INT)) {
			/* clear data int state */
			reg_val &= ~(0x1 << DATA_REQ_INT);
			reg_val |= (0x1 << WAKE_INT);
			regmap_write(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, reg_val);
		} else
			break;
	}
}

static void sunxi_mad_work_resume(struct work_struct *work)
{
	struct sunxi_mad_info *sunxi_mad =
		container_of(work, struct sunxi_mad_info, ws_resume);
	unsigned int break_flag = 0;
	u32 reg_val = 0;

	spin_lock(&(sunxi_mad->resume_spin));

	sunxi_mad->status = SUNXI_MAD_RESUME;

	spin_unlock(&(sunxi_mad->resume_spin));

	snd_printk("[%s] Start.\n", __func__);

	/* disable the wake req */
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
			0x1 << MAD_REQ_INT_MASK, 0x0 << MAD_REQ_INT_MASK);

	sunxi_lpsd_interrupt_status_clear(sunxi_mad);

#ifdef SUNXI_MAD_NETLINK_USE
	if (sunxi_mad_get_netlink_status(mad_event) == SNDRV_MAD_NETLINK_SUSPEND) {
		sunxi_mad_netlink_send(&mad_event, "resume");
		sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_RESUME);
	}
#endif

	/* it maybe need 200-800ms */
	for (;;) {
/*
		regmap_read(sunxi_mad->regmap, SUNXI_MAD_STA, &reg_val);
		if (((reg_val & (0x3 << MAD_LPSD_STAT)) == 0) &&
			((break_flag & 0x1) == 0x0)) {
			sunxi_mad_lpsd_chan_enable(0, false);
			break_flag |= 0x1 << 0x0;
		}
*/
		regmap_read(sunxi_mad->regmap,
				SUNXI_MAD_SRAM_CH_MASK, &reg_val);
		reg_val = (reg_val >> MAD_CH_CHANGE_EN) & 0x1;
		if ((reg_val == 0) && ((break_flag & 0x2) != 0x2))
			break_flag |= 0x2;

//		if (break_flag == 0x3)
		if (break_flag & 0x2)
			break;
		usleep_range(5000, 10000);
	}

#ifdef CONFIG_SUNXI_AUDIO_DEBUG
	/* SUNXI_MAD_DMA_TF_SIZE[0x4C] */
	regmap_read(sunxi_mad->regmap, SUNXI_MAD_DMA_TF_SIZE, &reg_val);
	snd_printk("SUNXI_MAD_DMA_TF_SIZE:0x%x\n", reg_val);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_RD_SIZE, &reg_val);
	snd_printk("KEY_WORD_OK: MAD_RD_SIZE[0x%x]:0x%x.\n",
			SUNXI_MAD_RD_SIZE, reg_val);
#endif

	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			0x1 << CPUS_RD_DONE, 0x0 << CPUS_RD_DONE);

#ifndef SUNXI_LPSD_CLK_ALWAYS_ON
	sunxi_mad_lpsd_chan_enable(0, false);
	sunxi_lpsd_clk_enable(false);
#endif

	sunxi_mad_enable(false);

	if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_LPSD_IRQ)
		sunxi_mad->wakeup_flag &= ~SUNXI_MAD_WAKEUP_LPSD_IRQ;

	if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_MAD_IRQ)
		sunxi_mad->wakeup_flag &= ~SUNXI_MAD_WAKEUP_MAD_IRQ;

	snd_printk("[%s] Stop.\n", __func__);
}

#ifdef SUNXI_MAD_NETLINK_USE
#if 0
static void sunxi_suspend_delayed(struct work_struct *work)
{
	struct sunxi_mad_info *sunxi_mad =
		container_of(work, struct sunxi_mad_info, ws_suspend.work);
	sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_SUSPEND);
}
#endif
#endif

/* not to wakeup to system when time is too small. */
static int sunxi_mad_suspend_update_time(struct sunxi_mad_info *sunxi_mad)
{
	if (sunxi_mad == NULL) {
		pr_alert("[%s] sunxi_mad is NULL!!!\n", __func__);
		return -EINVAL;
	}
	do_gettimeofday(&(sunxi_mad->suspend_tv));

	return 0;
}

#if 0
static int sunxi_mad_suspend_get_time(struct sunxi_mad_info *sunxi_mad,
				struct timeval *time_val)
{
	if (sunxi_mad == NULL) {
		pr_alert("[%s] sunxi_mad is NULL!!!\n", __func__);
		return -EINVAL;
	}
	memcpy(time_val, &(sunxi_mad->suspend_tv), sizeof(struct timeval));

	return 0;
}

static bool sunxi_mad_lpsd_check_delay(struct sunxi_mad_info *sunxi_mad,
				unsigned int time_msecond)
{
	struct timeval old_tv;
	struct timeval new_tv;
	unsigned int new_msecond = 0;
	unsigned int old_msecond = 0;

	do_gettimeofday(&new_tv);
	sunxi_mad_suspend_get_time(sunxi_mad, &old_tv);

	if (time_msecond < 1)
		time_msecond = 1;

	new_msecond = new_tv.tv_usec/1000 + new_tv.tv_sec * 1000;
	old_msecond = old_tv.tv_usec/1000 + old_tv.tv_sec * 1000;
	if (abs(new_msecond - old_msecond) > time_msecond)
		return true;
	else
		return false;
}
#endif

static irqreturn_t sunxi_lpsd_interrupt(int irq, void *dev_id)
{
	struct sunxi_mad_info *sunxi_mad = dev_id;

	pr_debug("[%s] Start.\n", __func__);

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_SRAM_RD_POINT,
			&(sunxi_mad->sram_rd_point));

	sunxi_mad->wakeup_flag |= SUNXI_MAD_WAKEUP_LPSD_IRQ;

	/* disable the wake req */
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
			0x1 << MAD_REQ_INT_MASK, 0x0 << MAD_REQ_INT_MASK);

	sunxi_lpsd_interrupt_status_clear(sunxi_mad);

	/* FIXME:not use for wakeup */
	if (!(sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_USE)) {
		sunxi_lpsd_interrupt_status_clear(sunxi_mad);
		pr_alert("[%s] SUNXI_MAD_WAKEUP_USE is off.\n", __func__);
		return IRQ_HANDLED;
	}

	if (sunxi_mad->status != SUNXI_MAD_SUSPEND) {
		sunxi_lpsd_interrupt_status_clear(sunxi_mad);
		pr_alert("[%s] device was not be suspended!\n", __func__);
		return IRQ_HANDLED;
	}

	if (device_may_wakeup(sunxi_mad->dev)) {
		/*
		 * FIXME: wakeup interrupt when suspending.
		 * the app should setup cmd example:
		 * cat /sys/power/wakeup_count
		 * echo xxx > /sys/power/wakeup_count
		 */
		pm_stay_awake(sunxi_mad->dev);
		pm_wakeup_event(sunxi_mad->dev, 0);
		pm_relax(sunxi_mad->dev);
		pr_warn("[%s] SRAM_RD_POINT:0x%x.\n", __func__,
				sunxi_mad->sram_rd_point);
	} else {
		sunxi_lpsd_interrupt_status_clear(sunxi_mad);
		pr_warn("[%s] device was not wakeup.\n", __func__);
	}

	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
		0x1 << KEY_WORD_OK, 0x1 << KEY_WORD_OK);

	if (sunxi_mad->suspend_flag & SUNXI_MAD_STANDBY_SRAM_MEM)
		sunxi_mad_dma_type(SUNXI_MAD_DMA_IO);

	sunxi_mad_sram_chan_com_config(sunxi_mad->audio_src_chan_num,
			sunxi_mad->standby_chan_sel, true);

#ifdef SUNXI_MAD_DATA_INT_USE
	/* enable the data req and wake req */
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
		0x1 << DATA_REQ_INT_MASK, 0x1 << DATA_REQ_INT_MASK);
#else
	schedule_work(&(sunxi_mad->ws_resume));
#endif

	pr_debug("[%s] Stop.\n", __func__);

	return IRQ_HANDLED;
}

static irqreturn_t sunxi_mad_interrupt(int irq, void *dev_id)
{
	struct sunxi_mad_info *sunxi_mad = dev_id;
	unsigned int val = 0;

	snd_printd("[%s] Start.\n", __func__);

	sunxi_mad->wakeup_flag |= SUNXI_MAD_WAKEUP_MAD_IRQ;

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_INT_ST_CLR, &val);
	if (val & (0x1 << DATA_REQ_INT)) {
		/* disable the data req and wake req */
		regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
			0x1 << DATA_REQ_INT_MASK, 0x0 << DATA_REQ_INT_MASK);

		schedule_work(&(sunxi_mad->ws_resume));

		/* clear data int state */
		sunxi_mad_interrupt_status_clear(sunxi_mad);
	}

	snd_printd("[%s] Stop.\n", __func__);

	return IRQ_HANDLED;
}

void sunxi_sram_dma_config(struct sunxi_dma_params *capture_dma_param)
{
	capture_dma_param->dma_addr = MAD_SRAM_DMA_SRC_ADDR;
	capture_dma_param->dma_drq_type_num = DRQSRC_MAD_RX;
}
EXPORT_SYMBOL_GPL(sunxi_sram_dma_config);

void sunxi_mad_dma_type(enum sunxi_mad_dma_type dma_type)
{
	/* config sunxi_mad_sram dma type should be before DMA_EN */
	switch (dma_type) {
	case SUNXI_MAD_DMA_MEM:
	default:
		regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
				0x1 << DMA_TYPE, 0x0 << DMA_TYPE);
		break;
	case SUNXI_MAD_DMA_IO:
		regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
				0x1 << DMA_TYPE, 0x1 << DMA_TYPE);
		break;
	}
}
EXPORT_SYMBOL_GPL(sunxi_mad_dma_type);

void sunxi_mad_dma_enable(bool enable)
{
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
		0x1 << DMA_EN, enable << DMA_EN);
}
EXPORT_SYMBOL_GPL(sunxi_mad_dma_enable);

int sunxi_mad_open(void)
{
	sunxi_mad->status = SUNXI_MAD_OPEN;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_open);

int sunxi_mad_enable(bool enable)
{
	u32 reg_val = 0;
	unsigned int mad_is_worked = 1; /*not work*/

	/*open MAD_EN*/
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			0x1 << MAD_EN, enable << MAD_EN);

	if (enable) {
		/* if mad is working well */
		regmap_read(sunxi_mad->regmap, SUNXI_MAD_STA, &reg_val);
		reg_val |= ~(0x1 << MAD_RUN);
		mad_is_worked = ~reg_val;
		if (mad_is_worked) {
			pr_alert("mad isn't working right!\n");
			return -EBUSY;
		}
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_enable);

void sunxi_mad_set_go_on_sleep(bool enable)
{
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			0x1 << GO_ON_SLEEP, enable << GO_ON_SLEEP);
}
EXPORT_SYMBOL_GPL(sunxi_mad_set_go_on_sleep);

int sunxi_mad_close(void)
{
	sunxi_mad_enable(false);
	/*
	 * when sram used for optee at suspend,
	 * mad should set the io type to memcpy.
	 */
	sunxi_mad_dma_type(SUNXI_MAD_DMA_MEM);

	sunxi_mad->audio_src_path = 0;
	sunxi_mad->audio_src_chan_num = 0;
	sunxi_mad->standby_chan_sel = 0;
	sunxi_mad->lpsd_chan_sel = 0;

	sunxi_mad->status = SUNXI_MAD_CLOSE;

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_close);

int sunxi_mad_hw_params(unsigned int mad_channels, unsigned int sample_rate)
{
	snd_printd("[%s] mad_channels: %d, sample_rate:%d\n", __func__,
		mad_channels, sample_rate);

	/*config mad sram audio source channel num*/
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_SRAM_CH_MASK,
		0x1f << MAD_AD_SRC_CH_NUM, mad_channels << MAD_AD_SRC_CH_NUM);

	sunxi_mad_sram_chan_params(mad_channels);

	/* keep lpsd running in 16kHz */
	if (sample_rate == 16000)
		regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			0x1 << AUDIO_DATA_SYNC_FRC, 0x0 << AUDIO_DATA_SYNC_FRC);
	else if (sample_rate == 48000)
		regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
			0x1 << AUDIO_DATA_SYNC_FRC, 0x1 << AUDIO_DATA_SYNC_FRC);
	else
		return -EINVAL;

	sunxi_mad->status = SUNXI_MAD_PARAMS;

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_hw_params);

int sunxi_mad_audio_source_sel(unsigned int path_sel, unsigned int enable)
{
	char *path_str = NULL;

	switch (path_sel) {
	case 0:
		path_str = "No-Audio";
	break;
	case 1:
		path_str = "I2S0-Input";
	break;
	case 2:
		path_str = "Codec-Input";
	break;
	case 3:
		path_str = "DMIC-Input";
	break;
	case 4:
		path_str = "I2S1-Input";
	break;
	case 5:
		path_str = "I2S2-Input";
	break;
	default:
		path_str = "Error-Input";
		return -EINVAL;
	break;
	}
	pr_warn("[%s] %s\n", __func__, path_str);

	if (enable) {
		if ((path_sel >= 1) && (path_sel <= 5)) {
			regmap_update_bits(sunxi_mad->regmap,
					SUNXI_MAD_AD_PATH_SEL,
					MAD_AD_PATH_SEL_MASK,
					path_sel << MAD_AD_PATH_SEL);
			sunxi_mad->audio_src_path = path_sel;
		} else
			return -EINVAL;
	} else {
		regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_AD_PATH_SEL,
			MAD_AD_PATH_SEL_MASK, 0 << MAD_AD_PATH_SEL);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_audio_source_sel);

#ifdef CONFIG_SUNXI_AUDIO_DEBUG
static void sunx_mad_show_all_regs(struct sunxi_mad_info *sunxi_mad)
{
	unsigned int reg_val[4] = {0};
	int reg_offset = 0;

	for (reg_offset = 0; reg_offset < 0x6c; reg_offset += 0x10) {
		regmap_read(sunxi_mad->regmap, reg_offset + 0x0, &reg_val[0]);
		regmap_read(sunxi_mad->regmap, reg_offset + 0x4, &reg_val[1]);
		regmap_read(sunxi_mad->regmap, reg_offset + 0x8, &reg_val[2]);
		regmap_read(sunxi_mad->regmap, reg_offset + 0xc, &reg_val[3]);
		pr_warn("[%s] 0x%x-0x%x:\t 0x%-8x\t 0x%-8x\t 0x%-8x\t 0x%-8x\n",
			__func__, reg_offset, reg_offset+0xc,
			reg_val[0], reg_val[1], reg_val[2], reg_val[3]);
	}
}
#endif

/*for ASOC cpu_dai*/
int sunxi_mad_suspend_external(void)
{
	if (sunxi_mad->status == SUNXI_MAD_SUSPEND) {
		pr_warn("[%s] sunxi mad has suspend!\n", __func__);
		return 0;
	}

	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_CTRL,
		0x1 << DMA_EN, 0x0 << DMA_EN);

	/*
	 * config sunxi_mad_sram as memory
	 * eg: because optee need use the sram.
	 * the sram should stop to data.
	 */
	if (sunxi_mad->suspend_flag & SUNXI_MAD_STANDBY_SRAM_MEM) {
		sunxi_mad_dma_type(SUNXI_MAD_DMA_MEM);
		sunxi_mad_sram_chan_params(0);
	} else
		sunxi_mad_sram_chan_com_config(sunxi_mad->audio_src_chan_num,
			sunxi_mad->standby_chan_sel, false);

#ifndef SUNXI_LPSD_CLK_ALWAYS_ON
	sunxi_lpsd_clk_enable(true);
#endif

	sunxi_lpsd_interrupt_status_clear(sunxi_mad);

	sunxi_mad_lpsd_init();
	sunxi_mad_lpsd_chan_enable(sunxi_mad->lpsd_chan_sel, true);

	sunxi_lpsd_interrupt_status_clear(sunxi_mad);

	/* disable the data req and enable wake req */
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
			0x1 << DATA_REQ_INT_MASK, 0x0 << DATA_REQ_INT_MASK);
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
			0x1 << MAD_REQ_INT_MASK, 0x1 << MAD_REQ_INT_MASK);

	sunxi_mad_enable(true);
	sunxi_mad_set_go_on_sleep(true);

	sunxi_mad->status = SUNXI_MAD_SUSPEND;

#ifdef SUNXI_MAD_NETLINK_USE
#ifdef SUNXI_MAD_NETLINK_APP_PAUSE_USE
	/* need to notify app the suspend status. */
	sunxi_mad_netlink_send(&mad_event, "suspend");
#endif
	sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_SUSPEND);
#endif

	sunxi_mad_suspend_update_time(sunxi_mad);


#ifdef CONFIG_SUNXI_AUDIO_DEBUG
	sunx_mad_show_all_regs(sunxi_mad);
#endif

	snd_printk("[%s] sunxi mad is working right!\n", __func__);

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_suspend_external);

/*for ASOC cpu_dai*/
int sunxi_mad_resume_external(void)
{
#ifdef CONFIG_SUNXI_AUDIO_DEBUG
	unsigned int reg_val = 0;
#endif
	snd_printd("[%s] Start.\n", __func__);

	/* disable the wake req */
	regmap_update_bits(sunxi_mad->regmap, SUNXI_MAD_INT_MASK,
			0x1 << MAD_REQ_INT_MASK, 0x0 << MAD_REQ_INT_MASK);

	if ((sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_LPSD_IRQ) ||
		(sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_MAD_IRQ)) {
		pr_alert("[%s] sunxi mad has wakeup irq!\n", __func__);
		return 0;
	}

	spin_lock(&(sunxi_mad->resume_spin));

	if (sunxi_mad->status == SUNXI_MAD_RESUME) {
		pr_warn("[%s] sunxi mad has resume!\n", __func__);
		spin_unlock(&(sunxi_mad->resume_spin));
		return 0;
	}
	sunxi_mad->status = SUNXI_MAD_RESUME;

	spin_unlock(&(sunxi_mad->resume_spin));

	/* not a lpsd interrupt resume */
	if (sunxi_mad->suspend_flag & SUNXI_MAD_STANDBY_SRAM_MEM)
		sunxi_mad_dma_type(SUNXI_MAD_DMA_IO);

	sunxi_mad_sram_chan_com_config(sunxi_mad->audio_src_chan_num,
			sunxi_mad->standby_chan_sel, true);

	schedule_work(&(sunxi_mad->ws_resume));;

	snd_printd("[%s] Stop.\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_mad_resume_external);

static void sunxi_mad_sram_set_bmode(bool mode)
{
	unsigned int reg = 0;
	void __iomem *sram_bmode_ctrl_reg = ioremap(SRAM_BMODE_CTRL_REG, 4);

	reg = readl(sram_bmode_ctrl_reg);
	if (mode)
		reg |= (0x1 << MAD_SRAM_BMODE_CTRL);
	else
		reg &= ~(0x1 << MAD_SRAM_BMODE_CTRL);
	writel(reg, sram_bmode_ctrl_reg);

	iounmap(sram_bmode_ctrl_reg);
}

/*for internal use*/
static int sunxi_mad_suspend(struct device *dev)
{
	/*
	 * the standb system driver will set it to BOOT Mode
	 * befor system suspended if the ddr will be setup refresh.
	 */
	//sunxi_mad_sram_set_bmode(MAD_SRAM_BMODE_BOOT);
	return 0;
}

/*for internal use*/
static int sunxi_mad_resume(struct device *dev)
{
	sunxi_mad_sram_set_bmode(MAD_SRAM_BMODE_NORMAL);
	return 0;
}

static const struct regmap_config sunxi_mad_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = SUNXI_MAD_DEBUG,
	.cache_type = REGCACHE_NONE,
};

#ifdef SUNXI_MAD_NETLINK_USE
static int mad_netlink_send(struct sunxi_mad_event *mad_event,
			struct sock *sock, int group,
			u16 type, void *msg, int len)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	int ret = 0;

	skb = nlmsg_new(len, GFP_ATOMIC);
	if (!skb) {
		kfree_skb(skb);
		return -EMSGSIZE;
	}

	nlh = nlmsg_put(skb, 0, 0, type, len, 0);
	if (!nlh) {
		kfree_skb(skb);
		return -EMSGSIZE;
	}
	memcpy(nlmsg_data(nlh), msg, len);

	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;

	if (sunxi_mad_get_netlink_status(mad_event) > SNDRV_MAD_NETLINK_CLOSE) {
		ret = netlink_unicast(sock, skb, mad_event->mad_netlink.pid, GFP_ATOMIC);
		if (ret < 0)
			sunxi_mad_set_netlink_status(mad_event, SNDRV_MAD_NETLINK_CLOSE);
	}
	return ret;
}

static void mad_netlink_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	int pid;

	nlh = (struct nlmsghdr *)skb->data;
	pid = nlh->nlmsg_pid; /*pid of sending process */

	/* user start capture? */
	if (!strncmp(nlmsg_data(nlh), "start", 5)) {
		mad_event.mad_netlink.pid = pid;
		sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_START);
	}

#ifdef SUNXI_MAD_NETLINK_APP_PAUSE_USE
	if (!strncmp(nlmsg_data(nlh), "pause", 5)) {
		sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_PAUSE);
		/* for app setup the mad working!!!!!!*/
		sunxi_mad_suspend_external();
	}
#endif

#ifdef SUNXI_MAD_NETLINK_APP_RESUME_USE
	if (!strncmp(nlmsg_data(nlh), "resume", 6))
		sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_RESUME);
#endif

	if (!strncmp(nlmsg_data(nlh), "close", 5))
		sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_CLOSE);
}

void sunxi_mad_netlink_send(struct sunxi_mad_event *mad_event,
			const char *fmt, ...)
{
	va_list args;

	char evt_data[EVT_MAX_SIZE];
	int size;

	va_start(args, fmt);
	size = vscnprintf(evt_data, EVT_MAX_SIZE, fmt, args);
	/* mark and strip a trailing newline */
	if (size && evt_data[size - 1] == '\n')
		size--;
	va_end(args);
	mad_netlink_send(mad_event, mad_event->sock, 0, 0, evt_data, size);
}
#endif

/*
 * Connector properties
 */
static ssize_t mad_standby_sram_type_store(struct device *device,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	int ret = 0;
	int new_val = 0;

	ret = sscanf(buf, "%d", &new_val);
	pr_warn("\nret:%d, new_val:%d, mad standby SRAM IO Mode: %s\n", ret,
		new_val, sunxi_mad->standby_sram_type?"Memory mode":"IO mode");

	if (!(new_val == 0 || new_val == 1)) {
		pr_err("\nOnly 1 or 0 can be support.\n");
		pr_err("0: for memory type.\n");
		pr_err("1: for IO type.\n");
		return count;
	}

	if (new_val == sunxi_mad->standby_sram_type) {
		pr_err("new status no change.\n");
		return count;
	}

	sunxi_mad->standby_sram_type = new_val;
	if (new_val)
		sunxi_mad->suspend_flag |= SUNXI_MAD_STANDBY_SRAM_MEM;
	else
		sunxi_mad->suspend_flag &= ~SUNXI_MAD_STANDBY_SRAM_MEM;

	return count;
}

static ssize_t mad_standby_sram_type_show(struct device *device,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", sunxi_mad->standby_sram_type);
}

DEVICE_ATTR(standby_sram_mem, 0644,
	    mad_standby_sram_type_show, mad_standby_sram_type_store);

static ssize_t wakeup_irq_switch_store(struct device *device,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	int ret = 0;
	int new_val = 0;

	ret = sscanf(buf, "%d", &new_val);
	pr_warn("\nret:%d, new_val:%d, wakeup_irq_en: %d\n",
		ret, new_val, sunxi_mad->wakeup_irq_en);

	if (!(new_val == 0 || new_val == 1)) {
		pr_err("\nOnly 1 or 0 can be support.\n");
		return count;
	}

	if (new_val == sunxi_mad->wakeup_irq_en) {
		pr_err("new status no change.\n");
		return count;
	}

	if (!(sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_ON)) {
		pr_err("wakeup-source not be setup at dts file!");
		return count;
	}

	sunxi_mad->wakeup_irq_en = new_val;
	if (new_val) {
		if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_USE) {
			pr_err("has setup the wakeup irq for wakeup source!");
			return count;
		}
		ret = dev_pm_set_wake_irq(device, sunxi_mad->lpsd_irq);
		if (ret < 0) {
			dev_err(device, "failed to setup sunxi_mad dev wakeup irq.\n");
			return count;
		}
		sunxi_mad->wakeup_flag |= SUNXI_MAD_WAKEUP_USE;
	} else {
		if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_USE) {
			dev_pm_clear_wake_irq(device);
			sunxi_mad->wakeup_flag &= ~SUNXI_MAD_WAKEUP_USE;
		}
	}
	return count;
}

static ssize_t wakeup_irq_switch_show(struct device *device,
			   struct device_attribute *attr,
			   char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", sunxi_mad->wakeup_irq_en);
}

DEVICE_ATTR(wakeup_en, 0644, wakeup_irq_switch_show, wakeup_irq_switch_store);

static ssize_t sunxi_mad_lpsd_status_store(struct device *device,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	return count;
}

static ssize_t sunxi_mad_lpsd_status_show(struct device *device,
				struct device_attribute *attr, char *buf)
{
	unsigned int lpsd_status = 0;
	unsigned int reg_val = 0;

	regmap_read(sunxi_mad->regmap, SUNXI_MAD_STA, &reg_val);
	lpsd_status = (reg_val >> MAD_LPSD_STAT) & 0x3;

	return snprintf(buf, PAGE_SIZE, "%u\n", lpsd_status);
}

DEVICE_ATTR(lpsd_status, 0644, sunxi_mad_lpsd_status_show,
			sunxi_mad_lpsd_status_store);

static struct attribute *audio_mad_debug_attrs[] = {
	&dev_attr_standby_sram_mem.attr,
	&dev_attr_wakeup_en.attr,
	&dev_attr_lpsd_status.attr,
	NULL,
};

static struct attribute_group audio_mad_debug_attr_group = {
	.name   = "sunxi_mad_audio",
	.attrs  = audio_mad_debug_attrs,
};

#ifdef SUNXI_MAD_NETLINK_USE
int sunxi_mad_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = mad_netlink_rcv,
	};

	mad_event.sock = netlink_kernel_create(&init_net,
					SUNXI_NETLINK_MAD, &cfg);
	if (!mad_event.sock) {
		pr_err("[%s] netlink create failed.\n", __func__);
		return -EMSGSIZE;
	}
	sunxi_mad_set_netlink_status(&mad_event, SNDRV_MAD_NETLINK_CLOSE);

	if (mad_event.sock) {
		pr_warn("[%s] Creating MAD netlink successfully.\n", __func__);
		return 0;
	}

	pr_err("Creating MAD netlink is failed\n");
	return -EBUSY;
}

static void sunxi_mad_netlink_exit(void)
{
	if (mad_event.sock) {
		netlink_kernel_release(mad_event.sock);
		mad_event.sock = NULL;
	}
}
#endif

static int sunxi_mad_probe(struct platform_device *pdev)
{
	struct resource res, *memregion;
	struct device_node *np = pdev->dev.of_node;
	unsigned int temp_val;
	int ret;

	sunxi_mad = devm_kzalloc(&pdev->dev, sizeof(struct sunxi_mad_info),
				 GFP_KERNEL);
	if (!sunxi_mad) {
		ret = -ENOMEM;
		goto err_node_put;
	}

	dev_set_drvdata(&pdev->dev, sunxi_mad);
	sunxi_mad->dev = &pdev->dev;

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get sunxi mad resource\n");
		return -EINVAL;
		goto err_devm_kfree;
	}

	memregion = devm_request_mem_region(&pdev->dev, res.start,
					    resource_size(&res), DRV_NAME);
	if (!memregion) {
		dev_err(&pdev->dev, "sunxi mad memory region already claimed\n");
		ret = -EBUSY;
		goto err_devm_kfree;
	}

	sunxi_mad->membase = devm_ioremap(&pdev->dev,
					res.start, resource_size(&res));
	if (!sunxi_mad->membase) {
		dev_err(&pdev->dev, "sunxi mad ioremap failed\n");
		ret = -EBUSY;
		goto err_devm_kfree;
	}

	sunxi_mad->regmap = devm_regmap_init_mmio(&pdev->dev,
			sunxi_mad->membase, &sunxi_mad_regmap_config);
	if (IS_ERR_OR_NULL(sunxi_mad->regmap)) {
		dev_err(&pdev->dev, "sunxi mad registers regmap failed\n");
		ret = -ENOMEM;
		goto err_iounmap;
	}

	sunxi_mad_sram_set_bmode(MAD_SRAM_BMODE_NORMAL);

	ret = of_property_read_u32(np, "lpsd_clk_src_cfg", &temp_val);
	if (ret < 0) {
		pr_debug("Default LPSD clk source is 24 MHz hosc!\n");
		sunxi_mad->pll_audio_src_used = 0;
		sunxi_mad->hosc_src_used = 1;
	} else if (temp_val == 0) {
		pr_debug("lpsd clk source is pll_audio, 48 div!\n");
		sunxi_mad->pll_audio_src_used = 1;
		sunxi_mad->hosc_src_used = 0;
	} else if (temp_val == 1) {
		pr_debug("sunxi lpsd clk source is 24 MHz hosc!\n");
		sunxi_mad->pll_audio_src_used = 0;
		sunxi_mad->hosc_src_used = 1;
	}

	if (sunxi_mad->pll_audio_src_used == 1) {
		sunxi_mad->pll_clk = of_clk_get(np, 0);
		if (IS_ERR(sunxi_mad->pll_clk)) {
			dev_err(&pdev->dev, "Can't get pll clocks\n");
			ret = PTR_ERR(sunxi_mad->pll_clk);
			goto err_iounmap;
		}
	} else if (sunxi_mad->hosc_src_used == 1) {
		sunxi_mad->hosc_clk = of_clk_get(np, 1);
		if (IS_ERR(sunxi_mad->hosc_clk)) {
			dev_err(&pdev->dev, "Can't get 24MHz hosc clocks\n");
			ret = PTR_ERR(sunxi_mad->hosc_clk);
			goto err_iounmap;
		}
	} else {
		dev_err(&pdev->dev, "Can't set parent of lpsd_clk.\n");
		ret = -EBUSY;
		goto err_iounmap;
	}

	sunxi_mad->lpsd_clk = of_clk_get(np, 2);
	if (IS_ERR(sunxi_mad->lpsd_clk)) {
		dev_err(&pdev->dev, "Can't get lpsd clocks\n");
		ret = PTR_ERR(sunxi_mad->lpsd_clk);
		goto err_lpsd_src_clk_put;
	} else if (sunxi_mad->pll_audio_src_used == 1) {
		if (clk_set_parent(sunxi_mad->lpsd_clk, sunxi_mad->pll_clk)) {
			dev_err(&pdev->dev, "set parent of lpsd_clk to pll_clk fail\n");
			ret = -EBUSY;
			goto err_lpsd_clk_put;
		} else if (clk_prepare_enable(sunxi_mad->pll_clk)) {
			dev_err(&pdev->dev, "Can't prepare enable pll_clk\n");
			goto err_lpsd_clk_put;
		}
	} else if (sunxi_mad->hosc_src_used == 1) {
		if (clk_set_parent(sunxi_mad->lpsd_clk, sunxi_mad->hosc_clk)) {
			dev_err(&pdev->dev, "set parent of lpsd_clk to hosc_clk fail\n");
			ret = -EBUSY;
			goto err_lpsd_clk_put;
		} else if (clk_prepare_enable(sunxi_mad->hosc_clk)) {
			dev_err(&pdev->dev, "Can't prepare enable hosc_clk\n");
			goto err_lpsd_clk_put;
		}

	}

#ifdef SUNXI_LPSD_CLK_ALWAYS_ON
	if (clk_prepare_enable(sunxi_mad->lpsd_clk)) {
		dev_err(&pdev->dev, "Can't prepare enable lpsd_clk\n");
		ret = -EBUSY;
		goto err_lpsd_src_clk_disable;
	}
#endif

	sunxi_mad->mad_clk = of_clk_get(np, 3);
	if (IS_ERR(sunxi_mad->mad_clk)) {
		dev_err(&pdev->dev, "Can't get mad clocks\n");
		ret = PTR_ERR(sunxi_mad->mad_clk);
		goto err_lpsd_clk_disable;
	}
#ifdef MAD_CLK_ALWAYS_ON
	if (clk_prepare_enable(sunxi_mad->mad_clk)) {
		dev_err(&pdev->dev, "Can't prepare enable mad_clk\n");
		goto err_mad_clk_put;
	}
#endif

	sunxi_mad->mad_ad_clk = of_clk_get(np, 4);
	if (IS_ERR(sunxi_mad->mad_ad_clk)) {
		dev_err(&pdev->dev, "Can't get mad ad clocks\n");
		ret = PTR_ERR(sunxi_mad->mad_ad_clk);
		goto err_mad_clk_disable;
	}
#ifdef MAD_CLK_ALWAYS_ON
	if (clk_prepare_enable(sunxi_mad->mad_ad_clk)) {
		dev_err(&pdev->dev, "Can't prepare enable mad_ad_clk\n");
		goto err_mad_ad_clk_put;
	}
#endif

	sunxi_mad->mad_cfg_clk = of_clk_get(np, 5);
	if (IS_ERR(sunxi_mad->mad_cfg_clk)) {
		dev_err(&pdev->dev, "Can't get mad cfg clocks\n");
		ret = PTR_ERR(sunxi_mad->mad_cfg_clk);
		goto err_mad_ad_clk_disable;
	}
#ifdef MAD_CLK_ALWAYS_ON
	if (clk_prepare_enable(sunxi_mad->mad_cfg_clk)) {
		dev_err(&pdev->dev, "Can't prepare enable mad_cfg_clk\n");
		goto err_mad_cfg_clk_put;
	}
#endif

	sunxi_mad->lpsd_irq = platform_get_irq(pdev, 0);
	if (sunxi_mad->lpsd_irq < 0) {
		dev_err(&pdev->dev, "Can't get lpsd irq.\n");
		goto err_mad_cfg_clk_disable;
	}

	/* IRQF_NO_SUSPEND */
	ret = devm_request_irq(sunxi_mad->dev, sunxi_mad->lpsd_irq,
				sunxi_lpsd_interrupt, 0, "lpsd-irq", sunxi_mad);
	if (ret < 0) {
		dev_err(&pdev->dev, "lpsd irq request failed.\n");
		goto err_mad_cfg_clk_disable;
	}

	if (of_get_property(np, "wakeup-source", NULL)) {
		ret = device_init_wakeup(sunxi_mad->dev, true);
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to init sunxi_mad dev wakeup.\n");
			goto err_lpsd_irq_free;
		}

		if (!device_can_wakeup(sunxi_mad->dev)) {
			dev_err(&pdev->dev, "it's not a wakup device.\n");
			goto err_dev_init_wakeup;
		}
		sunxi_mad->wakeup_flag |= SUNXI_MAD_WAKEUP_ON;

		ret = dev_pm_set_wake_irq(sunxi_mad->dev, sunxi_mad->lpsd_irq);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to setup sunxi_mad dev wakeup irq.\n");
			goto err_dev_init_wakeup;
		}
		sunxi_mad->wakeup_irq_en = 1;
		sunxi_mad->wakeup_flag |= SUNXI_MAD_WAKEUP_USE;
	} else {
		dev_warn(&pdev->dev, "can't find sunxi_mad dev wakeup-source.\n");
		sunxi_mad->wakeup_irq_en = 0;
		sunxi_mad->wakeup_flag &= ~SUNXI_MAD_WAKEUP_ON;
	}

	sunxi_mad->mad_irq = platform_get_irq(pdev, 1);
	if (sunxi_mad->mad_irq < 0) {
		pr_err("[audio] sunxi mad data get irq failed!\n");
		goto err_dev_wakeup_irq;
	}

	ret = devm_request_irq(sunxi_mad->dev, sunxi_mad->mad_irq,
			sunxi_mad_interrupt, 0, "mad-irq", sunxi_mad);
	if (ret < 0) {
		pr_err("[audio] sunxi mad data irq request failed!\n");
		goto err_dev_wakeup_irq;
	}

#ifdef SUNXI_MAD_NETLINK_USE
	ret = sunxi_mad_netlink_init();
	if (ret < 0) {
		pr_err("[audio] sunxi mad data irq request failed!\n");
		goto err_mad_irq_free;
	}
#endif

	ret = of_property_read_u32(np, "standby_sram_io_type", &temp_val);
	if (ret < 0) {
		pr_alert("Default sram io type is memory\n");
		sunxi_mad->standby_sram_type = 1;
		sunxi_mad->suspend_flag |= SUNXI_MAD_STANDBY_SRAM_MEM;
	} else if (temp_val == 1) {
		sunxi_mad->suspend_flag &= ~SUNXI_MAD_STANDBY_SRAM_MEM;
		sunxi_mad->standby_sram_type = 0;
		pr_warn("SRAM IO type is IO when system standby.\n");
	} else {
		sunxi_mad->suspend_flag |= SUNXI_MAD_STANDBY_SRAM_MEM;
		sunxi_mad->standby_sram_type = 1;
		pr_warn("SRAM IO type is memory when system standby.\n");
	}

	INIT_WORK(&(sunxi_mad->ws_resume), sunxi_mad_work_resume);
	spin_lock_init(&(sunxi_mad->resume_spin));

	ret  = sysfs_create_group(&pdev->dev.kobj, &audio_mad_debug_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "failed to create mad attr group.\n");
#ifdef SUNXI_MAD_NETLINK_USE
		goto err_mad_netlink_exit;
#else
		goto err_mad_irq_free;
#endif
	}

	pr_debug("[audio] sunxi mad probe succeed!\n");

	return 0;

#ifdef SUNXI_MAD_NETLINK_USE
err_mad_netlink_exit:
	sunxi_mad_netlink_exit();
#endif
err_mad_irq_free:
	devm_free_irq(sunxi_mad->dev, sunxi_mad->mad_irq, sunxi_mad);
err_dev_wakeup_irq:
	if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_USE)
		dev_pm_clear_wake_irq(&pdev->dev);
err_dev_init_wakeup:
	if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_ON)
		device_init_wakeup(&pdev->dev, false);

err_lpsd_irq_free:
	devm_free_irq(sunxi_mad->dev, sunxi_mad->lpsd_irq, sunxi_mad);
err_mad_cfg_clk_disable:
#ifdef MAD_CLK_ALWAYS_ON
	clk_disable_unprepare(sunxi_mad->mad_cfg_clk);
err_mad_cfg_clk_put:
#endif
	clk_put(sunxi_mad->mad_cfg_clk);
err_mad_ad_clk_disable:
#ifdef MAD_CLK_ALWAYS_ON
	clk_disable_unprepare(sunxi_mad->mad_ad_clk);
err_mad_ad_clk_put:
#endif
	clk_put(sunxi_mad->mad_ad_clk);
err_mad_clk_disable:
#ifdef MAD_CLK_ALWAYS_ON
	clk_disable_unprepare(sunxi_mad->mad_clk);
err_mad_clk_put:
#endif
	clk_put(sunxi_mad->mad_clk);
err_lpsd_clk_disable:
#ifdef SUNXI_LPSD_CLK_ALWAYS_ON
	clk_disable_unprepare(sunxi_mad->lpsd_clk);
err_lpsd_src_clk_disable:
#endif
	if (sunxi_mad->pll_audio_src_used == 1)
		clk_disable_unprepare(sunxi_mad->pll_clk);
	else if (sunxi_mad->hosc_src_used == 1)
		clk_disable_unprepare(sunxi_mad->hosc_clk);
err_lpsd_clk_put:
	clk_put(sunxi_mad->lpsd_clk);
err_lpsd_src_clk_put:
	if (sunxi_mad->pll_audio_src_used == 1)
		clk_put(sunxi_mad->pll_clk);
	else if (sunxi_mad->hosc_src_used == 1)
		clk_put(sunxi_mad->hosc_clk);
err_iounmap:
	iounmap(sunxi_mad->membase);
err_devm_kfree:
	devm_kfree(&pdev->dev, sunxi_mad);
err_node_put:
	of_node_put(np);
	return ret;
}

static int __exit sunxi_mad_remove(struct platform_device *pdev)
{
	struct sunxi_mad_info *sunxi_mad = dev_get_drvdata(&pdev->dev);

	cancel_work_sync(&(sunxi_mad->ws_resume));

	if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_USE)
		dev_pm_clear_wake_irq(&pdev->dev);

	if (sunxi_mad->wakeup_flag & SUNXI_MAD_WAKEUP_ON)
		device_init_wakeup(&pdev->dev, false);

	devm_free_irq(sunxi_mad->dev, sunxi_mad->mad_irq, sunxi_mad);
	devm_free_irq(sunxi_mad->dev, sunxi_mad->lpsd_irq, sunxi_mad);

#ifdef MAD_CLK_ALWAYS_ON
	clk_disable_unprepare(sunxi_mad->mad_clk);
	clk_disable_unprepare(sunxi_mad->mad_ad_clk);
	clk_disable_unprepare(sunxi_mad->mad_cfg_clk);
#endif
	clk_put(sunxi_mad->mad_clk);
	clk_put(sunxi_mad->mad_ad_clk);
	clk_put(sunxi_mad->mad_cfg_clk);

#ifdef SUNXI_LPSD_CLK_ALWAYS_ON
	clk_disable_unprepare(sunxi_mad->lpsd_clk);
#endif
	clk_put(sunxi_mad->lpsd_clk);
	if (sunxi_mad->pll_audio_src_used == 1) {
		clk_disable_unprepare(sunxi_mad->pll_clk);
		clk_put(sunxi_mad->pll_clk);
	} else if (sunxi_mad->hosc_src_used == 1) {
		clk_disable_unprepare(sunxi_mad->hosc_clk);
		clk_put(sunxi_mad->hosc_clk);
	}
#ifdef SUNXI_MAD_NETLINK_USE
	sunxi_mad_netlink_exit();
#endif
	iounmap(sunxi_mad->membase);
	devm_kfree(&pdev->dev, sunxi_mad);
	return 0;
}

static const struct dev_pm_ops sunxi_mad_pm_ops = {
	.suspend = sunxi_mad_suspend,
	.resume = sunxi_mad_resume,
};

static const struct of_device_id sunxi_mad_of_match[] = {
	{ .compatible = "allwinner,sunxi-mad", },
	{ },
};
MODULE_DEVICE_TABLE(of, sunxi_mad_of_match);

static struct platform_driver sunxi_mad_driver = {
	.probe = sunxi_mad_probe,
	.remove = __exit_p(sunxi_mad_remove),
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm	= &sunxi_mad_pm_ops,
		.of_match_table = sunxi_mad_of_match,
	},
};
module_platform_driver(sunxi_mad_driver);

MODULE_AUTHOR("qinzhenying <qinzhenying@allwinnertech.com>");
MODULE_DESCRIPTION("SUNXI MAD driver");
MODULE_ALIAS("platform:sunxi-mad");
MODULE_LICENSE("GPL");
