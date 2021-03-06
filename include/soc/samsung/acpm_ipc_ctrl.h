#ifndef __ACPM_IPC_CTRL_H__
#define __ACPM_IPC_CTRL_H__

typedef void (*ipc_callback)(unsigned int *cmd, unsigned int size);

struct ipc_config {
	unsigned int *cmd;
	unsigned int *indirection_base;
	unsigned int indirection_size;
	bool response;
	bool indirection;
};

#define ACPM_IPC_PROTOCOL_OWN			(31)
#define ACPM_IPC_PROTOCOL_RSP			(30)
#define ACPM_IPC_PROTOCOL_INDIRECTION		(29)
#define ACPM_IPC_PROTOCOL_STOP_WDT		(27)
#define ACPM_IPC_PROTOCOL_ID			(26)
#define ACPM_IPC_PROTOCOL_IDX			(0x7 << ACPM_IPC_PROTOCOL_ID)
#define ACPM_IPC_PROTOCOL_DP_A			(25)
#define ACPM_IPC_PROTOCOL_DP_D			(24)
#define ACPM_IPC_PROTOCOL_DP_CMD		(0x3 << ACPM_IPC_PROTOCOL_DP_D)
#define ACPM_IPC_PROTOCOL_TEST			(23)
#define ACPM_IPC_PROTOCOL_STOP			(22)
#define ACPM_IPC_PROTOCOL_SEQ_NUM		(16)

struct nfc_clk_req_log {
	unsigned int is_on;
	unsigned int timestamp;
};

#if defined(CONFIG_EXYNOS_ACPM) || defined(CONFIG_EXYNOS_ACPM_MODULE)
extern unsigned int acpm_ipc_request_channel(struct device_node *np, ipc_callback handler,
		unsigned int *id, unsigned int *size);
extern unsigned int acpm_ipc_release_channel(struct device_node *np, unsigned int channel_id);
extern int acpm_ipc_send_data(unsigned int channel_id, struct ipc_config *cfg);
extern int acpm_ipc_send_data_sync(unsigned int channel_id, struct ipc_config *cfg);
extern int acpm_ipc_set_ch_mode(struct device_node *np, bool polling);
extern void exynos_acpm_force_apm_wdt_reset(void);
extern void acpm_stop_log(void);
extern ktime_t acpm_time_calc(u32 start, u32 end);
extern u32 acpm_get_peri_timer(void);
extern void exynos_acpm_set_fast_switch(unsigned int ch);
extern bool is_acpm_ipc_busy(unsigned ch_id);
extern int acpm_get_nfc_log_buf(struct nfc_clk_req_log **buf, u32 *last_ptr, u32 *len);
#else

static inline unsigned int acpm_ipc_request_channel(struct device_node *np, ipc_callback handler,
		unsigned int *id, unsigned int *size)
{
	return 0;
}

static inline unsigned int acpm_ipc_release_channel(struct device_node *np, unsigned int channel_id)
{
	return 0;
}

static inline int acpm_ipc_send_data(unsigned int channel_id, struct ipc_config *cfg)
{
	return 0;
}

static inline int acpm_ipc_send_data_sync(unsigned int channel_id, struct ipc_config *cfg)
{
	return 0;
}

static inline int acpm_ipc_set_ch_mode(struct device_node *np, bool polling)
{
	return 0;
}

static inline void exynos_acpm_reboot(void)
{
	return;
}

static inline void exynos_acpm_force_wdt_reset(void)
{
	return;
}

static inline void acpm_stop_log(void)
{
	return;
}

static inline ktime_t acpm_time_calc(u32 start, u32 end)
{
	return 0;
}
static inline u32 acpm_get_peri_timer(void)
{
	return 0;
}
static inline void exynos_acpm_set_fast_switch(unsigned int ch)
{
}
bool is_acpm_ipc_busy(unsigned ch_id)
{
	return false;
}
static inline int acpm_get_nfc_log_buf(struct nfc_clk_req_log **buf, u32 *last_ptr, u32 *len)
{
		return 0;
}
#endif

#endif
