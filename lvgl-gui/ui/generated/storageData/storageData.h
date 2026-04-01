/**
 * @file storageData.h
 * @brief 变量存储管理库头文件
 * 
 * 这是一个轻量级的键值对存储库，支持将变量保存到JSON格式文件中。
 * 支持的数据类型：int, bool, string
 * 文件格式：JSON
 * 线程安全：否
 * 文件大小限制：100KB
 */

#ifndef STORAGE_DATA_H
#define STORAGE_DATA_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    TYPE_INT,      ///< 整数类型，对应C语言的int
    TYPE_BOOL,     ///< 布尔类型，对应C语言的bool
    TYPE_STRING    ///< 字符串类型，对应C语言的char*
} VarType;

typedef struct VarStorage VarStorage;

// ============ 存储管理函数 ============

VarStorage* var_storage_create(const char* filename, bool auto_save);


void var_storage_destroy(VarStorage* storage);


bool var_storage_save(VarStorage* storage);


bool var_storage_set_int(VarStorage* storage, const char* name, int value);


bool var_storage_set_bool(VarStorage* storage, const char* name, bool value);


bool var_storage_set_string(VarStorage* storage, const char* name, const char* value);


int var_storage_get_int(VarStorage* storage, const char* name, int default_value);


bool var_storage_get_bool(VarStorage* storage, const char* name, bool default_value);


char* var_storage_get_string(VarStorage* storage, const char* name, const char* default_value);


const char* var_storage_get_string_ref(VarStorage* storage, const char* name);


bool var_storage_remove(VarStorage* storage, const char* name);


bool var_storage_exists(VarStorage* storage, const char* name);


VarType var_storage_get_type(VarStorage* storage, const char* name);

int var_storage_count(VarStorage* storage);


void var_storage_print_all(VarStorage* storage);

void var_storage_clear(VarStorage* storage);


void var_storage_set_auto_save(VarStorage* storage, bool enabled);


char* var_storage_export_json(VarStorage* storage);


const char* var_storage_get_last_error(void);


void var_storage_clear_error(void);

#ifdef __cplusplus
}
#endif

#endif // STORAGE_DATA_H