#include <stdio.h>
#include "updatelogic.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <openssl/md5.h>

// 进度回调函数指针
progress_callback_t progress_callback = NULL;

// 设置进度回调
void set_progress_callback(progress_callback_t callback) {
    progress_callback = callback;
}

// 报告进度
void report_progress(int percentage, const char *message) {
    if (progress_callback != NULL) {
        progress_callback(percentage, message);
    }
}

// 计算文件MD5
int calculate_md5(const char *filename, unsigned char *result) {
    FILE *file = fopen(filename, "rb");
    if (!file) return -1;
    
    MD5_CTX context;
    MD5_Init(&context);
    
    unsigned char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        MD5_Update(&context, buffer, bytes_read);
    }
    
    MD5_Final(result, &context);
    fclose(file);
    return 0;
}

// 验证升级包
int verify_upgrade_package(const char *upgrade_dir) {
    char version_file[512];
    char package_file[512];
    FILE *fp;
    char line[256];
    char expected_md5[33];
    
    snprintf(version_file, sizeof(version_file), "%s/version.info", upgrade_dir);
    snprintf(package_file, sizeof(package_file), "%s/update.tar.gz", upgrade_dir);
    
    report_progress(5, "开始验证升级包...");
    
    // 读取版本文件
    fp = fopen(version_file, "r");
    if (!fp) {
        report_progress(0, "错误：无法打开版本文件");
        return -1;
    }
    
    // 简单的版本文件解析 (格式: version=1.0.0 md5=xxxxxxxx)
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "md5=")) {
            sscanf(line, "md5=%32s", expected_md5);
            break;
        }
    }
    fclose(fp);
    
    if (strlen(expected_md5) != 32) {
        report_progress(0, "错误：无效的MD5校验和");
        return -1;
    }
    
    // 计算实际MD5
    unsigned char actual_md5[MD5_DIGEST_LENGTH];
    if (calculate_md5(package_file, actual_md5) != 0) {
        report_progress(0, "错误：无法计算MD5");
        return -1;
    }
    
    // 转换为十六进制字符串
    char actual_md5_str[33];
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&actual_md5_str[i*2], "%02x", actual_md5[i]);
    }
    actual_md5_str[32] = '\0';
    
    // 比较MD5
    if (strcmp(actual_md5_str, expected_md5) != 0) {
        report_progress(0, "错误：MD5校验失败");
        return -1;
    }
    
    report_progress(20, "升级包验证成功");
    return 0;
}

// 停止应用程序
int stop_application() {
    report_progress(25, "停止当前应用程序...");
    
    // 方法1: 通过systemd停止
    if (system("systemctl stop my_application 2>/dev/null") == 0) {
        sleep(2);
        return 0;
    }
    
    // 方法2: 通过进程名停止
    if (system("killall my_application 2>/dev/null") == 0) {
        sleep(2);
        return 0;
    }
    
    // 方法3: 直接杀死进程
    system("pkill my_application");
    sleep(1);
    
    return 0;
}

// 备份当前应用
int backup_application() {
    report_progress(30, "备份当前应用...");
    
    // 创建备份目录
    mkdir("/tmp/app_backup", 0755);
    
    // 备份应用文件
    if (system("cp -r /opt/my_app /tmp/app_backup/ 2>/dev/null") != 0) {
        report_progress(30, "警告：备份应用文件失败");
    }
    
    // 备份配置文件
    if (system("cp -r /etc/my_app /tmp/app_backup/etc/ 2>/dev/null") != 0) {
        report_progress(30, "警告：备份配置文件失败");
    }
    
    return 0;
}

// 执行升级
int perform_upgrade(const char *upgrade_dir) {
    char command[512];
    char package_file[512];
    
    snprintf(package_file, sizeof(package_file), "%s/update.tar.gz", upgrade_dir);
    
    report_progress(0, "开始升级流程");
    
    // 1. 验证升级包
    if (verify_upgrade_package(upgrade_dir) != 0) {
        return -1;
    }
    
    // 2. 停止应用
    stop_application();
    
    // 3. 备份
    backup_application();
    
    // 4. 解压升级包
    report_progress(50, "解压升级文件...");
    snprintf(command, sizeof(command), "tar -xzf %s -C /", package_file);
    if (system(command) != 0) {
        report_progress(0, "错误：解压失败");
        return -1;
    }
    
    // 5. 设置权限
    report_progress(80, "设置文件权限...");
    system("chmod +x /opt/my_app/bin/* 2>/dev/null");
    system("chmod 644 /opt/my_app/lib/* 2>/dev/null");
    
    // 6. 恢复配置文件 (如果有备份)
    report_progress(90, "恢复配置文件...");
    system("cp -r /tmp/app_backup/etc/my_app /etc/ 2>/dev/null");
    
    // 7. 清理备份
    system("rm -rf /tmp/app_backup");
    
    // 8. 删除升级包标记，避免重复升级
    snprintf(command, sizeof(command), "rm -rf %s", upgrade_dir);
    system(command);
    
    report_progress(100, "升级完成");
    
    return 0;
}