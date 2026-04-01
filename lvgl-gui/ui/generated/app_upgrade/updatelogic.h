#ifndef UPDATELOGIC_H
#define UPDATELOGIC_H

// 进度回调函数类型
typedef void (*progress_callback_t)(int percentage, const char *message);

// 函数声明
void set_progress_callback(progress_callback_t callback);
void report_progress(int percentage, const char *message);
int verify_upgrade_package(const char *upgrade_dir);
int stop_application();
int backup_application();
int perform_upgrade(const char *upgrade_dir);

#endif