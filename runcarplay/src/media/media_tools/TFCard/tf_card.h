#ifndef TF_CARD_H
#define TF_CARD_H

#ifdef __cplusplus
extern "C" {
#endif

/** 默认 TF 卡挂载路径 */
#define TF_CARD_MOUNT_PATH     "/mnt/extsd"

#define TF_CARD_DEVICE         "/dev/mmcblk0"

#define TF_CARD_PARTITION_DEV  "/dev/mmcblk0p1"

int tf_card_is_mounted(void);

int tf_card_get_storage_bytes(unsigned long long *used_bytes, unsigned long long *total_bytes);

int tf_card_get_storage_mb(long long *used_mb, long long *total_mb);

int tf_card_format(const char *device, const char *volume_label, int quick);

#ifdef __cplusplus
}
#endif

#endif /* TF_CARD_H */
