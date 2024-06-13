/*
 * Firmware debug interface for XRadio drivers
 *
 * Copyright (c) 2013
 * Xradio Technology Co., Ltd. <www.xradiotech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef SUPPORT_FW_DBG_INF

/* register define */

#define FWD_PAS_REG_NTD_STATUS_EMPTY			(1<<31)
#define FWD_PAS_REG_NTD_STATUS_OVERFLOW			(1<<30)
#define FWD_PAS_REG_NTD_STATUS_TSF_REQ_SWITCH		(1<<28)
#define FWD_PAS_REG_NTD_STATUS_TSF_HW_EVENT_1		(1<<27)
#define FWD_PAS_REG_NTD_STATUS_TSF_HW_EVENT_0		(1<<26)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT		(1<<25)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT			(1<<24)
#define FWD_PAS_REG_NTD_STATUS_EBM_STATUS		(1<<23)

/* tsf request switch */

#define FWD_PAS_REG_NTD_STATUS_TSF_REQ_SWITCH_STATUS	(1<<29)

/* ptcs event */
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_MASK		(3<<16)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_SHIFT		(16)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_NONE		(0)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT		(1)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_TTCS_END	(2)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_RECIPE_END	(3)

#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_BT_ABORT	(1<<14)

/* tx request event */
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_MASK		(0x3F<<8)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_SHIFT		(8)

#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_AGGREND_RSPBA_0		(0)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_AGGREND_RSPBA_1		(1)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_RTS_0			(7)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_RTS_1			(8)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_UNI_RXED_TXACK_0		(19)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_UNI_RXED_TXACK_1		(20)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_TSF_HW_EVENT_0		(25)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_HIGH_TSF_HW_EVENT_1		(26)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_LOW_BAR_IMM_0			(36)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_LOW_BAR_IMM_1			(37)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_RESP_TIMEOUT			(52)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_GBM_TX_REQ0			(53)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_GBM_TX_REQ1			(54)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_REQUEST_EBM_SEQ_REQ			(55)

/* ebm queue */
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_EBM_SEQ_REQ_IN_TXOP		(1<<15)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_EBM_SEQ_REQ_WIN_AC_MASK	(3<<18)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_EBM_SEQ_REQ_WIN_AC_SHIFT	(18)

/* tx abort */
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_MASK	(0x7<<20)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_SHIFT	(20)

#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_READING_TTCS		(0)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_IFS_RUNNING		(1)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_IFS_COMPLETE		(2)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_MEDIUM_BUSY		(3)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_BACKOFF_COMPLETE	(4)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_ABORT_PHY_ERROR		(5)

/* tx bandwith */
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_TX_BANDWIDTH_MASK		(0x1<<5)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_TX_BANDWIDTH_SHIFT		(5)

#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_TX_BANDWIDTH_20M		(0)
#define FWD_PAS_REG_NTD_STATUS_PTCS_EVENT_TX_BANDWIDTH_40M		(1)

/* rx event */
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_MASK			(0x1F<<0)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_SHIFT			(0)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_RESP_RX_STROED		(1<<7)

#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_RESP_TIMEOUT		(25)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_ACK_RXED		(17)

#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_MULTICAST_RXED		(15)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_BEACON_RXED		(14)

#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_BA_IMM			(12)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_RTS_0			(07)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_CTS			(06)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_RESERVED_1		(05)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_CRC_ERROR		(04)
#define FWD_PAS_REG_NTD_STATUS_RX_EVENT_FRAME_ABORT		(03)

/* ebm status */
#define FWD_PAS_REG_EBM_INT_CONTEN_INT_CONTENTION_MASK		(0xF<<0)
#define FWD_PAS_REG_EBM_INT_CONTEN_INT_CONTENTION_SHIFT		(0)
#define FWD_PAS_REG_EBM_INT_FIRST_FRAME_ERROR_MASK		(0xF<<4)
#define FWD_PAS_REG_EBM_INT_FIRST_FRAME_ERROR_SHIFT		(4)
#define FWD_PAS_REG_EBM_INT_ACK_FAIL_MASK			(0xF<<8)
#define FWD_PAS_REG_EBM_INT_ACK_FAIL_SHIFT			(8)
#define FWD_PAS_REG_EBM_INT_TX_ERROR_MASK			(0xF<<12)
#define FWD_PAS_REG_EBM_INT_TX_ERROR_SHIFT			(12)

/* queue contronl */
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_FULL			(1<<31)
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_NOT_EMPTY			(1<<30)

#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_READ_POINTER_MASK		(0x3<<27)
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_READ_POINTER_SHIFT	(27)

#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_WRITE_POINTER_MASK	(0x3<<24)
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_WRITE_POINTER_SHIFT	(24)

#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_ACK_EXPECT_MASK		(0xF<<16)
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_ACK_EXPECT_SHIFT		(16)

#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_ACK_RXED_MASK		(0xF<<8)
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_ACK_RXED_SHIFT		(8)

#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_T_FLAG_MASK		(0xF<<0)
#define FWD_PAS_REG_SEQ_QUEUE_CONTROL_T_FLAG_SHIFT		(0)

/* ntd control */
#define FWD_PAS_REG_NTD_CONTROL_STATE_MASK			(0x1F<<8)
#define FWD_PAS_REG_NTD_CONTROL_STATE_SHIFT			(8)
#define FWD_PAS_REG_NTD_CONTROL_LOW_RX_RESP_REQ_MASK		(0x1F<<16)
#define FWD_PAS_REG_NTD_CONTROL_LOW_RX_RESP_REQ_SHIFT		(16)
#define FWD_PAS_REG_NTD_CONTROL_RESP_TIMEOUT_REQ		(1<<21)
#define FWD_PAS_REG_NTD_CONTROL_TSF_HW_EVENT0_REQ		(1<<21)
#define FWD_PAS_REG_NTD_CONTROL_TSF_HW_EVENT1_REQ		(1<<22)
#define FWD_PAS_REG_NTD_CONTROL_HIGH_RX_RESP_REQ_MASK		(0x1F<<24)
#define FWD_PAS_REG_NTD_CONTROL_HIGH_RX_RESP_REQ_SHIFT		(24)

#define FWD_PAS_REG_NTD_CONTROL_STATE_IDLE			(0)

#define FWD_PAS_REG_NTD_CONTROL_RX_RESP_REQ_NO_PENDING		(25)

/* txc state */
#define FWD_PAS_REG_TXC_STATUS_STATE				(0x3F)
#define FWD_PAS_REG_TXC_STATUS_IFS_COMPLETE_FLAG		(1<<6)
#define FWD_PAS_REG_TXC_STATUS_BACKOFF_COMPLETE_FLAG		(1<<7)
#define FWD_PAS_REG_TXC_STATUS_PTCS_POINTER_INDEX_MASK		(0x1F<<8)
#define FWD_PAS_REG_TXC_STATUS_PTCS_POINTER_INDEX_SHIFT		(8)
#define FWD_PAS_REG_TXC_STATUS_LAST_RX_CRC_ERROR		(1<<14)
#define FWD_PAS_REG_TXC_STATUS_MAC_CCA				(1<<15)

#define FWD_PAS_REG_TXC_STATUS_STATE_IDLE			(0)

/* rxc state */
#define FWD_PAS_REG_RXC_RX_CONTROL_RX_ENABLE			(1<<0)
#define FWD_PAS_REG_RXC_RX_CONTROL_BAP_DIS_PHY			(1<<1)
#define FWD_PAS_REG_RXC_RX_CONTROL_APPEND_ERROR			(1<<2)
#define FWD_PAS_REG_RXC_RX_CONTROL_APPEND_BAP_STATUS		(1<<3)
#define FWD_PAS_REG_RXC_RX_CONTROL_BT_DIS_RX			(1<<4)
#define FWD_PAS_REG_RXC_RX_CONTROL_KEEP_BAD_FRAMES			(1<<15)
#define FWD_PAS_REG_RXC_RX_CONTROL_AUTO_DISCARD			(1<<16)
#define FWD_PAS_REG_RXC_RX_CONTROL_AUTO_KEEP			(1<<17)
#define FWD_PAS_REG_RXC_RX_CONTROL_PHY_CCA				(1<<18)
#define FWD_PAS_REG_RXC_RX_CONTROL_PHY_CCA_VALID			(1<<19)
#define FWD_PAS_REG_RXC_RX_CONTROL_RXBUF_NEAR_FULL			(1<<20)
#define FWD_PAS_REG_RXC_RX_CONTROL_RXBUF_OVERFLOW			(1<<21)
#define FWD_PAS_REG_RXC_RX_CONTROL_RXFIFO_OVERFLOW			(1<<22)
#define FWD_PAS_REG_RXC_RX_CONTROL_RX_CTRL_RXBUSY			(1<<23)
#define FWD_PAS_REG_RXC_RX_CONTROL_APPEND_STATUS			(1<<24)
#define FWD_PAS_REG_RXC_RX_CONTROL_APPEND_TSF			(1<<26)
#define FWD_PAS_REG_RXC_RX_CONTROL_STORE_CSI_DATA			(1<<27)
#define FWD_PAS_REG_RXC_RX_CONTROL_DISABLE_DEAGGR			(1<<30)

#define FWD_PAS_REG_RXC_ERROR_CODE_ERROR_CODE			(0xFFFFFF)
#define FWD_PAS_REG_RXC_ERROR_CODE_SETTING_MASK			(0xF<<28)
#define FWD_PAS_REG_RXC_ERROR_CODE_SETTING_SHIFT			(28)

#define FWD_PAS_REG_RXC_ERROR_COUNT(_reg, _shift)	(((_reg) & (0xFF << (_shift))) \
								>> (_shift))

#define FWD_GET_REG_SECTION(_reg, _mask, _shift) \
		(((_reg) & ((_mask) << (_shift))) >> (_shift));

#define HWD_PAS_EBM_STAT_TIMING_STATE				(0xF)

/* firmware debug message id */
#define FWD_CMD_MAJOR_ID_MASK		(0xFF00)
#define FWD_CMD_MINOR_ID_MASK		(0x00FF)

#define FWD_CMD_MAJOR_ID_SYS		(0x0100)
#define FWD_CMD_MAJOR_ID_SOC		(0x0200)
#define FWD_CMD_MAJOR_ID_LMC		(0x0300)
#define FWD_CMD_MAJOR_ID_PAS		(0x0400)
#define FWD_CMD_MAJOR_ID_PHY		(0x0500)
#define FWD_CMD_MAJOR_ID_RF		(0x0600)
#define FWD_CMD_MAJOR_ID_EPTA		(0x0700)

enum FWD_CMD_MINOR_ID_SYS {
	FWD_CMD_MINOR_ID_SYS_UNDEF,
	FWD_CMD_MINOR_ID_SYS_CONFIG,
	FWD_CMD_MINOR_ID_SYS_CPU_LOAD,
	FWD_CMD_MINOR_ID_SYS_MAX
};

enum FWD_CMD_MINOR_ID_SOC {
	FWD_CMD_MINOR_ID_SOC_UNDEF,
	FWD_CMD_MINOR_ID_SOC_LPCLK_STAT,
	FWD_CMD_MINOR_ID_SOC_MAX
};

enum FWD_CMD_MINOR_ID_LMC {
	FWD_CMD_MINOR_ID_LMC_UNDEF,
	FWD_CMD_MINOR_ID_LMC_MAX
};

enum FWD_CMD_MINOR_ID_PAS {
	FWD_CMD_MINOR_ID_PAS_UNDEF,
	FWD_CMD_MINOR_ID_PAS_FIQ_DUMP,
	FWD_CMD_MINOR_ID_PAS_FIQ_TRACE,
	FWD_CMD_MINOR_ID_PAS_PTCS_DUMP,
	FWD_CMD_MINOR_ID_PAS_HW_STATUS,
	FWD_CMD_MINOR_ID_PAS_HW_STAT,
	FWD_CMD_MINOR_ID_PAS_FORCE_MODE,
	FWD_CMD_MINOR_ID_PAS_DUR_ENTRY,
	FWD_CMD_MINOR_ID_PAS_FORCE_TX,
	FWD_CMD_MINOR_ID_PAS_MAX
};

enum FWD_CMD_MINOR_ID_PHY {
	FWD_CMD_MINOR_ID_PHY_UNDEF,
	FWD_CMD_MINOR_ID_PHY_READ_REG,
	FWD_CMD_MAJOR_ID_PHY_LOAD_REG,
	FWD_CMD_MINOR_ID_PHY_MAX
};

enum FWD_CMD_MINOR_ID_RF {
	FWD_CMD_MINOR_ID_RF_UNDEF,
	FWD_CMD_MINOR_ID_RF_MAX
};

enum FWD_CMD_MINOR_ID_EPTA {
	FWD_CMD_MINOR_ID_EPTA_UNDEF,
	FWD_CMD_MINOR_ID_EPTA_TIME_LINE,
	FWD_CMD_MINOR_ID_EPTA_RF_STAT,
	FWD_CMD_MINOR_ID_EPTA_MAX
};

struct fwd_local {
	struct dentry *dbgfs_fw;
	struct dentry *dbgfs_sys;
	struct dentry *dbgfs_soc;
	struct dentry *dbgfs_lmc;
	struct dentry *dbgfs_pas;
	struct dentry *dbgfs_phy;
	struct dentry *dbgfs_rf;
	struct dentry *dbgfs_epta;
};

struct fwd_msg {
	u16 dbg_len;
	u16 dbg_id;
	u32 dbg_info;
};

struct fwd_sys_config {
	u32 frm_trace_addr;
	u32 cmd_trace_addr;
	u32 func_trace_addr;
	u32 fw_dump_addr;
	u32 fiq_dump_addr;
	u32 fiq_trace_addr;
};

struct fwd_sys_cpu_load_config {
	u32 enable;
};

struct fwd_sys_cpu_load {
	u32 stat_total_time;
	u32 cpu_active_time;
	u32 cpu_sleep_time;
	u32 event_proc_time;
	u32 event_proc_count;
	u32 irq_proc_time;
	u32 irq_proc_count;
	u32 fiq_proc_time;
	u32 fiq_proc_count;
	u32 sub_event_proc_time[32];
	u32 sub_event_proc_count[32];
	u32 sub_irq_proc_time[32];
	u32 sub_irq_proc_count[32];
};

#define FWD_SYS_CPU_LOAD_CAP_MAX_NUM	(180)
#define FWD_SYS_CPU_LOAD_CAT_MAX_NUM	(20)
struct fwd_sys_cpu_load_store {
	u32 ind_write_idx;
	u32 cat_read_idx;
	struct fwd_sys_cpu_load cap[FWD_SYS_CPU_LOAD_CAP_MAX_NUM];
};

struct fwd_sys_frame_trace {
	u32 frames_record[32];
	u32 frames_index;
};

struct fwd_sys_cmd_trace {
	u16 cmds_record[16];
	u32 cmd_index;
	u32 cmd_current;
};

struct fwd_sys_func_trace {
	u32 handler[32];
	u32 info[32];
	u32 stamp[32];
	u16 idx;
	u16 stage;
	u32 lr;
};

enum FWD_SYS_FUNC_TRACE_PLACE_NUM {
	FWD_SYS_FUNC_TRACE_PLACE_EVENT,
	FWD_SYS_FUNC_TRACE_PLACE_TIMER,
	FWD_SYS_FUNC_TRACE_PLACE_IRQ,
	FWD_SYS_FUNC_TRACE_PLACE_FIQ,
	FWD_SYS_FUNC_TRACE_PLACE_CPUIDLE,
	FWD_SYS_FUNC_TRACE_PLACE_FIRSTUP,
	FWD_SYS_FUNC_TRACE_PLACE_GOTODOWN,
	FWD_SYS_FUNC_TRACE_PLACE_WAKEUP,
};

#define FWD_SYS_FUNC_TRACE_MAKE_INFO_MASK	(0xFF00FF00)
#define FWD_SYS_FUNC_TRACE_GET_PLACE(_info)	(((_info) >> 16) & 0xFF)
#define FWD_SYS_FUNC_TRACE_GET_PROIRITY(_info)	(((_info) >> 0) & 0xFF)

#define FWD_SYS_FUNC_TRACE_STAGE_EVENT		(1 << FWD_SYS_FUNC_TRACE_PLACE_EVENT)
#define FWD_SYS_FUNC_TRACE_STAGE_TIMER		(1 << FWD_SYS_FUNC_TRACE_PLACE_TIMER)
#define FWD_SYS_FUNC_TRACE_STAGE_IRQ		(1 << FWD_SYS_FUNC_TRACE_PLACE_IRQ)
#define FWD_SYS_FUNC_TRACE_STAGE_FIQ		(1 << FWD_SYS_FUNC_TRACE_PLACE_FIQ)
#define FWD_SYS_FUNC_TRACE_STAGE_CPUIDLE	(1 << FWD_SYS_FUNC_TRACE_PLACE_CPUIDLE)
#define FWD_SYS_FUNC_TRACE_STAGE_FIRSTUP	(1 << FWD_SYS_FUNC_TRACE_PLACE_FIRSTUP)
#define FWD_SYS_FUNC_TRACE_STAGE_GOTODOWN	(1 << FWD_SYS_FUNC_TRACE_PLACE_GOTODOWN)
#define FWD_SYS_FUNC_TRACE_STAGE_WAKEUP		(1 << FWD_SYS_FUNC_TRACE_PLACE_WAKEUP)

struct fwd_sys_fw_dump {
	u32 state;
	u32 addr;
	u32 data;
};

#define FWD_SYS_FW_DUMP_STATE_WAIT		(0x1 << 0)
#define FWD_SYS_FW_DUMP_STATE_REQUEST		(0x1 << 1)
#define FWD_SYS_FW_DUMP_STATE_DONE		(0x1 << 2)

#define FWD_SYS_FW_DUMP_FLAG_FRAME		(0x1 << 0)
#define FWD_SYS_FW_DUMP_FLAG_CMD		(0x1 << 1)
#define FWD_SYS_FW_DUMP_FLAG_FUNC		(0x1 << 2)
#define FWD_SYS_FW_DUMP_FLAG_FW_BIN		(0x1 << 3)
#define FWD_SYS_FW_DUMP_FLAG_FIQ_DUMP		(0x1 << 4)
#define FWD_SYS_FW_DUMP_FLAG_FIQ_TRACE		(0x1 << 5)

struct fwd_soc_lpclk_stat {
	u32 state;
	s32 tsf_diff_sum;
	u32 tsf_diff_max;
	s32 tsf_diff_cur;
	u32 tsf_diff_cnt;
	u32 dpll_cnt_per_cal;
	u32 lpclk_cal_period;
	u32 suspend_time_cur;
	u32 resume_time_cur;
};

struct fwd_pas_fiq_dump_cfg {
	u32 dump_type;
	u32 dump_operation;
	u32 dump_position;
	u32 dump_max_cnt;
};

enum FWD_PAS_FIQ_DUMP_TYPE {
	FWD_PAS_FIQ_DUMP_TYPE_ONE_SHOT,
	FWD_PAS_FIQ_DUMP_TYPE_POSITION,
};

enum FWD_PAS_FIQ_DUMP_OPERATION {
	FWD_PAS_FIQ_DUMP_OPERATION_DISABLE,
	FWD_PAS_FIQ_DUMP_OPERATION_COUNTER,
	FWD_PAS_FIQ_DUMP_OPERATION_ALWAYS,
};

#define FWD_PAS_FIQ_DUMP_MAX_NUM 32
#define FWD_PAS_FIQ_DUMP_REG_NUM 7
#define FWD_PAS_FIQ_DUMP_STATUS__STORE_REG		(0x0)
#define FWD_PAS_FIQ_DUMP_STATUS__TO_HOST		(0x1)
#define FWD_PAS_FIQ_DUMP_IDX_STATUS__NOT_FULL		(0x0)
#define FWD_PAS_FIQ_DUMP_IDX_STATUS__FULL		(0x1)

struct fwd_pas_fiq_capture {
	u8 dump_status;
	u8 idx_status;
	u16 next_write_idx;
	u32 last_dump_time;
	u32 regs[FWD_PAS_FIQ_DUMP_MAX_NUM][FWD_PAS_FIQ_DUMP_REG_NUM];
};

struct fwd_pas_fiq_dump {
	u32 dump_position;
	struct fwd_pas_fiq_capture cap;
};

#define FWD_PAS_FIQ_TRACE_MAX_NUM 32
#define FWD_PAS_FIQ_TRACE_REG_NUM 6
#define FWD_PAS_FIQ_TRACE_STATUS__STORE_REG		(0x0)
#define FWD_PAS_FIQ_TRACE_STATUS__TO_HOST		(0x1)
#define FWD_PAS_FIQ_TRACE_IDX_STATUS__NOT_FULL		(0x0)
#define FWD_PAS_FIQ_TRACE_IDX_STATUS__FULL		(0x1)

struct fwd_pas_fiq_trace {
	u8 dump_status;
	u8 idx_status;
	u16 next_write_idx;
	u32 fiq_last_end_time;
	u32 regs[FWD_PAS_FIQ_TRACE_MAX_NUM][FWD_PAS_FIQ_TRACE_REG_NUM];
};

struct fwd_pas_hw_reg {
	u32 ntd_control;
	u32 txc_status;
	u32 rxc_rx_control;
	u32 rxc_rx_buf_in_pointer;
	u32 rxc_rx_buf_out_pointer;
	u32 rxc_buffer_size;
	u32 rxc_error_code;
	u32 rxc_rx_err_cnt0;
	u32 rxc_rx_err_cnt1;
	u32 rxc_rx_err_cnt2;
	u32 rxc_rx_delimiter_err_cnt;
	u32 rxc_rx_state;
	u32 tim_ebm_stat;
	u32 tim_ebm_stat_2;
	u32 tim_ebm_stat_3;
};

struct fwd_pas_hw_stat {
	u32 fiq_count;
	u32 rx_event_count;
	u32 write_port_count;
	u32 tx_success_count;
	u32 tx_failure_count;
	u32 rx_frame_stored_count;
	u32 rx_frame_error_count;
	u32 rx_beacon_count;
	u32 rx_multicast_count;
};

struct fwd_pas_ptcs_dump_cfg {
	u32 dump_position;
	u32 dump_operation;
	u32 dump_max_cnt;
};

enum FWD_PAS_PTCS_DUMP_TYPE {
	FWD_PAS_PTCS_DUMP_TYPE_GBM,
	FWD_PAS_PTCS_DUMP_TYPE_EBM,
};

enum FWD_PAS_PTCS_DUMP_OPERATION {
	FWD_PAS_PTCS_DUMP_OPERATION_DISABLE,
	FWD_PAS_PTCS_DUMP_OPERATION_COUNTER,
	FWD_PAS_PTCS_DUMP_OPERATION_ALWAYS,
};

#define FWD_PAS_PTCS_DUMP_POSITION_PREWRITE		(1<<0)
#define FWD_PAS_PTCS_DUMP_POSITION_AGGR_PTCS		(1<<1)
#define FWD_PAS_PTCS_DUMP_POSITION_AGGR_BUF		(1<<2)
#define FWD_PAS_PTCS_DUMP_POSITION_BURST_PTCS		(1<<3)

#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_DATA0		(1<<4)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_DATA1		(1<<5)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_BEACON		(1<<6)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_NULL		(1<<7)

#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_ACK0		(1<<8)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_ACK1		(1<<9)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_CTS0		(1<<10)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_CTS1		(1<<11)

#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_BA0		(1<<12)
#define FWD_PAS_PTCS_DUMP_POSITION_SM_TX_BA1		(1<<13)

struct fwd_pas_ptcs_dump {
	u32 dump_position;
	u32 dump_data[256];
};

struct fwd_pas_force_mode_cfg {
	u32 force_mode_select;
	u32 tx_scheme_type;
	u32 protection_type;
};

#define FWD_PAS_FORCE_MODE_SELECT_RESET			(0)
#define FWD_PAS_FORCE_MODE_SELECT_TX_SCHEME		(1<<0)
#define FWD_PAS_FORCE_MODE_SELECT_PROTECTION		(1<<1)

enum FWD_PAS_FORCE_MODE_TX_SCHEME_TYPE {
	FWD_PAS_FORCE_MODE_TX_SCHEME_TYPE_DEFAULT,
	FWD_PAS_FORCE_MODE_TX_SCHEME_TYPE_IGNORE_2ND_CCA,
	FWD_PAS_FORCE_MODE_TX_SCHEME_TYPE_REGARD_2ND_CCA,
};

enum FWD_PAS_FORCE_MODE_PROTECTION_TYPE {
	FWD_PAS_FORCE_MODE_PROTECTION_TYPE_DEFAULT,
	FWD_PAS_FORCE_MODE_PROTECTION_TYPE_USE_ERP_PROT,
	FWD_PAS_FORCE_MODE_PROTECTION_TYPE_USE_HT_PROT,
};

struct fwd_pas_dur_entry_cfg {
	u32 dur_idx;
};

struct fwd_pas_dur_entry {
	u32 resp_rate_dur_adj;
	u32 resp_mode_dur_adj;
	u32 burst_length_0;
	u32 burst_length_1;
};

struct fwd_pas_force_tx_cfg {
	u16 if_id;
	u16 rate_entry;
};

#define FWD_EPTA_TIME_LINE_DOTS		(64)
#define FWD_EPTA_TIME_LINE_WIDTH	(4)
#define FWD_EPTA_TIME_LINE_DATA_NUM	(FWD_EPTA_TIME_LINE_DOTS * FWD_EPTA_TIME_LINE_WIDTH)

#define FWD_EPTA_TIME_LINE_FILE_MAX_SIZE (1024*1024*1024)

struct fwd_epta_time_line_dump {
	u32 tl_data_len;
	u32 tl_data[FWD_EPTA_TIME_LINE_DATA_NUM];
};

#define FWD_EPTA_TIME_LINE_CFG_OPERATION_READ		(0x0)
#define FWD_EPTA_TIME_LINE_CFG_OPERATION_WRITE		(0x1)

#define FWD_EPTA_TIME_LINE_CFG_CMD_SET_FW		(0x0)
#define FWD_EPTA_TIME_LINE_CFG_CMD_RESET_FILE		(0x1)

struct fwd_epta_time_line_cfg {
	u32 operation;
	u32 rf_switch_ctrl;
};


#define FWD_EPTA_RF_STAT_RECORD_ORIGIN_DATA		0
#define FWD_EPTA_RF_STAT_RECORD_PARSED_DATA		1

#define FWD_EPTA_RF_STAT_POINT_NUM			65
#define FWD_EPTA_RF_STAT_INFO_NUM			6
#define FWD_EPTA_RF_HEAD_BUF_LEN			1500
#define FWD_EPTA_RF_DATA_BUF_LEN			(150 * FWD_EPTA_RF_STAT_POINT_NUM)
#define FWD_EPTA_RF_STAT_FILE_MAX_SIZE			(1024 * 1024 * 1024)
#define FWD_EPTA_RF_STAT_BT_TX_TYPE_NUM			40

#define FWD_EPTA_RF_STAT_GET_DURATION(a, b)		(((a) <= (b)) ? ((b) - (a)) \
											 : (((0xffffffff) - (a)) + (b)))
#define FWD_EPTA_RF_STAT_GET_BT_TYPE(bt_type)		((bt_type) ? ("tx") : ("rx"))

#define FWD_EPTA_RF_STAT_WLAN2BT_NOW_BT			(1)
#define FWD_EPTA_RF_STAT_WLAN2BT_NOW_WLAN		(2)
#define FWD_EPTA_RF_STAT_BT2WLAN_NOW_WLAN		(3)
#define FWD_EPTA_RF_STAT_BT2WLAN_NOW_BT			(4)
#define FWD_EPTA_RF_STAT_MISS_INFO			(5)
#define FWD_EPTA_RF_STAT_WLAN_DUR_TXRX_INFO		(6)

#define FWD_EPTA_RF_STAT_SUB_JUST_BT2WLAN		(0)
#define FWD_EPTA_RF_STAT_SUB_WLAN_ACTIVE		(1)
#define FWD_EPTA_RF_STAT_SUB_WLAN_INACTIVE		(2)
#define FWD_EPTA_RF_STAT_SUB_BT_REQ_NUM			(3)

#define FWD_EPTA_RF_STAT_MISS_DBG_IRQ			(1)
#define FWD_EPTA_RF_STAT_MISS_DBG_IRQ_STATUS		(2)
#define FWD_EPTA_RF_STAT_MISS_HIF_SEND			(3)

#define FWD_EPTA_RF_STAT_WLAN_DUR			(1)
#define FWD_EPTA_RF_STAT_IDLE_DUR			(2)

#define FWD_EPTA_RF_STAT_CFG_OPERATION_READ		(0x0)
#define FWD_EPTA_RF_STAT_CFG_OPERATION_WRITE		(0x1)

#define FWD_EPTA_RF_STAT_CFG_CMD_START_INFO		(0x0001)
#define FWD_EPTA_RF_STAT_CFG_CMD_RESET_FILE		(0xffff)


struct fwd_epta_rf_stat_cfg {
	u32 operation;
	u32 on_off_ctrl;
};

struct fwd_epta_rf_stat_record {
	u32 epta_data0_l;
	u32 epta_data1_l;
	u32 epta_data2_l;
	u32 epta_data3_l;
	u32 epta_data4_l;
	u32 epta_data5_l;
	u32 epta_index;
	u32 epta_rf_first;
	u32 epta_wl_type;
	u32 epta_wl_prio;
	char buffer[FWD_EPTA_RF_DATA_BUF_LEN];
	char head_buf[FWD_EPTA_RF_HEAD_BUF_LEN];
	char wlan_req_type[11][20];
	struct xr_file *origin_file;
	struct xr_file *parsed_file;
	struct semaphore epta_rf_stat_sema;
};

struct fwd_epta_rf_stat_dump {
	u32 rf_stat_len;
	u32 rf_stat_record[FWD_EPTA_RF_STAT_INFO_NUM][FWD_EPTA_RF_STAT_POINT_NUM];
};


struct fwd_common {
	u32 sys_dump_flag;
	struct fwd_sys_config sys_config;
	struct fwd_sys_cpu_load_store sys_cpu_load_store;
	struct xr_file *epta_time_line_file;
	struct semaphore epta_time_line_sema;
	int epta_time_line_file_size;
	struct fwd_epta_rf_stat_record epta_rf_stat;
};

int xradio_fw_dbg_init(struct xradio_common *hw_priv);
void xradio_fw_dbg_deinit(void);
int xradio_fw_dbg_confirm(void *buf_data, void *arg);
int xradio_fw_dbg_indicate(void *buf_data);
void xradio_fw_dbg_dump_in_direct_mode(struct xradio_common *hw_priv);
void xradio_fw_dbg_set_dump_flag_on_fw_exception(void);

#else

static inline int xradio_fw_dbg_init(struct xradio_common *hw_priv)
{
	return 0;
}

static inline void xradio_fw_dbg_deinit(void)
{
}

static inline int xradio_fw_dbg_confirm(void *buf_data, void *arg)
{
	return 0;
}

static inline int xradio_fw_dbg_indicate(void *buf_data)
{
	return 0;
}

#endif
