/******************************************************************************
 *
 * Copyright (c) 2012 - 2019 Samsung Electronics Co., Ltd. All rights reserved
 *
 *****************************************************************************/

#ifndef SLSI_UTILS_H__
#define SLSI_UTILS_H__

#include <linux/version.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <net/cfg80211.h>

#include "netif.h"

#ifdef CONFIG_SCSC_SMAPPER
struct slsi_skb_cb {
	u32 sig_length;
	u32 data_length;
	u32 frame_format;
	u32 colour;
	bool free_ma_unitdat;
	struct sk_buff *skb_addr;
};

static inline struct slsi_skb_cb *slsi_skb_cb_get(struct sk_buff *skb)
{
	return (struct slsi_skb_cb *)skb->cb;
}

static inline struct slsi_skb_cb *slsi_skb_cb_init(struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(struct slsi_skb_cb) > sizeof(skb->cb));

	memset(skb->cb, 0, sizeof(struct slsi_skb_cb));
	return slsi_skb_cb_get(skb);
}
#endif

static inline u32  slsi_convert_tlv_data_to_value(u8 *data, u16 length)
{
	u32 value = 0;
	int i;

	if (length > 4)
		return 0;
	for (i = 0; i < length; i++)
		value |= ((u32)data[i]) << i * 8;

	return value;
}

#ifdef __cplusplus
extern "C" {
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
#define SLSI_ETHER_COPY(dst, src)	ether_addr_copy((dst), (src))
#define SLSI_ETHER_EQUAL(mac1, mac2)	ether_addr_equal((mac1), (mac2))
#else
#define SLSI_ETHER_COPY(dst, src)	memcpy((dst), (src), ETH_ALEN)
#define SLSI_ETHER_EQUAL(mac1, mac2)	(memcmp((mac1), (mac2), ETH_ALEN) == 0)
#endif

extern uint slsi_sg_host_align_mask;
#define SLSI_HIP_FH_SIG_PREAMBLE_LEN 4
#define SLSI_SKB_GET_ALIGNMENT_OFFSET(skb) (0)

/* Get the Compiler to ignore Unused parameters */
#define SLSI_UNUSED_PARAMETER(x) ((void)(x))

/* Helper ERROR Macros */
#define SLSI_ECR(func) \
	do { \
		int _err = (func); \
		if (_err != 0) { \
			SLSI_ERR_NODEV("e=%d\n", _err); \
			return _err; \
		} \
	} while (0)

#define SLSI_EC(func) \
	do { \
		int _err = (func); \
		if (_err != 0) { \
			SLSI_ERR_NODEV("e=%d\n", _err); \
			return; \
		} \
	} while (0)

#define SLSI_EC_GOTO(func, err, label) \
	do { \
		(err) = func; \
		if ((err) != 0) { \
			WARN_ON(1); \
			SLSI_ERR(sdev, "fail at line:%d\n", __LINE__); \
			goto label; \
		} \
	} while (0)

/*------------------------------------------------------------------*/
/* Endian conversion. */
/*------------------------------------------------------------------*/
#define SLSI_BUFF_LE_TO_U16(ptr)        (((u16)((u8 *)(ptr))[0]) | ((u16)((u8 *)(ptr))[1]) << 8)
#define SLSI_U16_TO_BUFF_LE(uint, ptr) \
	do { \
		u32 local_uint_tmp = (uint); \
		((u8 *)(ptr))[0] = ((u8)((local_uint_tmp & 0x00FF))); \
		((u8 *)(ptr))[1] = ((u8)(local_uint_tmp >> 8)); \
	} while (0)

#define SLSI_U32_TO_BUFF_LE(uint, ptr) ((*(u32 *)ptr) = cpu_to_le32(uint))

#define SLSI_BUFF_LE_TO_U16_P(output, input) \
	do { \
		(output) = (u16)((((u16)(input)[1]) << 8) | ((u16)(input)[0])); \
		(input) += 2; \
	} while (0)

#define SLSI_BUFF_LE_TO_U32_P(output, input) \
	do { \
		(output) = le32_to_cpu(*(u32 *)input); \
		(input) += 4; \
	} while (0)

#define SLSI_U16_TO_BUFF_LE_P(output, input) \
	do { \
		(output)[0] = ((u8)((input) & 0x00FF));  \
		(output)[1] = ((u8)((input) >> 8)); \
		(output) += 2; \
	} while (0)

#define SLSI_U32_TO_BUFF_LE_P(output, input) \
	do { \
		(*(u32 *)output) = cpu_to_le32(input); \
		(output) += 4; \
	} while (0)

/* Android wakelock abstraction */
#ifdef CONFIG_SCSC_WLAN_ANDROID
#define slsi_wake_lock_init(lock, type, name)	wake_lock_init(lock, type, name)
#define slsi_wake_lock(lock)					wake_lock(lock)
#define slsi_wake_unlock(lock)					wake_unlock(lock)
#define slsi_wake_lock_timeout(lock, timeout)	wake_lock_timeout(lock, timeout)
#define slsi_wake_lock_active(lock)				wake_lock_active(lock)
#define slsi_wake_lock_destroy(lock)			wake_lock_destroy(lock)
#else
#define slsi_wake_lock_init(lock, type, name)
#define slsi_wake_lock(lock)
#define slsi_wake_unlock(lock)
#define slsi_wake_lock_timeout(lock, timeout)
#define slsi_wake_lock_active(lock)				(false)
#define slsi_wake_lock_destroy(lock)
#endif

#ifdef CONFIG_SCSC_WLAN_SKB_TRACKING
void slsi_dbg_track_skb_init(void);
void slsi_dbg_track_skb_reset(void);
void slsi_dbg_track_skb_f(struct sk_buff *skb, gfp_t flags, const char *file, int line);
bool slsi_dbg_untrack_skb_f(struct sk_buff *skb, const char *file, int line);
bool slsi_dbg_track_skb_marker_f(struct sk_buff *skb, const char *file, int line);
#define slsi_dbg_track_skb(skb_, flags_) slsi_dbg_track_skb_f(skb_, flags_, __FILE__, __LINE__)
#define slsi_dbg_untrack_skb(skb_) slsi_dbg_untrack_skb_f(skb_, __FILE__, __LINE__)
#define slsi_dbg_track_skb_marker(skb_) slsi_dbg_track_skb_marker_f(skb_, __FILE__, __LINE__)
void slsi_dbg_track_skb_report(void);

static inline struct sk_buff *slsi_dev_alloc_skb_f(unsigned int length, const char *file, int line)
{
	struct sk_buff *skb = dev_alloc_skb(SLSI_NETIF_SKB_HEADROOM + SLSI_NETIF_SKB_TAILROOM + length);

	if (skb) {
#ifdef CONFIG_SCSC_SMAPPER
		slsi_skb_cb_init(skb);
#endif
		skb_reserve(skb, SLSI_NETIF_SKB_HEADROOM - SLSI_SKB_GET_ALIGNMENT_OFFSET(skb));
		slsi_dbg_track_skb_f(skb, GFP_ATOMIC, file, line);
	}
	return skb;
}

static inline struct sk_buff *slsi_alloc_skb_f(unsigned int size, gfp_t priority, const char *file, int line)
{
	struct sk_buff *skb = alloc_skb(size, priority);

	if (skb) {
#ifdef CONFIG_SCSC_SMAPPER
		slsi_skb_cb_init(skb);
#endif
		slsi_dbg_track_skb_f(skb, priority, file, line);
	}

	return skb;
}

static inline struct sk_buff *slsi_alloc_skb_headroom_f(unsigned int size, gfp_t priority, const char *file, int line)
{
	struct sk_buff *skb = alloc_skb(SLSI_NETIF_SKB_HEADROOM + SLSI_NETIF_SKB_TAILROOM + size, priority);

	if (skb) {
#ifdef CONFIG_SCSC_SMAPPER
		slsi_skb_cb_init(skb);
#endif
		skb_reserve(skb, SLSI_NETIF_SKB_HEADROOM - SLSI_SKB_GET_ALIGNMENT_OFFSET(skb));
		slsi_dbg_track_skb_f(skb, priority, file, line);
	}
	return skb;
}

static inline void slsi_skb_unlink_f(struct sk_buff *skb, struct sk_buff_head *list, const char *file, int line)
{
	skb_unlink(skb, list);
	slsi_dbg_track_skb_marker_f(skb, file, line);
}

static inline void slsi_skb_queue_tail_f(struct sk_buff_head *list, struct sk_buff *skb, const char *file, int line)
{
	skb_queue_tail(list, skb);
	slsi_dbg_track_skb_marker_f(skb, file, line);
}

static inline void slsi_skb_queue_head_f(struct sk_buff_head *list, struct sk_buff *skb, const char *file, int line)
{
	skb_queue_head(list, skb);
	slsi_dbg_track_skb_marker_f(skb, file, line);
}

static inline struct sk_buff *slsi_skb_dequeue_f(struct sk_buff_head *list, const char *file, int line)
{
	struct sk_buff *skb = skb_dequeue(list);

	if (skb)
		slsi_dbg_track_skb_marker_f(skb, file, line);
	return skb;
}

static inline struct sk_buff *slsi_skb_realloc_headroom_f(struct sk_buff *skb, unsigned int headroom, const char *file, int line)
{
	skb = skb_realloc_headroom(skb, headroom);
	if (skb) {
#ifdef CONFIG_SCSC_SMAPPER
		slsi_skb_cb_init(skb);
#endif
		slsi_dbg_track_skb_f(skb, GFP_ATOMIC, file, line);
	}
	return skb;
}

static inline struct sk_buff *slsi_skb_copy_f(struct sk_buff *skb, gfp_t priority, const char *file, int line)
{
	skb = skb_copy(skb, priority);

	if (skb)
		slsi_dbg_track_skb_f(skb, priority, file, line);
	return skb;
}

static inline struct sk_buff *skb_copy_expand_f(struct sk_buff *skb, int newheadroom, int newtailroom, gfp_t priority, const char *file, int line)
{
	skb = skb_copy_expand(skb, newheadroom, newtailroom, priority);

	if (skb)
		slsi_dbg_track_skb_f(skb, priority, file, line);
	return skb;
}

static inline struct sk_buff *slsi_skb_clone_f(struct sk_buff *skb, gfp_t priority, const char *file, int line)
{
	skb = skb_clone(skb, priority);

	if (skb)
		slsi_dbg_track_skb_f(skb, priority, file, line);
	return skb;
}

static inline void slsi_kfree_skb_f(struct sk_buff *skb, const char *file, int line)
{
	/* If untrack fails we do not free the SKB
	 * This helps tracking bad pointers and double frees
	 */
#ifdef CONFIG_SCSC_SMAPPER
	struct slsi_skb_cb *cb;

	if (!skb)
		return;

	cb = (struct slsi_skb_cb *)skb->cb;

	if (cb && !cb->free_ma_unitdat && cb->skb_addr && slsi_dbg_untrack_skb_f(cb->skb_addr, file, line)) {
		kfree_skb(cb->skb_addr);
		cb->skb_addr = NULL;
	}
#endif
	if (slsi_dbg_untrack_skb_f(skb, file, line))
		kfree_skb(skb);
}

#define slsi_dev_alloc_skb(length_)      slsi_dev_alloc_skb_f(length_, __FILE__, __LINE__)
#define slsi_alloc_skb(size_, priority_) slsi_alloc_skb_f(size_, priority_, __FILE__, __LINE__)
#define slsi_alloc_skb_headroom(size_, priority_) slsi_alloc_skb_headroom_f(size_, priority_, __FILE__, __LINE__)
#define slsi_skb_realloc_headroom(skb_, headroom_)  slsi_skb_realloc_headroom_f(skb_, headroom_, __FILE__, __LINE__)
#define slsi_skb_copy(skb_, priority_)   slsi_skb_copy_f(skb_, priority_, __FILE__, __LINE__)
#define slsi_skb_copy_expand(skb_, newheadroom_, newtailroom_, priority_) skb_copy_expand_f(skb_, newheadroom_, newtailroom_, priority_, __FILE__, __LINE__)
#define slsi_skb_clone(skb_, priority_)  slsi_skb_clone_f(skb_, priority_, __FILE__, __LINE__)
#define slsi_kfree_skb(skb_)             slsi_kfree_skb_f(skb_, __FILE__, __LINE__)
#define slsi_skb_unlink(skb_, list_)     slsi_skb_unlink_f(skb_, list_, __FILE__, __LINE__)
#define slsi_skb_queue_tail(list_, skb_) slsi_skb_queue_tail_f(list_, skb_, __FILE__, __LINE__)
#define slsi_skb_queue_head(list_, skb_) slsi_skb_queue_head_f(list_, skb_, __FILE__, __LINE__)
#define slsi_skb_dequeue(list_)          slsi_skb_dequeue_f(list_, __FILE__, __LINE__)

static inline void slsi_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		slsi_kfree_skb(skb);
}

#else
#define slsi_dbg_track_skb_init()
#define slsi_dbg_track_skb_reset()
#define slsi_dbg_track_skb(skb_, flags_)
#define slsi_dbg_untrack_skb(skb_)
#define slsi_dbg_track_skb_marker(skb_)
#define slsi_dbg_track_skb_report()

static inline struct sk_buff *slsi_dev_alloc_skb_f(unsigned int length, const char *file, int line)
{
	struct sk_buff *skb = dev_alloc_skb(SLSI_NETIF_SKB_HEADROOM + SLSI_NETIF_SKB_TAILROOM + length);

	SLSI_UNUSED_PARAMETER(file);
	SLSI_UNUSED_PARAMETER(line);
	if (skb) {
#ifdef CONFIG_SCSC_SMAPPER
		slsi_skb_cb_init(skb);
#endif
		skb_reserve(skb, SLSI_NETIF_SKB_HEADROOM - SLSI_SKB_GET_ALIGNMENT_OFFSET(skb));
	}
	return skb;
}

static inline struct sk_buff *slsi_alloc_skb_f(unsigned int size, gfp_t priority, const char *file, int line)
{
	struct sk_buff *skb = alloc_skb(size, priority);

	SLSI_UNUSED_PARAMETER(file);
	SLSI_UNUSED_PARAMETER(line);
#ifdef CONFIG_SCSC_SMAPPER
	if (skb)
		slsi_skb_cb_init(skb);
#endif
	return skb;
}

static inline struct sk_buff *slsi_alloc_skb_headroom_f(unsigned int size, gfp_t priority, const char *file, int line)
{
	struct sk_buff *skb = alloc_skb(SLSI_NETIF_SKB_HEADROOM + SLSI_NETIF_SKB_TAILROOM + size, priority);

	SLSI_UNUSED_PARAMETER(file);
	SLSI_UNUSED_PARAMETER(line);

	if (skb) {
#ifdef CONFIG_SCSC_SMAPPER
		slsi_skb_cb_init(skb);
#endif
		skb_reserve(skb, SLSI_NETIF_SKB_HEADROOM - SLSI_SKB_GET_ALIGNMENT_OFFSET(skb));
	}
	return skb;
}

static inline void slsi_kfree_skb_f(struct sk_buff *skb)
{
	/* If untrack fails we do not free the SKB
	 * This helps tracking bad pointers and double frees
	 */
#ifdef CONFIG_SCSC_SMAPPER
	struct slsi_skb_cb *cb;

	if (!skb)
		return;

	cb = (struct slsi_skb_cb *)skb->cb;

	if (cb && !cb->free_ma_unitdat && cb->skb_addr) {
		kfree_skb(cb->skb_addr);
		cb->skb_addr = NULL;
	}
#endif
	kfree_skb(skb);
}

#define slsi_dev_alloc_skb(length_)       slsi_dev_alloc_skb_f(length_, __FILE__, __LINE__)
#define slsi_alloc_skb(size_, priority_)  slsi_alloc_skb_f(size_, priority_, __FILE__, __LINE__)
#define slsi_alloc_skb_headroom(size_, priority_)  slsi_alloc_skb_headroom_f(size_, priority_, __FILE__, __LINE__)
#define slsi_skb_realloc_headroom(skb_, headroom_)       skb_realloc_headroom(skb_, headroom_)
#define slsi_skb_copy(skb_, priority_)    skb_copy(skb_, priority_)
#define slsi_skb_copy_expand(skb_, newheadroom_, newtailroom_, priority_) skb_copy_expand(skb_, newheadroom_, newtailroom_, priority_)
#define slsi_skb_clone(skb_, priority_)   skb_clone(skb_, priority_)
#define slsi_kfree_skb(skb_)              slsi_kfree_skb_f(skb_)
#define slsi_skb_unlink(skb_, list_)      skb_unlink(skb_, list_)
#define slsi_skb_queue_tail(list_, skb_)  skb_queue_tail(list_, skb_)
#define slsi_skb_queue_head(list_, skb_)  skb_queue_head(list_, skb_)
#define slsi_skb_dequeue(list_)           skb_dequeue(list_)
#define slsi_skb_queue_purge(list_)       slsi_skb_queue_purge(list_)

static inline void slsi_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		slsi_kfree_skb(skb);
}

#endif

struct slsi_spinlock {
	/* a std spinlock */
	spinlock_t    lock;
	unsigned long flags;
};

/* Spinlock create can't fail, so return success regardless. */
static inline void slsi_spinlock_create(struct slsi_spinlock *lock)
{
	spin_lock_init(&lock->lock);
}

static inline void slsi_spinlock_lock(struct slsi_spinlock *lock)
{
	spin_lock_bh(&lock->lock);
}

static inline void slsi_spinlock_unlock(struct slsi_spinlock *lock)
{
	spin_unlock_bh(&lock->lock);
}

struct slsi_dev;
struct slsi_skb_work {
	struct slsi_dev         *sdev;
	struct net_device       *dev;   /* This can be NULL */
	struct workqueue_struct *workqueue;
	struct work_struct      work;
	struct sk_buff_head     queue;
	void __rcu              *sync_ptr;
};

static inline int slsi_skb_work_init(struct slsi_dev *sdev, struct net_device *dev, struct slsi_skb_work *work, const char *name, void (*func)(struct work_struct *work))
{
	rcu_assign_pointer(work->sync_ptr, (void *)sdev);
	work->sdev = sdev;
	work->dev = dev;
	skb_queue_head_init(&work->queue);
	INIT_WORK(&work->work, func);
	work->workqueue = alloc_ordered_workqueue(name, 0);

	if (!work->workqueue)
		return -ENOMEM;
	return 0;
}

static inline void slsi_skb_schedule_work(struct slsi_skb_work *work)
{
	queue_work(work->workqueue, &work->work);
}

static inline void slsi_skb_work_enqueue_l(struct slsi_skb_work *work, struct sk_buff *skb)
{
	void *sync_ptr;

	rcu_read_lock();

	sync_ptr = rcu_dereference(work->sync_ptr);

	if (WARN_ON(!sync_ptr)) {
		slsi_kfree_skb(skb);
		rcu_read_unlock();
		return;
	}
	skb_queue_tail(&work->queue, skb);
	slsi_skb_schedule_work(work);

	rcu_read_unlock();
}

static inline struct sk_buff *slsi_skb_work_dequeue_l(struct slsi_skb_work *work)
{
	return skb_dequeue(&work->queue);
}

static inline void slsi_skb_work_deinit(struct slsi_skb_work *work)
{
	rcu_read_lock();

	if (WARN_ON(!work->sync_ptr)) {
		rcu_read_unlock();
		return;
	}

	rcu_assign_pointer(work->sync_ptr, NULL);
	rcu_read_unlock();

	synchronize_rcu();
	flush_workqueue(work->workqueue);
	destroy_workqueue(work->workqueue);
	work->workqueue = NULL;
	slsi_skb_queue_purge(&work->queue);
}

static inline void slsi_cfg80211_put_bss(struct wiphy *wiphy, struct cfg80211_bss *bss)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
	cfg80211_put_bss(wiphy, bss);
#else
	cfg80211_put_bss(bss);
#endif  /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)) */
}

#ifdef CONFIG_SCSC_WLAN_SKB_TRACKING
static inline void slsi_skb_work_enqueue_f(struct slsi_skb_work *work, struct sk_buff *skb, const char *file, int line)
{
	slsi_dbg_track_skb_marker_f(skb, file, line);
	slsi_skb_work_enqueue_l(work, skb);
}

static inline struct sk_buff *slsi_skb_work_dequeue_f(struct slsi_skb_work *work, const char *file, int line)
{
	struct sk_buff *skb;

	skb = slsi_skb_work_dequeue_l(work);
	if (skb)
		slsi_dbg_track_skb_marker_f(skb, file, line);
	return skb;
}

#define slsi_skb_work_enqueue(work_, skb_) slsi_skb_work_enqueue_f(work_, skb_, __FILE__, __LINE__)
#define slsi_skb_work_dequeue(work_) slsi_skb_work_dequeue_f(work_, __FILE__, __LINE__)
#else
#define slsi_skb_work_enqueue(work_, skb_) slsi_skb_work_enqueue_l(work_, skb_)
#define slsi_skb_work_dequeue(work_) slsi_skb_work_dequeue_l(work_)
#endif

static inline void slsi_eth_zero_addr(u8 *addr)
{
	memset(addr, 0x00, ETH_ALEN);
}

static inline void slsi_eth_broadcast_addr(u8 *addr)
{
	memset(addr, 0xff, ETH_ALEN);
}

static inline int slsi_str_to_int(char *str, int *result)
{
	int i = 0;

	*result = 0;
	if ((str[i] == '-') || ((str[i] >= '0') && (str[i] <= '9'))) {
		if (str[0] == '-')
			i++;
		while (str[i] >= '0' && str[i] <= '9') {
			*result *= 10;
			*result += (int)str[i++] - '0';
		}

		*result = ((str[0] == '-') ? (-(*result)) : *result);
	}
	return i;
}

#define P80211_OUI_LEN		3

struct ieee80211_snap_hdr {
	u8 dsap;                /* always 0xAA */
	u8 ssap;                /* always 0xAA */
	u8 ctrl;                /* always 0x03 */
	u8 oui[P80211_OUI_LEN]; /* organizational universal id */
} __packed;

struct msdu_hdr {
	unsigned char             da[ETH_ALEN];
	unsigned char             sa[ETH_ALEN];
	__be16                    length;
	struct ieee80211_snap_hdr snap;
	__be16                    ether_type;
} __packed;

#define ETHER_TYPE_SIZE		2
#define MSDU_HLEN		sizeof(struct msdu_hdr)
#define MSDU_LENGTH		(sizeof(struct ieee80211_snap_hdr) + sizeof(__be16))

static inline int slsi_skb_msdu_to_ethhdr(struct sk_buff *skb)
{
	struct ethhdr   *eth;
	struct msdu_hdr *msdu;

	unsigned char   da[ETH_ALEN];
	unsigned char   sa[ETH_ALEN];
	__be16          proto;

	msdu = (struct msdu_hdr *)skb->data;
	SLSI_ETHER_COPY(da, msdu->da);
	SLSI_ETHER_COPY(sa, msdu->sa);
	proto = msdu->ether_type;

	skb_pull(skb, MSDU_HLEN);

	eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);

	SLSI_ETHER_COPY(eth->h_dest, da);
	SLSI_ETHER_COPY(eth->h_source, sa);
	eth->h_proto = proto;

	return 0;
}

static inline int slsi_skb_ethhdr_to_msdu(struct sk_buff *skb)
{
	struct ethhdr   *eth;
	struct msdu_hdr *msdu;
	unsigned int    len;
	__be16          ether_type;

	if (skb_headroom(skb) < (MSDU_HLEN - ETH_HLEN))
		return -EINVAL;

	eth = eth_hdr(skb);
	ether_type = eth->h_proto;

	len = skb->len;

	skb_pull(skb, ETH_HLEN);

	msdu = (struct msdu_hdr *)skb_push(skb, MSDU_HLEN);

	SLSI_ETHER_COPY(msdu->da, eth->h_dest);
	SLSI_ETHER_COPY(msdu->sa, eth->h_source);
	msdu->length = htons(len - ETH_HLEN + MSDU_LENGTH);
	memcpy(&msdu->snap, rfc1042_header, sizeof(struct ieee80211_snap_hdr));
	msdu->ether_type = ether_type;

	return 0;
}

static inline u32 slsi_get_center_freq1(struct slsi_dev *sdev, u16 chann_info, u16 center_freq)
{
	u32 center_freq1 = 0x0000;

	SLSI_UNUSED_PARAMETER(sdev);

	switch (chann_info & 0xFF) {
	case 40:
		center_freq1 = center_freq - 20 * ((chann_info & 0xFF00) >> 8) + 10;
		break;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 9))
	case 80:
		center_freq1 = center_freq - 20 * ((chann_info & 0xFF00) >> 8) + 30;
		break;
#endif
	default:
		break;
	}
	return center_freq1;
}

/* Name: strtoint
 * Desc: Converts a string to a decimal or hexadecimal integer
 * s: the string to be converted
 * res: pointer to the calculated integer
 * return: 0 (success), 1(failure)
 */
static inline int strtoint(const char *s, int *res)
{
	int base = 10;

	if (strlen(s) > 2)
		if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
			base = 16;
	return kstrtoint(s, base, res);
}

static inline u8 *slsi_mem_dup(u8 *src, size_t len)
{
	u8 *dest;

	dest = kmalloc(len, GFP_KERNEL);
	if (!dest)
		return NULL;
	memcpy(dest, src, len);
	return dest;
}

static inline void slsi_get_random_bytes(u8 *byte_buffer, u32 buffer_len)
{
	return get_random_bytes(byte_buffer, buffer_len);
}

static inline int slsi_util_nla_get_u8(const struct nlattr *attr, u8 *val)
{
	if (nla_len(attr) >= sizeof(u8)) {
		*val = nla_get_u8(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_u16(const struct nlattr *attr, u16 *val)
{
	if (nla_len(attr) >= sizeof(u16)) {
		*val = nla_get_u16(attr);
		return 0;
	}
	return -EINVAL;

}

static inline int slsi_util_nla_get_u32(const struct nlattr *attr, u32 *val)
{
	if (nla_len(attr) >= sizeof(u32)) {
		*val = nla_get_u32(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_u64(const struct nlattr *attr, u64 *val)
{
	if (nla_len(attr) >= sizeof(u64)) {
		*val = nla_get_u64(attr);
		return 0;
	}
	return -EINVAL;
}


static inline int slsi_util_nla_get_s8(const struct nlattr *attr, s8 *val)
{
	if (nla_len(attr) >= sizeof(s8)) {
		*val = nla_get_s8(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_s16(const struct nlattr *attr, s16 *val)
{
	if (nla_len(attr) >= sizeof(s16)) {
		*val = nla_get_s16(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_s32(const struct nlattr *attr, s32 *val)
{
	if (nla_len(attr) >= sizeof(s32)) {
		*val = nla_get_s32(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_s64(const struct nlattr *attr, s64 *val)
{
	if (nla_len(attr) >= sizeof(s64)) {
		*val = nla_get_s64(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_be16(const struct nlattr *attr, __be16 *val)
{
	if (nla_len(attr) >= sizeof(__be16)) {
		*val = nla_get_be16(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_be32(const struct nlattr *attr, __be32 *val)
{
	if (nla_len(attr) >= sizeof(__be32)) {
		*val = nla_get_be32(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_be64(const struct nlattr *attr, __be64 *val)
{
	if (nla_len(attr) >= sizeof(__be64)) {
		*val = nla_get_be64(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_le16(const struct nlattr *attr,  __le16  *val)
{
	if (nla_len(attr) >= sizeof(__le16)) {
		*val = nla_get_le16(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_le32(const struct nlattr *attr, __le32 *val)
{
	if (nla_len(attr) >= sizeof(__le32)) {
		*val = nla_get_le32(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_le64(const struct nlattr *attr, __le64 *val)
{
	if (nla_len(attr) >= sizeof(__le64)) {
		*val = nla_get_le64(attr);
		return 0;
	}
	return -EINVAL;
}

static inline int slsi_util_nla_get_data(const struct nlattr *attr, size_t size, void *val)
{
	if(nla_len(attr) >= size) {
		memcpy(val, nla_data(attr), size);
		return 0;
	}
	return -EINVAL;
}

#ifdef __cplusplus
}
#endif

#endif /* SLSI_UTILS_H__ */
