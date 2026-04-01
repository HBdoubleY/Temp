
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <mtd/mtd-user.h>
#include "bootlogoUp.h"

// 函数声明
static int get_mtd_info(int fd, struct mtd_info_user *mtd_info);
static int create_backup(const char *mnt_dir, const char *backup_dir);
static int copy_file(const char *src, const char *dst);
static int replace_bootlogo(const char *mnt_dir, const char *new_logo_path);
static int verify_replacement(const char *mnt_dir, const char *new_logo_path);
static int mount_mtd9(const char *mnt_dir);
static int umount_mtd9(const char *mnt_dir);
static int is_mounted(const char *device, const char *mnt_dir);
static int format_vfat(const char *device);
static void print_usage(const char *prog_name);
static int check_file_exists(const char *path);

int bootlogoUp(char *path){
    int ret = 0;
    int mtd_fd = -1;
    char backup_path[256];
    time_t t;
    struct tm *tm_info;
    
    printf("========================================\n");
    printf("全志V853开发板开机Logo替换工具\n");
    printf("MTD9分区: %s (%s)\n", MTD9_DEVICE, MTD9_NAME);
    printf("========================================\n\n");
    

    const char *new_logo_path = path;
    
    // 检查新Logo文件是否存在
    if (access(new_logo_path, R_OK) != 0) {
        fprintf(stderr, "错误: 无法访问Logo文件 '%s': %s\n", 
                new_logo_path, strerror(errno));
        return EXIT_FAILURE;
    }
    
    // 检查文件大小
    struct stat st;
    if (stat(new_logo_path, &st) != 0) {
        fprintf(stderr, "错误: 无法获取文件信息: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    
    printf("新Logo文件: %s\n", new_logo_path);
    printf("文件大小: %ld bytes (%.2f KB)\n", 
           st.st_size, st.st_size / 1024.0);
    
    // 创建备份目录
    printf("\n1. 创建备份目录...\n");
    if (mkdir(BACKUP_DIR, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "警告: 无法创建备份目录 %s: %s\n", 
                BACKUP_DIR, strerror(errno));
    } else {
        printf("   ✓ 备份目录: %s\n", BACKUP_DIR);
    }
    
    // 检查MTD设备
    printf("\n2. 检查MTD9设备...\n");
    if (access(MTD9_DEVICE, R_OK|W_OK) != 0) {
        fprintf(stderr, "错误: 无法访问MTD设备 %s: %s\n", 
                MTD9_DEVICE, strerror(errno));
        return EXIT_FAILURE;
    }
    
    // 打开MTD设备
    mtd_fd = open(MTD9_DEVICE, O_RDWR);
    if (mtd_fd < 0) {
        fprintf(stderr, "错误: 无法打开MTD设备: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    printf("   ✓ MTD设备打开成功\n");
    
    // 获取MTD信息
    struct mtd_info_user mtd_info;
    if (get_mtd_info(mtd_fd, &mtd_info) != 0) {
        close(mtd_fd);
        return EXIT_FAILURE;
    }

    
    // 挂载MTD9分区
    printf("\n3. 挂载MTD9分区...\n");
    if (mount_mtd9(MOUNT_POINT) != 0) {
        // 挂载失败，可能需要格式化
        fprintf(stderr, "警告: 挂载失败，尝试格式化分区...\n");
        
        // 创建挂载点
        if (mkdir(MOUNT_POINT, 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "错误: 无法创建挂载点: %s\n", strerror(errno));
            close(mtd_fd);
            return EXIT_FAILURE;
        }
            
        // 重新挂载
        if (mount_mtd9(MOUNT_POINT) != 0) {
            fprintf(stderr, "错误: 挂载格式化后的分区失败\n");
            close(mtd_fd);
            return EXIT_FAILURE;
        }
    }
    printf("   ✓ 分区挂载成功: %s\n", MOUNT_POINT);
    
    // 检查分区内容
    printf("\n4. 检查分区内容...\n");
    if (access(MOUNT_POINT, R_OK) != 0) {
        fprintf(stderr, "错误: 无法访问挂载点\n");
        umount_mtd9(MOUNT_POINT);
        close(mtd_fd);
        return EXIT_FAILURE;
    }
    
    char bootlogo_path[256], magic_path[256];
    snprintf(bootlogo_path, sizeof(bootlogo_path), "%s/%s", MOUNT_POINT, BOOTLOGO_NAME);
    snprintf(magic_path, sizeof(magic_path), "%s/%s", MOUNT_POINT, MAGIC_NAME);
    
    int has_bootlogo = check_file_exists(bootlogo_path);
    int has_magic = check_file_exists(magic_path);
    
    if (has_bootlogo) {
        printf("   ✓ 找到bootlogo.jpg文件\n");
    } else {
        printf("   ⚠ 未找到bootlogo.jpg文件，将创建新文件\n");
    }
    
    if (has_magic) {
        printf("   ✓ 找到magic.bin文件\n");
    } else {
        printf("   ⚠ 未找到magic.bin文件\n");
    }
    
    // 备份原始文件
    printf("\n5. 备份原始文件...\n");
    if (create_backup(MOUNT_POINT, BACKUP_DIR) != 0) {
        fprintf(stderr, "警告: 备份文件失败\n");
    } else {
        printf("   ✓ 备份完成\n");
    }
    
    // 替换bootlogo
    printf("\n6. 替换bootlogo.jpg文件...\n");
    if (replace_bootlogo(MOUNT_POINT, new_logo_path) != 0) {
        fprintf(stderr, "错误: 替换bootlogo失败\n");
        umount_mtd9(MOUNT_POINT);
        close(mtd_fd);
        return EXIT_FAILURE;
    }
    printf("   ✓ bootlogo.jpg已替换\n");
    
    // 验证替换
    printf("\n7. 验证文件替换...\n");
    if (verify_replacement(MOUNT_POINT, new_logo_path) != 0) {
        fprintf(stderr, "警告: 文件验证有差异\n");
    } else {
        printf("   ✓ 文件验证通过\n");
    }
    
    // 列出目录内容
    printf("\n8. 分区内容列表:\n");
    DIR *dir = opendir(MOUNT_POINT);
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char filepath[256];
                snprintf(filepath, sizeof(filepath), "%s/%s", MOUNT_POINT, entry->d_name);
                
                struct stat file_stat;
                if (stat(filepath, &file_stat) == 0) {
                    printf("   - %s (%ld bytes)\n", entry->d_name, file_stat.st_size);
                } else {
                    printf("   - %s\n", entry->d_name);
                }
            }
        }
        closedir(dir);
    }
    
    // 卸载分区
    printf("\n9. 卸载分区...\n");
    if (umount_mtd9(MOUNT_POINT) != 0) {
        fprintf(stderr, "警告: 卸载分区失败\n");
    } else {
        printf("   ✓ 分区已卸载\n");
    }
    
    // 清理挂载点
    rmdir(MOUNT_POINT);
    
    // 关闭文件描述符
    close(mtd_fd);
    
    printf("\n========================================\n");
    printf("开机Logo替换完成！\n");
    printf("请重启系统查看效果: sync && reboot\n");
    printf("========================================\n");
    
    return EXIT_SUCCESS;
}

static int get_mtd_info(int fd, struct mtd_info_user *mtd_info) {
    if (fd < 0 || !mtd_info) {
        fprintf(stderr, "错误: 无效的参数\n");
        return -1;
    }
    
    // 获取MTD设备信息
    if (ioctl(fd, MEMGETINFO, mtd_info) < 0) {
        fprintf(stderr, "错误: 无法获取MTD设备信息: %s\n", strerror(errno));
        return -1;
    }
    
    return 0;
}

/**
 * 挂载MTD9分区
 */
static int mount_mtd9(const char *mnt_dir) {
    // 创建挂载点
    if (mkdir(mnt_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "创建挂载点失败: %s\n", strerror(errno));
        return -1;
    }
    
    // 检查是否已挂载
    if (is_mounted(MTD9_BLOCK_DEV, mnt_dir)) {
        printf("   ⚠ 分区已挂载，尝试卸载后重新挂载\n");
        if (umount(mnt_dir) != 0 && errno != EINVAL) {
            fprintf(stderr, "卸载失败: %s\n", strerror(errno));
        }
    }
    
    // 挂载分区
    printf("   挂载 %s 到 %s ...\n", MTD9_BLOCK_DEV, mnt_dir);
    if (mount(MTD9_BLOCK_DEV, mnt_dir, "vfat", MS_NOATIME | MS_SYNCHRONOUS, "") != 0) {
        if (errno == ENOENT) {
            // 可能是vfat文件系统未识别，尝试挂载为msdos
            if (mount(MTD9_BLOCK_DEV, mnt_dir, "msdos", MS_NOATIME | MS_SYNCHRONOUS, "") != 0) {
                fprintf(stderr, "挂载失败: %s\n", strerror(errno));
                return -1;
 }
        } else {
            fprintf(stderr, "挂载失败: %s\n", strerror(errno));
            return -1;
        }
    }
    
    return 0;
}

/**
 * 卸载MTD9分区
 */
static int umount_mtd9(const char *mnt_dir) {
    // 同步文件系统
    sync();
    
    // 卸载
    if (umount(mnt_dir) != 0) {
        // 如果卸载失败，尝试延迟卸载
        fprintf(stderr, "卸载失败: %s，尝试延迟卸载\n", strerror(errno));
        sync();
        sleep(1);
        
        if (umount(mnt_dir) != 0) {
            // 强制卸载
            fprintf(stderr, "强制卸载: %s\n", strerror(errno));
            if (umount2(mnt_dir, MNT_FORCE) != 0) {
                fprintf(stderr, "强制卸载失败: %s\n", strerror(errno));
                return -1;
            }
        }
    }
    
    return 0;
}

/**
 * 检查设备是否已挂载
 */
static int is_mounted(const char *device, const char *mnt_dir) {
    FILE *mounts = fopen("/proc/mounts", "r");
    if (!mounts) {
        return 0;
    }
    
    char line[256];
    int mounted = 0;
    
    while (fgets(line, sizeof(line), mounts)) {
        if (strstr(line, device) != NULL || strstr(line, mnt_dir) != NULL) {
            mounted = 1;
            break;
        }
    }
    
    fclose(mounts);
    return mounted;
}

/**
 * 格式化分区为VFAT文件系统
 */
static int format_vfat(const char *device) {
    printf("   格式化 %s 为VFAT文件系统...\n", device);
    
    // 使用mkfs.vfat命令格式化
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkfs.vfat %s 2>&1", device);
    
    printf("   执行命令: %s\n", cmd);
    int ret = system(cmd);
    
    if (WIFEXITED(ret)) {
        int exit_status = WEXITSTATUS(ret);
        if (exit_status != 0) {
            fprintf(stderr, "格式化失败，退出码: %d\n", exit_status);
            return -1;
        }
    } else {
        fprintf(stderr, "格式化命令异常终止\n");
        return -1;
    }
    
    printf("   ✓ 格式化完成\n");
    return 0;
}

/**
 * 创建备份
 */
static int create_backup(const char *mnt_dir, const char *backup_dir) {
    char src_path[256];
    char dst_path[256];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    
    // 创建时间戳
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);
    
    // 备份bootlogo.jpg
    snprintf(src_path, sizeof(src_path), "%s/%s", mnt_dir, BOOTLOGO_NAME);
    snprintf(dst_path, sizeof(dst_path), "%s/bootlogo_%s.jpg", backup_dir, timestamp);
    
    if (access(src_path, R_OK) == 0) {
        printf("   备份 %s -> %s\n", BOOTLOGO_NAME, dst_path);
        if (copy_file(src_path, dst_path) != 0) {
            fprintf(stderr, "备份 %s 失败\n", BOOTLOGO_NAME);
        }
    }
    
    // 备份magic.bin
    snprintf(src_path, sizeof(src_path), "%s/%s", mnt_dir, MAGIC_NAME);
    snprintf(dst_path, sizeof(dst_path), "%s/magic_%s.bin", backup_dir, timestamp);
    
    if (access(src_path, R_OK) == 0) {
        printf("   备份 %s -> %s\n", MAGIC_NAME, dst_path);
        if (copy_file(src_path, dst_path) != 0) {
            fprintf(stderr, "备份 %s 失败\n", MAGIC_NAME);
        }
    }
    
    return 0;
}

/**
 * 复制文件
 */
static int copy_file(const char *src, const char *dst) {
    FILE *src_fp = fopen(src, "rb");
    if (!src_fp) {
        fprintf(stderr, "无法打开源文件: %s\n", strerror(errno));
        return -1;
    }
    
    FILE *dst_fp = fopen(dst, "wb");
    if (!dst_fp) {
        fprintf(stderr, "无法创建目标文件: %s\n", strerror(errno));
        fclose(src_fp);
        return -1;
    }
    
    char buffer[4096];
    size_t bytes_read, bytes_written;
    size_t total = 0;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_fp)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, dst_fp);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "写入目标文件失败\n");
            fclose(src_fp);
            fclose(dst_fp);
            return -1;
        }
        total += bytes_written;
    }
    
    fclose(src_fp);
    fclose(dst_fp);
    
    printf("   ✓ 复制完成: %ld bytes\n", total);
    return 0;
}

/**
 * 替换bootlogo文件
 */
static int replace_bootlogo(const char *mnt_dir, const char *new_logo_path) {
    char dst_path[256];
    snprintf(dst_path, sizeof(dst_path), "%s/%s", mnt_dir, BOOTLOGO_NAME);
    
    // 删除原有文件
    if (access(dst_path, F_OK) == 0) {
        if (unlink(dst_path) != 0) {
            fprintf(stderr, "删除原有文件失败: %s\n", strerror(errno));
        }
    }
    
    // 复制新文件
    printf("   复制 %s -> %s\n", new_logo_path, dst_path);
    if (copy_file(new_logo_path, dst_path) != 0) {
        fprintf(stderr, "复制Logo文件失败\n");
        return -1;
    }
    
    return 0;
}

/**
 * 验证替换结果
 */
static int verify_replacement(const char *mnt_dir, const char *new_logo_path) {
    char bootlogo_path[256];
    snprintf(bootlogo_path, sizeof(bootlogo_path), "%s/%s", mnt_dir, BOOTLOGO_NAME);
    
    FILE *src_fp = fopen(new_logo_path, "rb");
    FILE *dst_fp = fopen(bootlogo_path, "rb");
    
    if (!src_fp || !dst_fp) {
        if (src_fp) fclose(src_fp);
        if (dst_fp) fclose(dst_fp);
        return -1;
    }
    
    char src_buf[4096];
    char dst_buf[4096];
    size_t src_read, dst_read;
    int match = 1;
    size_t total = 0;
    
    do {
        src_read = fread(src_buf, 1, sizeof(src_buf), src_fp);
        dst_read = fread(dst_buf, 1, sizeof(dst_buf), dst_fp);
        
        if (src_read != dst_read) {
            match = 0;
            break;
        }
        
        if (memcmp(src_buf, dst_buf, src_read) != 0) {
            match = 0;
            break;
        }
        
        total += src_read;
    } while (src_read > 0);
    
    fclose(src_fp);
    fclose(dst_fp);
    
    if (match) {
        printf("   文件验证: 匹配 (%ld bytes)\n", total);
        return 0;
    } else {
        fprintf(stderr, "   文件验证: 不匹配\n");
        return -1;
    }
}

/**
 * 检查文件是否存在
 */
static int check_file_exists(const char *path) {
    return access(path, R_OK) == 0;
}

/**
 * 打印使用说明
 */
static void print_usage(const char *prog_name) {
    printf("使用说明:\n");
    printf("  %s <logo_image_file>\n\n", prog_name);
    printf("参数:\n");
    printf("  logo_image_file  要写入的Logo图片文件(JPG格式)\n\n");
    printf("示例:\n");
    printf("  %s /path/to/new_logo.jpg\n", prog_name);
    printf("  %s /tmp/logo_800x480.jpg\n\n", prog_name);
    printf("注意:\n");
    printf("  1. 需要root权限运行\n");
    printf("  2. 操作前会自动备份原始文件到 %s/\n", BACKUP_DIR);
    printf("  3. Logo文件建议使用JPG格式，尺寸与屏幕分辨率匹配\n");
    printf("  4. 操作有风险，请确保有恢复手段\n\n");
    printf("MTD9分区信息:\n");
    printf("  设备文件: %s\n", MTD9_DEVICE);
    printf("  块设备:   %s\n", MTD9_BLOCK_DEV);
    printf("  分区名:   %s\n", MTD9_NAME);
    printf("  文件系统: vfat\n");
    printf("  挂载点:   %s\n", MOUNT_POINT);
}