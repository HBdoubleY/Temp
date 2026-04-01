#include "tf_card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define PARTITION_START_LBA   2048u
#define MBR_SIG_OFFSET        510
#define MBR_PART_ENTRY0       446
#define PART_TYPE_FAT32_LBA   0x0Cu
#define BYTES_PER_SECTOR     512
#define SECTORS_PER_CLUSTER  8
#define RESERVED_SECTORS     32
#define FAT_COUNT            2
#define ROOT_DIR_CLUSTER     2
#define FAT_ENTRY_SIZE       4
#define FAT_END_OF_CHAIN     0x0FFFFFF8
#define FAT_FREE_CLUSTER     0x00000000
#define BOOT_SIGNATURE_55AA  0xAA55

#pragma pack(push, 1)
typedef struct {
    uint8_t  jump[3];
    char     oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
    uint8_t  boot_code[420];
    uint16_t boot_signature_55aa;
} fat32_boot_sector_t;

typedef struct {
    uint32_t signature1;
    uint8_t  reserved1[480];
    uint32_t signature2;
    uint32_t free_clusters;
    uint32_t next_free_cluster;
    uint8_t  reserved2[12];
    uint32_t signature3;
} fsinfo_sector_t;
#pragma pack(pop)

static void put_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static uint64_t get_device_size(const char *device)
{
    if (!device) return 0;
    int fd = open(device, O_RDONLY);
    if (fd < 0) return 0;
    uint64_t size = 0;
    if (ioctl(fd, BLKGETSIZE64, &size) < 0) {
        struct stat st;
        if (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode)) {
            uint32_t sectors = 0;
            if (ioctl(fd, BLKGETSIZE, &sectors) == 0)
                size = (uint64_t)sectors * 512;
        }
    }
    close(fd);
    return size;
}

static int calculate_fat_size(uint32_t total_sectors)
{
    uint32_t fat_sectors = 0;
    uint32_t prev = 0;
    int iter = 0;
    do {
        uint32_t data_sectors = total_sectors - RESERVED_SECTORS - (FAT_COUNT * fat_sectors);
        uint32_t total_clusters = data_sectors / SECTORS_PER_CLUSTER;
        uint32_t new_fat = (total_clusters * FAT_ENTRY_SIZE + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR;
        prev = fat_sectors;
        fat_sectors = new_fat;
        if (++iter > 100 || fat_sectors > total_sectors / 2) break;
    } while (fat_sectors != prev);
    return (int)fat_sectors;
}

static uint32_t calculate_total_clusters(uint32_t total_sectors, uint32_t fat_sectors)
{
    uint32_t data_sectors = total_sectors - RESERVED_SECTORS - (FAT_COUNT * fat_sectors);
    return data_sectors / SECTORS_PER_CLUSTER;
}

static bool validate_device_path(const char *path)
{
    if (!path || !path[0]) return false;
    if (strstr(path, "../") || strstr(path, "..")) return false;
    struct stat st;
    if (stat(path, &st) < 0) return false;
    return S_ISBLK(st.st_mode) ? true : false;
}

static bool validate_volume_label(const char *label)
{
    if (!label) return true;
    size_t len = strlen(label);
    if (len > 11) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)label[i];
        if (!isalnum(c) && c != ' ' && c != '_' && c != '-' && c != '.' && c != '!' &&
            c != '#' && c != '$' && c != '%' && c != '&' && c != '\'' &&
            c != '(' && c != ')' && c != '@' && c != '^' && c != '`' &&
            c != '{' && c != '}' && c != '~')
            return false;
    }
    return true;
}

static bool write_dos_mbr(int fd, uint32_t start_lba, uint32_t sector_count)
{
    uint8_t mbr[BYTES_PER_SECTOR];

    memset(mbr, 0, sizeof(mbr));
    uint8_t *pe = mbr + MBR_PART_ENTRY0;
    pe[0] = 0x00;
    pe[1] = 0xFE;
    pe[2] = 0xFF;
    pe[3] = 0xFF;
    pe[4] = PART_TYPE_FAT32_LBA;
    pe[5] = 0xFE;
    pe[6] = 0xFF;
    pe[7] = 0xFF;
    put_u32_le(pe + 8, start_lba);
    put_u32_le(pe + 12, sector_count);
    mbr[MBR_SIG_OFFSET] = 0x55;
    mbr[MBR_SIG_OFFSET + 1] = 0xAA;

    if (lseek(fd, 0, SEEK_SET) < 0)
        return false;
    if (write(fd, mbr, sizeof(mbr)) != (ssize_t)sizeof(mbr))
        return false;
    return true;
}

static bool write_boot_sector(int fd, off_t base_off, uint32_t partition_sectors,
                              uint32_t hidden_sectors, uint32_t fat_sectors,
                              const char *volume_label)
{
    fat32_boot_sector_t boot;
    memset(&boot, 0, sizeof(boot));
    boot.jump[0] = 0xEB;
    boot.jump[1] = 0x58;
    boot.jump[2] = 0x90;
    memcpy(boot.oem_name, "MSWIN4.1", 8);
    boot.bytes_per_sector = BYTES_PER_SECTOR;
    boot.sectors_per_cluster = SECTORS_PER_CLUSTER;
    boot.reserved_sectors = RESERVED_SECTORS;
    boot.fat_count = FAT_COUNT;
    boot.media_type = 0xF8;
    boot.sectors_per_track = 63;
    boot.head_count = 255;
    boot.hidden_sectors = hidden_sectors;
    boot.total_sectors_32 = partition_sectors;
    boot.sectors_per_fat_32 = fat_sectors;
    boot.root_cluster = ROOT_DIR_CLUSTER;
    boot.fsinfo_sector = 1;
    boot.backup_boot_sector = 6;
    boot.drive_number = 0x80;
    boot.boot_signature = 0x29;
    srand((unsigned int)time(NULL));
    boot.volume_id = (uint32_t)rand() << 16 | (uint32_t)rand();
    if (volume_label && volume_label[0]) {
        strncpy(boot.volume_label, volume_label, 11);
        for (int i = (int)strlen(volume_label); i < 11; i++)
            boot.volume_label[i] = ' ';
    } else {
        memcpy(boot.volume_label, "NO NAME    ", 11);
    }
    memcpy(boot.fs_type, "FAT32   ", 8);
    boot.boot_signature_55aa = BOOT_SIGNATURE_55AA;

    if (lseek(fd, base_off, SEEK_SET) < 0) return false;
    if (write(fd, &boot, sizeof(boot)) != (ssize_t)sizeof(boot)) return false;
    return true;
}

static bool write_backup_boot_sector(int fd, off_t base_off)
{
    fat32_boot_sector_t boot;
    if (lseek(fd, base_off, SEEK_SET) < 0) return false;
    if (read(fd, &boot, sizeof(boot)) != (ssize_t)sizeof(boot)) return false;
    if (lseek(fd, base_off + 6 * BYTES_PER_SECTOR, SEEK_SET) < 0) return false;
    if (write(fd, &boot, sizeof(boot)) != (ssize_t)sizeof(boot)) return false;
    return true;
}

static bool write_fsinfo_sector(int fd, off_t base_off, uint32_t total_clusters)
{
    fsinfo_sector_t fsinfo;
    memset(&fsinfo, 0, sizeof(fsinfo));
    fsinfo.signature1 = 0x41615252;
    fsinfo.signature2 = 0x61417272;
    fsinfo.free_clusters = total_clusters - 1;
    fsinfo.next_free_cluster = ROOT_DIR_CLUSTER + 1;
    fsinfo.signature3 = 0xAA550000;
    if (lseek(fd, base_off + BYTES_PER_SECTOR, SEEK_SET) < 0) return false;
    if (write(fd, &fsinfo, sizeof(fsinfo)) != (ssize_t)sizeof(fsinfo)) return false;
    return true;
}

static bool initialize_fat_table(int fd, off_t base_off, uint32_t fat_sectors, uint32_t total_clusters)
{
    size_t fat_size = fat_sectors * BYTES_PER_SECTOR;
    uint8_t *fat_buffer = (uint8_t *)calloc(1, fat_size);
    if (!fat_buffer) return false;
    uint32_t *fat = (uint32_t *)fat_buffer;
    uint32_t fat_entries = (uint32_t)(fat_size / FAT_ENTRY_SIZE);
    fat[0] = 0xFFFFFFF8;
    fat[1] = 0xFFFFFFFF;
    fat[2] = FAT_END_OF_CHAIN;
    uint32_t limit = total_clusters < fat_entries - 1 ? total_clusters : fat_entries - 1;
    for (uint32_t i = 3; i < limit; i++)
        fat[i] = FAT_FREE_CLUSTER;

    off_t fat1_offset = base_off + RESERVED_SECTORS * BYTES_PER_SECTOR;
    if (lseek(fd, fat1_offset, SEEK_SET) < 0) { free(fat_buffer); return false; }
    if (write(fd, fat_buffer, fat_size) != (ssize_t)fat_size) { free(fat_buffer); return false; }
    if (lseek(fd, fat1_offset + (off_t)fat_size, SEEK_SET) < 0) { free(fat_buffer); return false; }
    if (write(fd, fat_buffer, fat_size) != (ssize_t)fat_size) { free(fat_buffer); return false; }
    free(fat_buffer);
    return true;
}

static bool initialize_root_directory(int fd, off_t base_off, uint32_t fat_sectors)
{
    uint8_t zero_sector[BYTES_PER_SECTOR];
    memset(zero_sector, 0, sizeof(zero_sector));
    off_t root_offset = base_off + (RESERVED_SECTORS + (FAT_COUNT * fat_sectors)) * (off_t)BYTES_PER_SECTOR;
    if (lseek(fd, root_offset, SEEK_SET) < 0) return false;
    for (int i = 0; i < SECTORS_PER_CLUSTER; i++) {
        if (write(fd, zero_sector, sizeof(zero_sector)) != (ssize_t)sizeof(zero_sector))
            return false;
    }
    return true;
}

static bool format_fat32_volume_at(int fd, off_t part_byte_off, uint32_t partition_sectors,
                                   uint32_t hidden_lba, uint32_t fat_sectors,
                                   uint32_t total_clusters, const char *volume_label)
{
    if (!write_boot_sector(fd, part_byte_off, partition_sectors, hidden_lba, fat_sectors, volume_label))
        return false;
    if (!write_backup_boot_sector(fd, part_byte_off)) return false;
    if (!write_fsinfo_sector(fd, part_byte_off, total_clusters)) return false;
    if (!initialize_fat_table(fd, part_byte_off, fat_sectors, total_clusters)) return false;
    if (!initialize_root_directory(fd, part_byte_off, fat_sectors)) return false;
    return true;
}

static bool do_quick_format_mbr(int fd, uint32_t disk_sectors, uint32_t partition_sectors,
                                uint32_t fat_sectors, uint32_t total_clusters,
                                const char *volume_label)
{
    off_t part_off = (off_t)PARTITION_START_LBA * BYTES_PER_SECTOR;
    (void)disk_sectors;

    if (!format_fat32_volume_at(fd, part_off, partition_sectors, PARTITION_START_LBA,
                               fat_sectors, total_clusters, volume_label))
        return false;
    if (!write_dos_mbr(fd, PARTITION_START_LBA, partition_sectors))
        return false;
    return true;
}

static bool do_full_format_mbr(int fd, uint32_t disk_sectors, uint32_t partition_sectors,
                               uint32_t fat_sectors, uint32_t total_clusters,
                               const char *volume_label)
{
    const size_t buf_size = 64 * 1024;
    uint8_t *zero_buf = (uint8_t *)calloc(1, buf_size);
    if (!zero_buf) return false;
    uint64_t total_bytes = (uint64_t)disk_sectors * BYTES_PER_SECTOR;
    uint64_t written = 0;
    if (lseek(fd, 0, SEEK_SET) < 0) { free(zero_buf); return false; }
    while (written < total_bytes) {
        size_t to_write = (size_t)((total_bytes - written) > buf_size ? buf_size : (total_bytes - written));
        ssize_t n = write(fd, zero_buf, to_write);
        if (n != (ssize_t)to_write) { free(zero_buf); return false; }
        written += (uint64_t)n;
    }
    free(zero_buf);

    off_t part_off = (off_t)PARTITION_START_LBA * BYTES_PER_SECTOR;
    if (!format_fat32_volume_at(fd, part_off, partition_sectors, PARTITION_START_LBA,
                                fat_sectors, total_clusters, volume_label)) {
        return false;
    }
    if (!write_dos_mbr(fd, PARTITION_START_LBA, partition_sectors))
        return false;
    return true;
}

static void reread_partition_table(const char *whole_disk)
{
    int rfd = open(whole_disk, O_RDWR | O_NONBLOCK);
    if (rfd < 0) {
        fprintf(stderr, "[TFCard] BLKRRPART open %s: %s\n", whole_disk, strerror(errno));
        return;
    }
    if (ioctl(rfd, BLKRRPART, NULL) != 0)
        fprintf(stderr, "[TFCard] BLKRRPART failed: %s (try blockdev --rereadpt)\n", strerror(errno));
    close(rfd);

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "blockdev --rereadpt %s 2>/dev/null", whole_disk);
    (void)system(cmd);
}

static void mount_tf_partition(void)
{
    char cmd[160];
    snprintf(cmd, sizeof(cmd), "mount %s %s", TF_CARD_PARTITION_DEV, TF_CARD_MOUNT_PATH);
    if (system(cmd) == 0)
        printf("[TFCard] Mounted %s on %s\n", TF_CARD_PARTITION_DEV, TF_CARD_MOUNT_PATH);
    else
        fprintf(stderr, "[TFCard] mount %s failed\n", TF_CARD_PARTITION_DEV);
}

int tf_card_format(const char *device, const char *volume_label, int quick)
{
    if (!validate_device_path(device)) {
        fprintf(stderr, "[TFCard] Invalid device path\n");
        return -1;
    }
    if (!validate_volume_label(volume_label)) {
        fprintf(stderr, "[TFCard] Invalid volume label\n");
        return -1;
    }

    char umount_cmd[128];
    snprintf(umount_cmd, sizeof(umount_cmd), "umount %s 2>/dev/null", TF_CARD_MOUNT_PATH);
    if (system(umount_cmd) == 0)
        printf("[TFCard] Unmounted %s\n", TF_CARD_MOUNT_PATH);

    int fd = open(device, O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "[TFCard] Open device %s failed: %s\n", device, strerror(errno));
        return -1;
    }

    uint64_t device_size = get_device_size(device);
    if (device_size == 0) {
        fprintf(stderr, "[TFCard] Get device size failed\n");
        close(fd);
        return -1;
    }

    uint32_t disk_sectors = (uint32_t)(device_size / BYTES_PER_SECTOR);
    if (disk_sectors <= PARTITION_START_LBA + 65536u) {
        fprintf(stderr, "[TFCard] Device too small for MBR layout + FAT32\n");
        close(fd);
        return -1;
    }

    uint32_t partition_sectors = disk_sectors - PARTITION_START_LBA;
    int fat_sectors_int = calculate_fat_size(partition_sectors);
    if (fat_sectors_int <= 0) {
        close(fd);
        return -1;
    }
    uint32_t fat_sectors = (uint32_t)fat_sectors_int;
    uint32_t total_clusters = calculate_total_clusters(partition_sectors, fat_sectors);
    if (total_clusters < 65525) {
        fprintf(stderr, "[TFCard] Not enough clusters for FAT32 in partition\n");
        close(fd);
        return -1;
    }

    bool ok = quick ? do_quick_format_mbr(fd, disk_sectors, partition_sectors, fat_sectors,
                                          total_clusters, volume_label)
                    : do_full_format_mbr(fd, disk_sectors, partition_sectors, fat_sectors,
                                        total_clusters, volume_label);

    if (!ok) {
        fprintf(stderr, "[TFCard] Format failed\n");
        close(fd);
        return -1;
    }

    if (fsync(fd) < 0)
        fprintf(stderr, "[TFCard] fsync warning: %s\n", strerror(errno));

    off_t verify_off = (off_t)PARTITION_START_LBA * BYTES_PER_SECTOR;
    fat32_boot_sector_t boot;
    if (lseek(fd, verify_off, SEEK_SET) < 0 || read(fd, &boot, sizeof(boot)) != (ssize_t)sizeof(boot)) {
        close(fd);
        return -1;
    }
    close(fd);

    if (boot.boot_signature_55aa != BOOT_SIGNATURE_55AA) {
        fprintf(stderr, "[TFCard] Boot signature verify failed\n");
        return -1;
    }

    reread_partition_table(device);
    usleep(200000);
    mount_tf_partition();

    printf("[TFCard] FAT32+MBR format complete, label=%.11s\n", boot.volume_label);
    return 0;
}
