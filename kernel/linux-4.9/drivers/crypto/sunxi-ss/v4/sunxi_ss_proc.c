/*
 * The driver of SUNXI SecuritySystem controller.
 *
 * Copyright (C) 2018 Allwinner.
 *
 * <xupengliu@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/highmem.h>
#include <linux/dmaengine.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>
#include <crypto/des.h>

#include <crypto/aead.h>
#include <crypto/internal/aead.h>

#include "../sunxi_ss.h"
#include "../sunxi_ss_proc.h"
#include "sunxi_ss_reg.h"

void ce_print_task_desc(ce_task_desc_t *task)
{
	int i = 0;

#ifndef SUNXI_CE_DEBUG
		return;
#endif
	pr_err("---------------------task_info--------------------\n");
	pr_err("task->comm_ctl = 0x%x\n", task->comm_ctl);
	pr_err("task->sym_ctl = 0x%x\n", task->sym_ctl);
	pr_err("task->asym_ctl = 0x%x\n", task->asym_ctl);
	pr_err("task->key_addr = 0x%x\n", task->key_addr);
	pr_err("task->iv_addr = 0x%x\n", task->iv_addr);
	pr_err("task->ctr_addr = 0x%x\n", task->ctr_addr);
	pr_err("task->data_len = 0x%x\n", task->data_len);
	for (i = 0; i < 8; i++) {
		if (task->src[i].addr) {
			pr_err("task->src[%d].addr = 0x%x\n", i, task->src[i].addr);
			pr_err("task->src[%d].len = 0x%x\n", i, task->src[i].len);
		}
	}

	for (i = 0; i < 8; i++) {
		if (task->dst[i].addr) {
			pr_err("task->dst[%d].addr = 0x%x\n", i, task->dst[i].addr);
			pr_err("task->dst[%d].len = 0x%x\n", i, task->dst[i].len);
		}
	}
	pr_err("task->task_phy_addr = 0x%px\n", (void *)task->task_phy_addr);
}

/* ss_new_task_desc_init used for hash/rng alg */
void ss_new_task_desc_init(ce_new_task_desc_t *task, u32 flow)
{
	memset(task, 0, sizeof(ce_new_task_desc_t));

	task->common_ctl |= (flow << CE_CTL_CHAN_MASK);
	task->common_ctl |= CE_CTL_IE_MASK;
}

void ss_task_desc_init(ce_task_desc_t *task, u32 flow)
{
	memset(task, 0, sizeof(ce_task_desc_t));
	task->chan_id = flow;
	task->comm_ctl |= CE_COMM_CTL_TASK_INT_MASK;
}

static int ss_sg_len(struct scatterlist *sg, int total)
{
	int nbyte = 0;
	struct scatterlist *cur = sg;

	while (cur != NULL) {
		SS_DBG("cur: %p, len: %d, is_last: %ld\n",
			cur, cur->length, sg_is_last(cur));
		nbyte += cur->length;

		cur = sg_next(cur);
	}

	return nbyte;
}

static int ss_aes_align_size(int type, int mode)
{
	if ((type == SS_METHOD_ECC) || CE_METHOD_IS_HMAC(type)
		|| (CE_IS_AES_MODE(type, mode, CTS))
		|| (CE_IS_AES_MODE(type, mode, XTS)))
		return 4;
	else if ((type == SS_METHOD_DES) || (type == SS_METHOD_3DES))
		return DES_BLOCK_SIZE;
	else
		return AES_BLOCK_SIZE;
}

static int ss_copy_from_user(void *to, struct scatterlist *from, u32 size)
{
	void *vaddr = NULL;
	struct page *ppage = sg_page(from);

	vaddr = kmap(ppage);
	if (vaddr == NULL) {
		WARN(1, "Fail to map the last sg 0x%p (%d).\n", from, size);
		return -1;
	}

	SS_DBG("vaddr = %p, sg_addr = 0x%p, size = %d\n", vaddr, from, size);
	memcpy(to, vaddr + from->offset, size);
	kunmap(ppage);
	return 0;
}

static int ss_copy_to_user(struct scatterlist *to, void *from, u32 size)
{
	void *vaddr = NULL;
	struct page *ppage = sg_page(to);

	vaddr = kmap(ppage);
	if (vaddr == NULL) {
		WARN(1, "Fail to map the last sg: 0x%p (%d).\n", to, size);
		return -1;
	}

	SS_DBG("vaddr = %p, sg_addr = 0x%p, size = %d\n", vaddr, to, size);
	memcpy(vaddr+to->offset, from, size);
	kunmap(ppage);
	return 0;
}
static int ss_aead_sg_config(ce_scatter_t *scatter,
	ss_dma_info_t *info, int type, int mode, int tail)
{
	int cnt = 0;
	int last_sg_len = 0;
	struct scatterlist *cur = info->sg;

	while (cur != NULL) {
		if (cnt >= CE_SCATTERS_PER_TASK-1) {
			WARN(1, "Too many scatter: %d\n", cnt);
			return -1;
		}

		scatter[cnt+1].addr = sg_dma_address(cur) >> 2;	/* address in words*/
		scatter[cnt+1].len = sg_dma_len(cur)/4;
		info->last_sg = cur;
		last_sg_len = sg_dma_len(cur);
		SS_DBG("%d cur: 0x%p, scatter: addr 0x%x, len %d (%d)\n",
				cnt, cur, scatter[cnt+1].addr,
				scatter[cnt+1].len, sg_dma_len(cur));
		cnt++;
		cur = sg_next(cur);
	}

	info->nents = cnt;
	if (tail == 0) {
		info->has_padding = 0;
		return 0;
	}

	/* CTS/CTR/CFB/OFB need algin with word/block, so replace the last sg.*/

	last_sg_len += ss_aes_align_size(0, mode) - tail;
	info->padding = kzalloc(last_sg_len, GFP_KERNEL);
	if (info->padding == NULL) {
		SS_ERR("Failed to kmalloc(%d)!\n", last_sg_len);
		return -ENOMEM;
	}
	SS_DBG("AES(%d)-%d padding: 0x%p, tail = %d/%d, cnt = %d\n",
		type, mode, info->padding, tail, last_sg_len, cnt);

	scatter[cnt].addr = virt_to_phys(info->padding) >> 2;	/* address in words*/
	ss_copy_from_user(info->padding,
		info->last_sg, last_sg_len - ss_aes_align_size(0, mode) + tail);
	scatter[cnt].len = last_sg_len/4;

	info->has_padding = 1;
	return 0;
}

static int ss_sg_config(ce_scatter_t *scatter,
	ss_dma_info_t *info, int type, int mode, int tail, int hash_type)
{
	int cnt = 0;
	int last_sg_len = 0;
	struct scatterlist *cur = info->sg;

	while (cur != NULL) {
		if (cnt >= CE_SCATTERS_PER_TASK) {
			WARN(1, "Too many scatter: %d\n", cnt);
			return -1;
		}

		scatter[cnt].addr = sg_dma_address(cur) >> 2;/* address in words*/
		scatter[cnt].len = sg_dma_len(cur)/4;
		info->last_sg = cur;
		last_sg_len = sg_dma_len(cur);
		SS_DBG("%d cur: 0x%p, scatter: addr 0x%x, len %d (%d)\n",
				cnt, cur, scatter[cnt].addr,
				scatter[cnt].len, sg_dma_len(cur));
		cnt++;
		cur = sg_next(cur);
	}

#ifdef SS_HASH_HW_PADDING
		if (CE_METHOD_IS_HMAC(type)) {
			scatter[cnt-1].len += (tail+3)/4;
			info->has_padding = 0;
			return 0;
		}
#endif

	info->nents = cnt;
	if (tail == 0) {
		info->has_padding = 0;
		return 0;
	}

	if (hash_type) {
		scatter[cnt-1].len -= tail/4;
		return 0;
	}
	/* CTS/CTR/CFB/OFB need algin with word/block, so replace the last sg.*/

	last_sg_len += ss_aes_align_size(0, mode) - tail;
	info->padding = kzalloc(last_sg_len, GFP_KERNEL);
	if (info->padding == NULL) {
		SS_ERR("Failed to kmalloc(%d)!\n", last_sg_len);
		return -ENOMEM;
	}
	SS_DBG("AES(%d)-%d padding: 0x%p, tail = %d/%d, cnt = %d\n",
		type, mode, info->padding, tail, last_sg_len, cnt);

	scatter[cnt-1].addr = virt_to_phys(info->padding) >> 2;/* address in words*/
	ss_copy_from_user(info->padding,
		info->last_sg, last_sg_len - ss_aes_align_size(0, mode) + tail);
	scatter[cnt-1].len = last_sg_len/4;

	info->has_padding = 1;
	return 0;
}

static void ss_aes_unpadding(ce_scatter_t *scatter,
	ss_dma_info_t *info, int mode, int tail)
{
	int last_sg_len = 0;
	int index = info->nents - 1;

	if (info->has_padding == 0)
		return;

	/* Only the dst sg need to be recovered. */
	if (info->dir == DMA_DEV_TO_MEM) {
		last_sg_len = scatter[index].len * 4;
		last_sg_len -= ss_aes_align_size(0, mode) - tail;
		ss_copy_to_user(info->last_sg, info->padding, last_sg_len);
	}

	kfree(info->padding);
	info->padding = NULL;
	info->has_padding = 0;
}

static void ss_aes_map_padding(ce_scatter_t *scatter,
	ss_dma_info_t *info, int mode, int dir)
{
	int len = 0;
	int index = info->nents - 1;

	if (info->has_padding == 0)
		return;

	len = scatter[index].len * 4;
	SS_DBG("AES padding: 0x%x, len: %d, dir: %d\n",
		scatter[index].addr, len, dir);

	/* task address in words, so phys-to-virt need to change*/
	dma_map_single(&ss_dev->pdev->dev,
		phys_to_virt(scatter[index].addr << 2), len, dir);
	info->dir = dir;
}

static void ss_aead_map_padding(ce_scatter_t *scatter,
	ss_dma_info_t *info, int mode, int dir)
{
	int len = 0;
	int index = info->nents;

	if (info->has_padding == 0)
		return;

	len = scatter[index].len * 4;
	SS_DBG("AES padding: 0x%x, len: %d, dir: %d\n",
		scatter[index].addr, len, dir);

	/* task address in words, so phys-to-virt need to change*/
	dma_map_single(&ss_dev->pdev->dev,
		phys_to_virt(scatter[index].addr << 2), len, dir);
	info->dir = dir;
}

static void ss_aes_unmap_padding(ce_scatter_t *scatter,
	ss_dma_info_t *info, int mode, int dir)
{
	int len = 0;
	int index = info->nents - 1;

	if (info->has_padding == 0)
		return;

	len = scatter[index].len * 4;
	SS_DBG("AES padding: 0x%x, len: %d, dir: %d\n",
		scatter[index].addr, len, dir);
	dma_unmap_single(&ss_dev->pdev->dev, scatter[index].addr, len, dir);
}
static void ss_aead_unmap_padding(ce_scatter_t *scatter,
	ss_dma_info_t *info, int mode, int dir)
{
	int len = 0;
	int index = info->nents;

	if (info->has_padding == 0)
		return;

	len = scatter[index].len * 4;
	SS_DBG("AES padding: 0x%x, len: %d, dir: %d\n",
		scatter[index].addr, len, dir);
	dma_unmap_single(&ss_dev->pdev->dev, scatter[index].addr, len, dir);
}

void ss_change_clk(int type)
{
#ifdef SS_RSA_CLK_ENABLE
	if ((type == SS_METHOD_RSA) || (type == SS_METHOD_ECC))
		ss_clk_set(ss_dev->rsa_clkrate);
	else
		ss_clk_set(ss_dev->gen_clkrate);
#endif
}

void ss_hash_rng_change_clk(void)
{
#ifdef SS_RSA_CLK_ENABLE
		ss_clk_set(ss_dev->gen_clkrate);
#endif
}

static int ss_hmac_start(ss_aes_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx, int len)
{
	int ret = 0;
	int i = 0;
	int src_len = len;
	int align_size;
	int flow = ctx->comm.flow;
	phys_addr_t phy_addr = 0;
	ce_new_task_desc_t *task = (ce_new_task_desc_t *)&ss_dev->flows[flow].task;

	ss_hash_rng_change_clk();
	ss_new_task_desc_init(task, flow);

	ss_pending_clear(flow);
	ss_irq_enable(flow);

	if (CE_METHOD_IS_HMAC(req_ctx->type) && (req_ctx->type == SS_METHOD_HMAC_SHA1))
		ss_hmac_method_set(SS_METHOD_SHA1, task);
	else if (CE_METHOD_IS_HMAC(req_ctx->type) && (req_ctx->type == SS_METHOD_HMAC_SHA256))
		ss_hmac_method_set(SS_METHOD_SHA256, task);
	else
		ss_hash_method_set(req_ctx->type, task);

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d / %d\n", flow,
		req_ctx->dir, req_ctx->type, req_ctx->mode, len, ctx->cnt);

	phy_addr = virt_to_phys(ctx->key);
	SS_DBG("ctx->key addr, vir = 0x%p, phy = %pa\n", ctx->key, &phy_addr);
	phy_addr = virt_to_phys(ctx->iv);
	SS_DBG("ctx->iv addr, vir = 0x%p, phy = %pa\n", ctx->iv, &phy_addr);
	phy_addr = virt_to_phys(task);
	SS_DBG("Task addr, vir = 0x%p, phy = %pa\n", task, &phy_addr);

	ss_rng_key_set(ctx->key, ctx->key_size, task);
	ctx->comm.flags &= ~SS_FLAG_NEW_KEY;
	dma_map_single(&ss_dev->pdev->dev,
		ctx->key, ctx->key_size, DMA_MEM_TO_DEV);

	align_size = ss_aes_align_size(req_ctx->type, req_ctx->mode);

	/* Prepare the src scatterlist */
	req_ctx->dma_src.nents = ss_sg_cnt(req_ctx->dma_src.sg, src_len);
	src_len = ss_sg_len(req_ctx->dma_src.sg, len);
	dma_map_sg(&ss_dev->pdev->dev,
		req_ctx->dma_src.sg, req_ctx->dma_src.nents, DMA_MEM_TO_DEV);
	ss_sg_config(task->src, &req_ctx->dma_src,
		req_ctx->type, req_ctx->mode, src_len%align_size, 1);
	ss_aes_map_padding(task->src,
		&req_ctx->dma_src, req_ctx->mode, DMA_MEM_TO_DEV);

	/* Prepare the dst scatterlist */
	req_ctx->dma_dst.nents = ss_sg_cnt(req_ctx->dma_dst.sg, len);
	dma_map_sg(&ss_dev->pdev->dev,
		req_ctx->dma_dst.sg, req_ctx->dma_dst.nents, DMA_DEV_TO_MEM);
	ss_sg_config(task->dst, &req_ctx->dma_dst,
		req_ctx->type, req_ctx->mode, len%align_size, 1);
	ss_aes_map_padding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, DMA_DEV_TO_MEM);

	/* data_len set and last_flag set */
	ss_hmac_sha1_last(task);
	ss_hash_data_len_set((src_len - SHA256_BLOCK_SIZE) * 8, task);

	/*  for openssl add SHA256_BLOCK_SIZE after data*/
	task->src[0].len = (task->src[0].len << 2) - SHA256_BLOCK_SIZE;

	/* addr should set in word, src_len and dst_len set in bytes */
	for (i = 0; i < 8; i++) {
		task->dst[i].len = (task->dst[i].len) << 2;
	}

	/* Start CE controller. */
	init_completion(&ss_dev->flows[flow].done);
	dma_map_single(&ss_dev->pdev->dev, task,
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);

	SS_DBG("Before CE, COMM_CTL: 0x%08x, ICR: 0x%08x\n",
		task->common_ctl, ss_reg_rd(CE_REG_ICR));
	ss_hash_rng_ctrl_start(task);

	ret = wait_for_completion_timeout(&ss_dev->flows[flow].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		ret = -ETIMEDOUT;
	}
	ss_irq_disable(flow);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(task),
			sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);

	/* Unpadding and unmap the dst sg. */
	ss_aes_unpadding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, len%align_size);
	ss_aes_unmap_padding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, DMA_DEV_TO_MEM);
	dma_unmap_sg(&ss_dev->pdev->dev,
		req_ctx->dma_dst.sg, req_ctx->dma_dst.nents, DMA_DEV_TO_MEM);

	/* Unpadding and unmap the src sg. */
	ss_aes_unpadding(task->src,
		&req_ctx->dma_src, req_ctx->mode, src_len%align_size);
	ss_aes_unmap_padding(task->src,
		&req_ctx->dma_src, req_ctx->mode, DMA_MEM_TO_DEV);
	dma_unmap_sg(&ss_dev->pdev->dev,
		req_ctx->dma_src.sg, req_ctx->dma_src.nents, DMA_MEM_TO_DEV);

	dma_unmap_single(&ss_dev->pdev->dev,
		virt_to_phys(ctx->key), ctx->key_size, DMA_MEM_TO_DEV);

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
			ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));
	SS_DBG("After CE, dst data:\n");

	return 0;
}
static int ss_aead_start(ss_aead_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx)
{
	int ret = 0;
	int gcm_iv_mode;
	int i = 0;
	int in_len = ctx->cryptlen;
	int data_len;
	int align_size = 0;
	u32 flow = ctx->comm.flow;
	phys_addr_t phy_addr = 0;
	ce_task_desc_t *task = &ss_dev->flows[flow].task;
	int more;

	ss_change_clk(req_ctx->type);
	ss_task_desc_init(task, flow);

	ss_pending_clear(flow);
	ss_irq_enable(flow);

	ss_method_set(req_ctx->dir, req_ctx->type, task);

	ss_aead_mode_set(req_ctx->mode, task);

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d\n", flow,
		req_ctx->dir, req_ctx->type, req_ctx->mode, in_len);

	/* hardware gcm may be only support continus data, so maybe need software to fix it. */
	/* set iv_addr for task descriptor */
	memset(ctx->task_iv, 0, sizeof(ctx->task_iv));
	if (req_ctx->dir == SS_DIR_DECRYPT) {
		if (!ctx->assoclen) {
			strncpy(ctx->tag, ((char *)sg_dma_address(req_ctx->dma_src.sg) +
					(sg_dma_len(req_ctx->dma_src.sg) - ctx->tag_len)), ctx->tag_len);
		} else {
			strncpy(ctx->tag, ((char *)sg_dma_address(sg_next(req_ctx->dma_src.sg)) +
					(sg_dma_len(sg_next(req_ctx->dma_src.sg)) - ctx->tag_len)), ctx->tag_len);
		}
		for (i = TAG_START; i < ctx->tag_len + TAG_START; i++) {
			ctx->task_iv[i] = ctx->tag[i-TAG_START];  /* only decrypt need */
		}
	}

	ctx->task_iv[IV_SIZE_START] = (ctx->iv_size * 8) & 0xff;
	ctx->task_iv[IV_SIZE_START+1] = ((ctx->iv_size * 8)>>8) & 0xff;
	ctx->task_iv[IV_SIZE_START+2] = ((ctx->iv_size * 8)>>16) & 0xff;
	ctx->task_iv[IV_SIZE_START+3] = ((ctx->iv_size * 8)>>24) & 0xff;

	ctx->task_iv[AAD_SIZE_START] = (ctx->assoclen * 8) & 0xff;
	ctx->task_iv[AAD_SIZE_START+1] = ((ctx->assoclen * 8)>>8) & 0xff;
	ctx->task_iv[AAD_SIZE_START+2] = ((ctx->assoclen * 8)>>16) & 0xff;
	ctx->task_iv[AAD_SIZE_START+3] = ((ctx->assoclen * 8)>>24) & 0xff;

	ctx->task_iv[PT_SIZE_START] = (ctx->cryptlen * 8) & 0xff;
	ctx->task_iv[PT_SIZE_START+1] = ((ctx->cryptlen * 8)>>8) & 0xff;
	ctx->task_iv[PT_SIZE_START+2] = ((ctx->cryptlen * 8)>>16) & 0xff;
	ctx->task_iv[PT_SIZE_START+3] = ((ctx->cryptlen * 8)>>24) & 0xff;

	phy_addr = virt_to_phys(ctx->key);
	SS_DBG("ctx->key addr, vir = 0x%p, phy = %pa\n", ctx->key, &phy_addr);
	ss_key_set(ctx->key, ctx->key_size, task);
	ctx->comm.flags &= ~SS_FLAG_NEW_KEY;
	dma_map_single(&ss_dev->pdev->dev,
		ctx->key, ctx->key_size, DMA_MEM_TO_DEV);

	phy_addr = virt_to_phys(ctx->task_iv);
	SS_DBG("ctx->task_iv vir = 0x%p, phy = 0x%pa\n", ctx->task_iv, &phy_addr);
	ss_iv_set(ctx->task_iv, sizeof(ctx->task_iv), task);
	dma_map_single(&ss_dev->pdev->dev,
		ctx->task_iv, sizeof(ctx->task_iv), DMA_MEM_TO_DEV);

	phy_addr = virt_to_phys(ctx->task_ctr);
	SS_DBG("ctx->task_ctr vir = 0x%p, phy = 0x%pa\n", ctx->task_ctr, &phy_addr);
	ss_gcm_cnt_set(ctx->task_ctr, sizeof(ctx->task_ctr), task);
	dma_map_single(&ss_dev->pdev->dev,
		ctx->task_ctr, sizeof(ctx->task_ctr), DMA_DEV_TO_MEM);

	align_size = ss_aes_align_size(req_ctx->type, req_ctx->mode);

	/* Prepare the src scatterlist */
	req_ctx->dma_src.nents = ss_sg_cnt(req_ctx->dma_src.sg, in_len);
	dma_map_sg(&ss_dev->pdev->dev,
		req_ctx->dma_src.sg, req_ctx->dma_src.nents, DMA_MEM_TO_DEV);

	phy_addr = virt_to_phys(ctx->iv);
	ss_gcm_src_config(&(task->src[0]), phy_addr, DIV_ROUND_UP(ctx->iv_size, align_size)*align_size);
	dma_map_single(&ss_dev->pdev->dev,
		ctx->iv, sizeof(ctx->iv_size), DMA_DEV_TO_MEM);

	ss_aead_sg_config(task->src, &req_ctx->dma_src,
		req_ctx->type, req_ctx->mode, in_len%align_size);
	ss_aead_map_padding(task->src,
		&req_ctx->dma_src, req_ctx->mode, DMA_MEM_TO_DEV);
	if (req_ctx->dir == SS_DIR_DECRYPT)
		task->src[req_ctx->dma_src.nents].len = ((task->src[req_ctx->dma_src.nents].len << 2) - ctx->tag_len) >> 2;

	/* Prepare the dst scatterlist */
	req_ctx->dma_dst.nents = ss_sg_cnt(req_ctx->dma_dst.sg, in_len);
	dma_map_sg(&ss_dev->pdev->dev,
		req_ctx->dma_dst.sg, req_ctx->dma_dst.nents, DMA_DEV_TO_MEM);
	ss_sg_config(task->dst,	&req_ctx->dma_dst,
		req_ctx->type, req_ctx->mode, in_len%align_size, 0);
	ss_aes_map_padding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, DMA_DEV_TO_MEM);

	ss_tag_len_set((ctx->tag_len) * 8, task);
	if (ctx->iv_size == 12)
		gcm_iv_mode = 1;
	else
		gcm_iv_mode = 3;
	ss_gcm_iv_mode(task, gcm_iv_mode);
	ss_cts_last(task);
	more = (req_ctx->dir) ? 0 : DIV_ROUND_UP(ctx->tag_len, align_size) * align_size;
	ss_gcm_reserve_set(task, ctx->iv_size, ctx->assoclen, in_len);
	data_len = (DIV_ROUND_UP(in_len, align_size)*align_size +
			DIV_ROUND_UP(ctx->iv_size, align_size)*align_size +
			DIV_ROUND_UP(ctx->assoclen, align_size)*align_size + more);
	ss_data_len_set(data_len, task);

	/* Start CE controller. */
	init_completion(&ss_dev->flows[flow].done);
	dma_map_single(&ss_dev->pdev->dev, task, sizeof(ce_task_desc_t),
		DMA_MEM_TO_DEV);

	SS_DBG("preCE, COMM: 0x%08x, SYM: 0x%08x, ASYM: 0x%08x, data_len:%d\n",
		task->comm_ctl, task->sym_ctl, task->asym_ctl, task->data_len);

	ss_ctrl_start(task, req_ctx->type, req_ctx->mode);

	ret = wait_for_completion_timeout(&ss_dev->flows[flow].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		ret = -ETIMEDOUT;
	}
	ss_irq_disable(flow);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(task),
		sizeof(ce_task_desc_t), DMA_MEM_TO_DEV);


	/* Unpadding and unmap the dst sg. */
	ss_aes_unpadding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, in_len%align_size);
	ss_aes_unmap_padding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, DMA_DEV_TO_MEM);
	dma_unmap_sg(&ss_dev->pdev->dev,
		req_ctx->dma_dst.sg, req_ctx->dma_dst.nents, DMA_DEV_TO_MEM);

	/* Unpadding and unmap the src sg. */
	ss_aes_unpadding(task->src,
		&req_ctx->dma_src, req_ctx->mode, in_len%align_size);
	ss_aead_unmap_padding(task->src,
		&req_ctx->dma_src, req_ctx->mode, DMA_MEM_TO_DEV);
	dma_unmap_sg(&ss_dev->pdev->dev,
		req_ctx->dma_src.sg, req_ctx->dma_src.nents, DMA_MEM_TO_DEV);

	dma_unmap_single(&ss_dev->pdev->dev,
		virt_to_phys(ctx->task_iv), sizeof(ctx->task_iv), DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(ctx->iv),
			ctx->iv_size, DMA_MEM_TO_DEV);

	dma_unmap_single(&ss_dev->pdev->dev,
		virt_to_phys(ctx->key), ctx->key_size, DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev,
		virt_to_phys(ctx->task_ctr), sizeof(ctx->task_ctr), DMA_DEV_TO_MEM);

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
		ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));
	if (ss_flow_err(flow)) {
		SS_ERR("CE return error: %d\n", ss_flow_err(flow));
		return -EINVAL;
	}

	return 0;
}

static int ss_aes_start(ss_aes_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx, int len)
{
	int ret = 0;
	int src_len = len;
	int align_size = 0;
	u32 flow = ctx->comm.flow;
	phys_addr_t phy_addr = 0;
	ce_task_desc_t *task = &ss_dev->flows[flow].task;

	ss_change_clk(req_ctx->type);
	ss_task_desc_init(task, flow);

	ss_pending_clear(flow);
	ss_irq_enable(flow);

#ifdef SS_XTS_MODE_ENABLE
	if (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, XTS))
		ss_method_set(req_ctx->dir, SS_METHOD_RAES, task);
	else
#endif
	ss_method_set(req_ctx->dir, req_ctx->type, task);

	if ((req_ctx->type == SS_METHOD_RSA)
		|| (req_ctx->type == SS_METHOD_DH)) {
		ss_rsa_width_set(len, task);
		ss_rsa_op_mode_set(req_ctx->mode, task);
	} else if (req_ctx->type == SS_METHOD_ECC) {
		ss_ecc_width_set(len>>1, task);
		ss_ecc_op_mode_set(req_ctx->mode, task);
	} else
		ss_aes_mode_set(req_ctx->mode, task);

#ifdef SS_CFB_MODE_ENABLE
	if (CE_METHOD_IS_AES(req_ctx->type)
		&& (req_ctx->mode == SS_AES_MODE_CFB))
		ss_cfb_bitwidth_set(req_ctx->bitwidth, task);
#endif

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d\n", flow,
		req_ctx->dir, req_ctx->type, req_ctx->mode, len);

	phy_addr = virt_to_phys(ctx->key);
	SS_DBG("ctx->key addr, vir = 0x%p, phy = %pa\n", ctx->key, &phy_addr);
	phy_addr = virt_to_phys(task);
	SS_DBG("Task addr, vir = 0x%p, phy = 0x%pa\n", task, &phy_addr);

#ifdef SS_XTS_MODE_ENABLE
	SS_DBG("The current Key:\n");
	ss_print_hex(ctx->key, ctx->key_size, ctx->key);

	if (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, XTS))
		ss_key_set(ctx->key, ctx->key_size/2, task);
	else
#endif
	ss_key_set(ctx->key, ctx->key_size, task);
	ctx->comm.flags &= ~SS_FLAG_NEW_KEY;
	dma_map_single(&ss_dev->pdev->dev,
		ctx->key, ctx->key_size, DMA_MEM_TO_DEV);

	if (ctx->iv_size > 0) {
		phy_addr = virt_to_phys(ctx->iv);
		SS_DBG("ctx->iv vir = 0x%p, phy = 0x%pa\n", ctx->iv, &phy_addr);
		ss_iv_set(ctx->iv, ctx->iv_size, task);
		dma_map_single(&ss_dev->pdev->dev,
			ctx->iv, ctx->iv_size, DMA_MEM_TO_DEV);

		phy_addr = virt_to_phys(ctx->next_iv);
		SS_DBG("ctx->next_iv addr, vir = 0x%p, phy = 0x%pa\n",
			ctx->next_iv, &phy_addr);
		ss_cnt_set(ctx->next_iv, ctx->iv_size, task);
		dma_map_single(&ss_dev->pdev->dev,
			ctx->next_iv, ctx->iv_size, DMA_DEV_TO_MEM);
	}

	align_size = ss_aes_align_size(req_ctx->type, req_ctx->mode);

	/* Prepare the src scatterlist */
	req_ctx->dma_src.nents = ss_sg_cnt(req_ctx->dma_src.sg, src_len);
	if ((req_ctx->type == SS_METHOD_ECC)
		|| ((req_ctx->type == SS_METHOD_RSA) &&
			(req_ctx->mode == CE_RSA_OP_M_MUL)))
		src_len = ss_sg_len(req_ctx->dma_src.sg, len);

	dma_map_sg(&ss_dev->pdev->dev,
		req_ctx->dma_src.sg, req_ctx->dma_src.nents, DMA_MEM_TO_DEV);
	ss_sg_config(task->src,	&req_ctx->dma_src,
		req_ctx->type, req_ctx->mode, src_len%align_size, 0);
	ss_aes_map_padding(task->src,
		&req_ctx->dma_src, req_ctx->mode, DMA_MEM_TO_DEV);

	/* Prepare the dst scatterlist */
	req_ctx->dma_dst.nents = ss_sg_cnt(req_ctx->dma_dst.sg, len);
	dma_map_sg(&ss_dev->pdev->dev,
		req_ctx->dma_dst.sg, req_ctx->dma_dst.nents, DMA_DEV_TO_MEM);
	ss_sg_config(task->dst,	&req_ctx->dma_dst,
		req_ctx->type, req_ctx->mode, len%align_size, 0);
	ss_aes_map_padding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, DMA_DEV_TO_MEM);

#ifdef SS_SUPPORT_CE_V3_1
	if (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, CTS)) {
		ss_data_len_set(len, task);
/*if (len < SZ_4K)  A bad way to determin the last packet of CTS mode. */
			ss_cts_last(task);
	} else
		ss_data_len_set(
			DIV_ROUND_UP(src_len, align_size)*align_size/4, task);
#else
	if (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, CTS)) {
		/* A bad way to determin the last packet. */
		/* if (len < SZ_4K) */
		ss_cts_last(task);
		ss_data_len_set(src_len, task);
	} else if (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, XTS)) {
		ss_xts_first(task);
		ss_xts_last(task);
		ss_data_len_set(src_len, task);
	} else if (req_ctx->type == SS_METHOD_RSA)
		ss_data_len_set(len*3, task);
	else
		ss_data_len_set(DIV_ROUND_UP(src_len, align_size)*align_size,
			task);
#endif

	/* Start CE controller. */
	init_completion(&ss_dev->flows[flow].done);
	dma_map_single(&ss_dev->pdev->dev, task, sizeof(ce_task_desc_t),
		DMA_MEM_TO_DEV);

	SS_DBG("preCE, COMM: 0x%08x, SYM: 0x%08x, ASYM: 0x%08x, data_len:%d\n",
		task->comm_ctl, task->sym_ctl, task->asym_ctl, task->data_len);

	ss_ctrl_start(task, req_ctx->type, req_ctx->mode);

	ret = wait_for_completion_timeout(&ss_dev->flows[flow].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		ret = -ETIMEDOUT;
	}
	ss_irq_disable(flow);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(task),
		sizeof(ce_task_desc_t), DMA_MEM_TO_DEV);

	/* Unpadding and unmap the dst sg. */
	ss_aes_unpadding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, len%align_size);
	ss_aes_unmap_padding(task->dst,
		&req_ctx->dma_dst, req_ctx->mode, DMA_DEV_TO_MEM);
	dma_unmap_sg(&ss_dev->pdev->dev,
		req_ctx->dma_dst.sg, req_ctx->dma_dst.nents, DMA_DEV_TO_MEM);

	/* Unpadding and unmap the src sg. */
	ss_aes_unpadding(task->src,
		&req_ctx->dma_src, req_ctx->mode, src_len%align_size);
	ss_aes_unmap_padding(task->src,
		&req_ctx->dma_src, req_ctx->mode, DMA_MEM_TO_DEV);
	dma_unmap_sg(&ss_dev->pdev->dev,
		req_ctx->dma_src.sg, req_ctx->dma_src.nents, DMA_MEM_TO_DEV);

	if (ctx->iv_size > 0) {
		dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(ctx->iv),
			ctx->iv_size, DMA_MEM_TO_DEV);
		dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(ctx->next_iv),
			ctx->iv_size, DMA_DEV_TO_MEM);
	}
	/* Backup the next IV from ctr_descriptor, except CBC/CTS/XTS mode. */
	if (CE_METHOD_IS_AES(req_ctx->type)
		&& (req_ctx->mode != SS_AES_MODE_CBC)
		&& (req_ctx->mode != SS_AES_MODE_CTS)
		&& (req_ctx->mode != SS_AES_MODE_XTS))
		memcpy(ctx->iv, ctx->next_iv, ctx->iv_size);

	dma_unmap_single(&ss_dev->pdev->dev,
		virt_to_phys(ctx->key), ctx->key_size, DMA_MEM_TO_DEV);

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
		ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));
	if (ss_flow_err(flow)) {
		SS_ERR("CE return error: %d\n", ss_flow_err(flow));
		return -EINVAL;
	}

	return 0;
}

/* verify the key_len */
int ss_aes_key_valid(struct crypto_ablkcipher *tfm, int len)
{
	if (unlikely(len > SS_RSA_MAX_SIZE)) {
		SS_ERR("Unsupported key size: %d\n", len);
		tfm->base.crt_flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}
	return 0;
}

#ifdef SS_RSA_PREPROCESS_ENABLE
static void ss_rsa_preprocess(ss_aes_ctx_t *ctx,
	ss_aes_req_ctx_t *req_ctx, int len)
{
	struct scatterlist sg = {0};
	ss_aes_req_ctx_t *tmp_req_ctx = NULL;

	if (!((req_ctx->type == SS_METHOD_RSA) &&
		(req_ctx->mode != CE_RSA_OP_M_MUL)))
		return;

	tmp_req_ctx = kmalloc(sizeof(ss_aes_req_ctx_t), GFP_KERNEL);
	if (tmp_req_ctx == NULL) {
		SS_ERR("Failed to malloc(%d)\n", sizeof(ss_aes_req_ctx_t));
		return;
	}

	memcpy(tmp_req_ctx, req_ctx, sizeof(ss_aes_req_ctx_t));
	tmp_req_ctx->mode = CE_RSA_OP_M_MUL;

	sg_init_one(&sg, ctx->key, ctx->iv_size*2);
	tmp_req_ctx->dma_src.sg = &sg;

	ss_aes_start(ctx, tmp_req_ctx, len);

	SS_DBG("The preporcess of RSA complete!\n\n");
	kfree(tmp_req_ctx);
}
#endif

static int ss_rng_start(ss_aes_ctx_t *ctx, u8 *rdata, u32 dlen, u32 trng)
{
	int ret = 0;
	int i = 0;
	int flow = ctx->comm.flow;
	int rng_len = 0;
	char *buf = NULL;
	phys_addr_t phy_addr = 0;
	ce_new_task_desc_t *task = (ce_new_task_desc_t *)&ss_dev->flows[flow].task;

	if (trng)
		rng_len = DIV_ROUND_UP(dlen, 32)*32; /* align with 32 Bytes */
	else
		rng_len = DIV_ROUND_UP(dlen, 20)*20; /* align with 20 Bytes */

	if (rng_len > SS_RNG_MAX_LEN) {
		SS_ERR("The RNG length is too large: %d\n", rng_len);
		rng_len = SS_RNG_MAX_LEN;
	}

	buf = kmalloc(rng_len, GFP_KERNEL);
	if (buf == NULL) {
		SS_ERR("Failed to malloc(%d)\n", rng_len);
		return -ENOMEM;
	}

	ss_hash_rng_change_clk();

	ss_new_task_desc_init(task, flow);

	ss_pending_clear(flow);
	ss_irq_enable(flow);

	if (trng)
		ss_rng_method_set(SS_METHOD_SHA256, SS_METHOD_TRNG, task);
	else
		ss_rng_method_set(SS_METHOD_SHA1, SS_METHOD_PRNG, task);

	phy_addr = virt_to_phys(ctx->key);
	SS_DBG("ctx->key addr, vir = 0x%p, phy = %pa\n", ctx->key, &phy_addr);

	if (trng == 0) {
		/* Must set the seed addr in PRNG, key_len 5 words stable*/
		ctx->key_size = 5 * sizeof(int);
		ss_rng_key_set(ctx->key, ctx->key_size, task);
		task->data_len = 0;
		ctx->comm.flags &= ~SS_FLAG_NEW_KEY;
		dma_map_single(&ss_dev->pdev->dev,
			ctx->key, ctx->key_size, DMA_MEM_TO_DEV);
	}
	phy_addr = virt_to_phys(buf);
	SS_DBG("buf addr, vir = 0x%p, phy = %pa\n", buf, &phy_addr);

	/* Prepare the dst scatterlist */
	task->dst[0].addr = virt_to_phys(buf) >> 2;	/*address in words*/
	task->dst[0].len  = (rng_len + 3)/4;
	dma_map_single(&ss_dev->pdev->dev, buf, rng_len, DMA_DEV_TO_MEM);

	SS_DBG("Flow: %d, Request: %d, Aligned: %d\n", flow, dlen, rng_len);

	phy_addr = virt_to_phys(task);
	SS_DBG("Task addr, vir = 0x%p, phy = %pa\n", task, &phy_addr);

	/* addr should set in word, src_len and dst_len set in bytes */
	for (i = 0; i < 8; i++) {
		task->src[i].len = (task->src[i].len) << 2;
		task->dst[i].len = (task->dst[i].len) << 2;
	}

	/* Start CE controller. */
	init_completion(&ss_dev->flows[flow].done);
	dma_map_single(&ss_dev->pdev->dev, task,
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);

	SS_DBG("Before CE, COMM_CTL: 0x%08x, ICR: 0x%08x\n",
		task->common_ctl, ss_reg_rd(CE_REG_ICR));
	ss_hash_rng_ctrl_start(task);

	ret = wait_for_completion_timeout(&ss_dev->flows[flow].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		ret = -ETIMEDOUT;
	}
	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
		ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));
	SS_DBG("After CE, dst data:\n");

	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(task),
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(buf),
		rng_len, DMA_DEV_TO_MEM);
	if (trng == 0)
		dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(ctx->key),
			ctx->key_size, DMA_MEM_TO_DEV);
	memcpy(rdata, buf, dlen);
	kfree(buf);
	ss_irq_disable(flow);
	ret = dlen;

	return ret;
}

int ss_rng_get_random(struct crypto_rng *tfm, u8 *rdata, u32 dlen, u32 trng)
{
	int ret = 0;
	u8 *data = rdata;
	u32 len = dlen;
	ss_aes_ctx_t *ctx = crypto_rng_ctx(tfm);

	SS_DBG("flow = %d, data = %p, len = %d, trng = %d\n",
		ctx->comm.flow, data, len, trng);
	if (ss_dev->suspend) {
		SS_ERR("SS has already suspend.\n");
		return -EAGAIN;
	}

	ss_dev_lock();
	ret = ss_rng_start(ctx, data, len, trng);
	ss_dev_unlock();

	SS_DBG("Get %d byte random.\n", ret);

	return ret;
}

static int ss_drbg_start(ss_drbg_ctx_t *ctx, u8 *src, u32 slen, u8 *rdata, u32 dlen, u32 mode)
{
	int ret = 0;
	int flow = ctx->comm.flow;
	int rng_len = dlen;
	char *buf = NULL;
	char *entropy = NULL;
	char *person = NULL;
	phys_addr_t phy_addr = 0;
	ce_new_task_desc_t *task = (ce_new_task_desc_t *)&ss_dev->flows[flow].task;

	if (rng_len > SS_RNG_MAX_LEN) {
		SS_ERR("The RNG length is too large: %d\n", rng_len);
		rng_len = SS_RNG_MAX_LEN;
	}

	if (ctx->entropt_size < 80 / 8) {
		SS_ERR("The DRBG length is too small: %d, less than 80 bit\n", ctx->entropt_size);
		return -EINVAL;
	}

	buf = kmalloc(rng_len, GFP_KERNEL);
	if (buf == NULL) {
		SS_ERR("Failed to malloc(%d)\n", rng_len);
		return -ENOMEM;
	}

	entropy = kmalloc(ctx->entropt_size, GFP_KERNEL);
	if (entropy == NULL) {
		SS_ERR("Failed to malloc(%d)\n", ctx->entropt_size);
		return -ENOMEM;
	}
	memcpy(entropy, ctx->entropt, ctx->entropt_size);

	if (ctx->person_size) {
		person = kmalloc(ctx->person_size, GFP_KERNEL);
		if (buf == NULL) {
			SS_ERR("Failed to malloc(%d)\n", ctx->person_size);
			return -ENOMEM;
		}
	}
	ss_hash_rng_change_clk();

	ss_new_task_desc_init(task, flow);

	ss_pending_clear(flow);
	ss_irq_enable(flow);

	ss_rng_method_set(mode, SS_METHOD_DRBG, task);

	phy_addr = virt_to_phys(ctx->entropt);
	SS_DBG("ctx->entropt, vir = 0x%p, phy = %pa\n", ctx->entropt, &phy_addr);

	phy_addr = virt_to_phys(ctx->person);
	SS_DBG("ctx->person, vir = 0x%p, phy = %pa\n", ctx->person, &phy_addr);

	phy_addr = virt_to_phys(buf);
	SS_DBG("buf addr, vir = 0x%p, phy = %pa\n", buf, &phy_addr);

	/* Prepare the dst scatterlist */
	task->src[0].addr = virt_to_phys(entropy) >> 2;	/*address in words*/
	task->src[0].len = ctx->entropt_size;
	task->src[1].addr = (virt_to_phys(ctx->nonce)) >> 2;	/* in software, not used*/
	task->src[1].len = 0;
	task->src[2].addr = virt_to_phys(person) >> 2;	/*address in words*/
	task->src[2].len = ctx->person_size;
	task->src[3].addr = virt_to_phys(src) >> 2;	/*address in words*/
	task->src[3].len = slen;

	dma_map_single(&ss_dev->pdev->dev, entropy, ctx->entropt_size, DMA_MEM_TO_DEV);
	dma_map_single(&ss_dev->pdev->dev, ctx->nonce, ctx->nonce_size, DMA_MEM_TO_DEV);
	dma_map_single(&ss_dev->pdev->dev, person, ctx->person_size, DMA_MEM_TO_DEV);
	dma_map_single(&ss_dev->pdev->dev, src, slen, DMA_MEM_TO_DEV);

	/* Prepare the dst scatterlist */
	task->dst[0].addr = virt_to_phys(buf) >> 2;	/*address in words*/
	task->dst[0].len  = rng_len;

	dma_map_single(&ss_dev->pdev->dev, buf, rng_len, DMA_DEV_TO_MEM);

	SS_DBG("Flow: %d, Request: %d, Aligned: %d\n", flow, dlen, rng_len);

	phy_addr = virt_to_phys(task);
	SS_DBG("Task addr, vir = 0x%p, phy = %pa\n", task, &phy_addr);

	/* Start CE controller. */
	init_completion(&ss_dev->flows[flow].done);
	dma_map_single(&ss_dev->pdev->dev, task,
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);

	SS_DBG("Before CE, COMM_CTL: 0x%08x, ICR: 0x%08x\n",
		task->common_ctl, ss_reg_rd(CE_REG_ICR));

	ss_hash_rng_ctrl_start(task);

	ret = wait_for_completion_timeout(&ss_dev->flows[flow].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		ret = -ETIMEDOUT;
	}
	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
		ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));
	SS_DBG("After CE, dst data:\n");

	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(task),
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);

	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(entropy),
					ctx->entropt_size, DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(ctx->nonce),
					ctx->nonce_size, DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(person),
					ctx->person_size, DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(src),
					slen, DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(buf),
					rng_len, DMA_DEV_TO_MEM);

	memcpy(rdata, buf, dlen);
	kfree(buf);
	if (ctx->person_size)
		kfree(person);
	kfree(entropy);
	ss_irq_disable(flow);
	ret = dlen;

	return ret;
}

int ss_drbg_get_random(struct crypto_rng *tfm, const u8 *src, u32 slen, u8 *rdata, u32 dlen, u32 mode)
{
	int ret = 0;
	u8 *data = rdata;
	u32 len = dlen;
	u8 *src_t;
	ss_drbg_ctx_t *ctx = crypto_rng_ctx(tfm);

	SS_DBG("flow = %d, src = %p, slen = %d, data = %p, len = %d, hash_mode = %d\n",
		ctx->comm.flow, src, slen, data, len, mode);
	if (ss_dev->suspend) {
		SS_ERR("SS has already suspend.\n");
		return -EAGAIN;
	}

	src_t = kzalloc(slen, GFP_KERNEL);
	if (src_t == NULL) {
		SS_ERR("Failed to kmalloc(%d)\n", slen);
		return -ENOMEM;
	}
	memcpy(src_t, src, slen);

	ss_dev_lock();
	ret = ss_drbg_start(ctx, src_t, slen, data, len, mode);
	ss_dev_unlock();

	kfree(src_t);
	SS_DBG("Get %d byte random.\n", ret);

	return ret;
}


u32 ss_hash_start(ss_hash_ctx_t *ctx,
		ss_aes_req_ctx_t *req_ctx, u32 len, u32 last)
{
	int ret = 0;
	int i = 0;
	int flow = ctx->comm.flow;
	u32 blk_size = ss_hash_blk_size(req_ctx->type);
	char *digest = NULL;
	phys_addr_t phy_addr = 0;
	ce_new_task_desc_t *task = (ce_new_task_desc_t *)&ss_dev->flows[flow].task;

	/* Total len is too small, so process it in the padding data later. */
	if ((last == 0) && (len > 0) && (len < blk_size)) {
		ctx->cnt += len;
		return 0;
	}
	ss_hash_rng_change_clk();

	digest = kzalloc(SHA512_DIGEST_SIZE, GFP_KERNEL);
	if (digest == NULL) {
		SS_ERR("Failed to kmalloc(%d)\n", SHA512_DIGEST_SIZE);
		return -ENOMEM;
	}

	ss_new_task_desc_init(task, flow);

	ss_pending_clear(flow);
	ss_irq_enable(flow);

	ss_hash_method_set(req_ctx->type, task);

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d / %d\n", flow,
		req_ctx->dir, req_ctx->type, req_ctx->mode, len, ctx->cnt);
	SS_DBG("IV address = 0x%p, size = %d\n", ctx->md, ctx->md_size);
	phy_addr = virt_to_phys(task);
	SS_DBG("Task addr, vir = 0x%p, phy = %pa\n", task, &phy_addr);

	ss_hash_iv_set(ctx->md, ctx->md_size, task);
	ss_hash_iv_mode_set(1, task);
	dma_map_single(&ss_dev->pdev->dev,
		ctx->md, ctx->md_size, DMA_MEM_TO_DEV);

	if (last == 1) {
		ss_hmac_sha1_last(task);
		ss_hash_data_len_set(ctx->tail_len*8, task);
	} else
		ss_hash_data_len_set((len - len%blk_size)*8, task);

	/* Prepare the src scatterlist */
	req_ctx->dma_src.nents = ss_sg_cnt(req_ctx->dma_src.sg, len);
	dma_map_sg(&ss_dev->pdev->dev, req_ctx->dma_src.sg,
		req_ctx->dma_src.nents, DMA_MEM_TO_DEV);
	ss_sg_config(task->src,
		&req_ctx->dma_src, req_ctx->type, 0, len%blk_size, 1);

#ifdef SS_HASH_HW_PADDING
	if (last == 1) {
		task->src[0].len = (ctx->tail_len + 3)/4;	/* src/dst data_len special*/
		SS_DBG("cnt %d, tail_len %d.\n", ctx->cnt, ctx->tail_len);
		ctx->cnt <<= 3; /* Translate to bits in the last pakcket */
		task->data_len = ctx->cnt;
	}
#endif

	/* Prepare the dst scatterlist */
	task->dst[0].addr = virt_to_phys(digest) >> 2; /*address in word*/
	task->dst[0].len  = ctx->md_size/4 ;		/* src/dst data_len special*/

	if (last == 1) {
		if (req_ctx->type == SS_METHOD_SHA224)
			task->dst[0].len  = SHA224_DIGEST_SIZE/4 ;
		if (req_ctx->type == SS_METHOD_SHA384)
			task->dst[0].len  = SHA384_DIGEST_SIZE/4 ;
	}

	dma_map_single(&ss_dev->pdev->dev,
		digest, SHA512_DIGEST_SIZE, DMA_DEV_TO_MEM);
	phy_addr = virt_to_phys(digest);
	SS_DBG("digest addr, vir = 0x%p, phy = %pa\n", digest, &phy_addr);

	/* addr should set in word, src_len and dst_len set in bytes */
		for (i = 0; i < 8; i++) {
			task->src[i].len = (task->src[i].len) << 2;
			task->dst[i].len = (task->dst[i].len) << 2;
		}

	/* Start CE controller. */
	init_completion(&ss_dev->flows[flow].done);
	dma_map_single(&ss_dev->pdev->dev, task,
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);

	SS_DBG("Before CE, COMM_CTL: 0x%08x, ICR: 0x%08x\n",
		task->common_ctl, ss_reg_rd(CE_REG_ICR));
	ss_hash_rng_ctrl_start(task);

	ret = wait_for_completion_timeout(&ss_dev->flows[flow].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ss_reset();
		ret = -ETIMEDOUT;
	}
	ss_irq_disable(flow);

	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(task),
		sizeof(ce_new_task_desc_t), DMA_MEM_TO_DEV);
	dma_unmap_single(&ss_dev->pdev->dev, virt_to_phys(digest),
		SHA512_DIGEST_SIZE, DMA_DEV_TO_MEM);
	dma_unmap_sg(&ss_dev->pdev->dev, req_ctx->dma_src.sg,
		req_ctx->dma_src.nents, DMA_MEM_TO_DEV);
#ifdef SS_HASH_HW_PADDING
	if (last == 1) {
		ctx->cnt >>= 3;
	}
#endif

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
			ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));
	SS_DBG("After CE, dst data:\n");
	ss_print_hex(digest, SHA512_DIGEST_SIZE, digest);

	if (ss_flow_err(flow)) {
		SS_ERR("CE return error: %d\n", ss_flow_err(flow));
		kfree(digest);
		return -EINVAL;
	}

	/* Backup the MD to ctx->md. */
	memcpy(ctx->md, digest, ctx->md_size);

	if (last == 0)
		ctx->cnt += len;
	kfree(digest);
	return 0;
}

void ss_load_iv(ss_aes_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx,
	char *buf, int size)
{
	if (buf == NULL)
		return;

	/* Only AES/DES/3DES-ECB don't need IV. */
	if (CE_METHOD_IS_AES(req_ctx->type) &&
		(req_ctx->mode == SS_AES_MODE_ECB))
		return;

	/* CBC/CTS/GCM need update the IV eachtime. */
	if ((ctx->cnt == 0)
		|| (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, CBC))
		|| (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, CTS))
		|| (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, GCM))
		|| (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, XTS))) {
		SS_DBG("IV address = %p, size = %d\n", buf, size);
		ctx->iv_size = size;
		memcpy(ctx->iv, buf, ctx->iv_size);
	}

	SS_DBG("The current IV:\n");
	ss_print_hex(ctx->iv, ctx->iv_size, ctx->iv);
}

void ss_aead_load_iv(ss_aead_ctx_t *ctx, ss_aes_req_ctx_t *req_ctx,
	char *buf, int size)
{
	if (buf == NULL)
		return;

	/* CBC/CTS/GCM need update the IV eachtime. */
	if ((ctx->cnt == 0)
		|| (CE_IS_AES_MODE(req_ctx->type, req_ctx->mode, GCM))) {
		SS_DBG("IV address = %p, size = %d\n", buf, size);
		ctx->iv_size = size;
		memcpy(ctx->iv, buf, ctx->iv_size);
	}

	SS_DBG("The current IV:\n");
	ss_print_hex(ctx->iv, ctx->iv_size, ctx->iv);
}

int ss_aead_one_req(sunxi_ss_t *sss, struct aead_request *req)
{
	int ret = 0;
	struct crypto_aead *tfm = NULL;
	ss_aead_ctx_t *ctx = NULL;
	ss_aes_req_ctx_t *req_ctx = NULL;

	SS_ENTER();
	if (!req->src || !req->dst) {
		SS_ERR("Invalid sg: src = %p, dst = %p\n", req->src, req->dst);
		return -EINVAL;
	}

	ss_dev_lock();

	tfm = crypto_aead_reqtfm(req);
	req_ctx = aead_request_ctx(req);
	ctx = crypto_aead_ctx(tfm);

	ss_aead_load_iv(ctx, req_ctx, req->iv, crypto_aead_ivsize(tfm));

	memset(ctx->task_ctr, 0, sizeof(ctx->task_ctr));
	ctx->tag_len = tfm->authsize;
	ctx->assoclen = req->assoclen;
	ctx->cryptlen = req->cryptlen;

	req_ctx->dma_src.sg = req->src;
	req_ctx->dma_dst.sg = req->dst;

	ret = ss_aead_start(ctx, req_ctx);
	if (ret < 0)
		SS_ERR("ss_aes_start fail(%d)\n", ret);

	ss_dev_unlock();

	return ret;
}

int ss_aes_one_req(sunxi_ss_t *sss, struct ablkcipher_request *req)
{
	int ret = 0;
	struct crypto_ablkcipher *tfm = NULL;
	ss_aes_ctx_t *ctx = NULL;
	ss_aes_req_ctx_t *req_ctx = NULL;

	SS_ENTER();
	if (!req->src || !req->dst) {
		SS_ERR("Invalid sg: src = %p, dst = %p\n", req->src, req->dst);
		return -EINVAL;
	}

	ss_dev_lock();

	tfm = crypto_ablkcipher_reqtfm(req);
	req_ctx = ablkcipher_request_ctx(req);
	ctx = crypto_ablkcipher_ctx(tfm);

	ss_load_iv(ctx, req_ctx, req->info, crypto_ablkcipher_ivsize(tfm));

	req_ctx->dma_src.sg = req->src;
	req_ctx->dma_dst.sg = req->dst;

#ifdef SS_RSA_PREPROCESS_ENABLE
	ss_rsa_preprocess(ctx, req_ctx, req->nbytes);
#endif

	if (CE_METHOD_IS_HMAC(req_ctx->type)) {
		ret = ss_hmac_start(ctx, req_ctx, req->nbytes);
	} else {
		ret = ss_aes_start(ctx, req_ctx, req->nbytes);
	}
	if (ret < 0)
		SS_ERR("ss_aes_start fail(%d)\n", ret);

	ss_dev_unlock();

#ifdef SS_CTR_MODE_ENABLE
	if (req_ctx->mode == SS_AES_MODE_CTR) {
		SS_DBG("CNT: %08x %08x %08x %08x\n",
			*(int *)&ctx->iv[0], *(int *)&ctx->iv[4],
			*(int *)&ctx->iv[8], *(int *)&ctx->iv[12]);
	}
#endif

	ctx->cnt += req->nbytes;
	return ret;
}

irqreturn_t sunxi_ss_irq_handler(int irq, void *dev_id)
{
	int i;
	int pending = 0;
	sunxi_ss_t *sss = (sunxi_ss_t *)dev_id;
	unsigned long flags = 0;

	spin_lock_irqsave(&sss->lock, flags);

	pending = ss_pending_get();
	SS_DBG("pending: %#x\n", pending);
	for (i = 0; i < SS_FLOW_NUM; i++) {
		if (pending & (CE_CHAN_PENDING << (2 * i))) {
			SS_DBG("Chan %d completed. pending: %#x\n", i, pending);
			ss_pending_clear(i);
			complete(&sss->flows[i].done);
		}
	}

	spin_unlock_irqrestore(&sss->lock, flags);
	return IRQ_HANDLED;
}