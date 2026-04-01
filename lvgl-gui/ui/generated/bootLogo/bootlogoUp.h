#ifndef BOOTLOGOUP_H__
#define BOOTLOGOUP_H__

#define MTD9_DEVICE      "/dev/mtd9"
#define MTD9_BLOCK_DEV   "/dev/mtdblock9"
#define MTD9_NAME        "boot-resource"
#define MOUNT_POINT      "/tmp/boot-resource"
#define BOOTLOGO_NAME    "bootlogo.jpg"
#define MAGIC_NAME       "magic.bin"
#define BACKUP_DIR       "/tmp/logo_backup"
#define EXTERN_LOGO_PATH    "/opt/work/app/logo/bootlogo.jpg"
#define INTERN_LOGO_PATH    "/mnt/extsd/logo"


int bootlogoUp(char *path);
#endif