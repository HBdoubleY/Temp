#include "tf_card.h"
#include <sys/statvfs.h>
#include <unistd.h>

int tf_card_is_mounted(void)
{
    if (access(TF_CARD_DEVICE, F_OK) == 0)
        return 1;
    return 0;
}

int tf_card_get_storage_bytes(unsigned long long *used_bytes, unsigned long long *total_bytes)
{
    struct statvfs fs_info;

    if (access(TF_CARD_DEVICE, F_OK) != 0)
        return -1;
    if (statvfs(TF_CARD_MOUNT_PATH "/", &fs_info) == -1)
        return -1;

    unsigned long long total = (unsigned long long)fs_info.f_blocks * fs_info.f_frsize;
    unsigned long long free = (unsigned long long)fs_info.f_bfree * fs_info.f_frsize;
    unsigned long long used = total > free ? total - free : 0;

    if (used_bytes)
        *used_bytes = used;
    if (total_bytes)
        *total_bytes = total;
    return 0;
}

int tf_card_get_storage_mb(long long *used_mb, long long *total_mb)
{
    unsigned long long used_bytes, total_bytes;

    if (tf_card_get_storage_bytes(&used_bytes, &total_bytes) != 0)
        return -1;

    if (used_mb)
        *used_mb = (long long)(used_bytes / (1024 * 1024));
    if (total_mb)
        *total_mb = (long long)(total_bytes / (1024 * 1024));
    return 0;
}

