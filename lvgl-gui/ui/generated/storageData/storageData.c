#include "storageData.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

// ============ 常量定义 ============
#define MAX_NAME_LEN 255
#define MAX_STRING_LEN 65535
#define MAX_FILE_SIZE (1024 * 100)  // 100KB
#define JSON_INDENT 2  // JSON缩进空格数

// ============ 变量结构 ============
typedef struct Variable {
    char name[MAX_NAME_LEN + 1];
    VarType type;
    union {
        int int_value;
        bool bool_value;
        char* string_value;
    } value;
    struct Variable* next;
} Variable;

// ============ 存储管理器结构 ============
typedef struct VarStorage{
    char filename[1024];
    Variable* variables;
    int count;
    bool auto_save;  // 是否自动保存
} VarStorage;

// ============ 辅助函数 ============

// 安全的字符串复制
static void safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) return;
    
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

// 验证变量名是否有效
static bool is_valid_name(const char* name) {
    if (!name || name[0] == '\0') return false;
    
    // 检查长度
    size_t len = strlen(name);
    if (len > MAX_NAME_LEN) return false;
    
    // 检查是否以字母或下划线开头
    if (!((name[0] >= 'a' && name[0] <= 'z') ||
          (name[0] >= 'A' && name[0] <= 'Z') ||
          name[0] == '_')) {
        return false;
    }
    
    // 检查剩余字符
    for (size_t i = 1; i < len; i++) {
        if (!((name[i] >= 'a' && name[i] <= 'z') ||
              (name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= '0' && name[i] <= '9') ||
              name[i] == '_')) {
            return false;
        }
    }
    
    return true;
}

// 查找变量
static Variable* find_variable(VarStorage* storage, const char* name) {
    if (!storage || !name) return NULL;
    
    Variable* current = storage->variables;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 创建新变量
static Variable* create_variable(const char* name, VarType type) {
    Variable* var = (Variable*)calloc(1, sizeof(Variable));
    if (!var) return NULL;
    
    safe_strcpy(var->name, name, sizeof(var->name));
    var->type = type;
    var->next = NULL;
    
    return var;
}

// 释放变量内存
static void free_variable(Variable* var) {
    if (!var) return;
    
    if (var->type == TYPE_STRING && var->value.string_value) {
        free(var->value.string_value);
    }
    free(var);
}

// 计算文件大小
static long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// 确保目录存在
static bool ensure_directory_exists(const char* filename) {
    char path[1024];
    safe_strcpy(path, filename, sizeof(path));
    
    // 查找最后一个斜杠
    char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        return true;  // 没有目录部分
    }
    
    *last_slash = '\0';
    
    // 如果目录已经存在
    struct stat st = {0};
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    
    // 递归创建目录
    char* p = path;
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(path, 0755) != 0 && errno != EEXIST) {
                return false;
            }
            *p = '/';
        }
        p++;
    }
    
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
        return false;
    }
    
    return true;
}

// JSON转义字符串
static char* json_escape_string(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    // 计算转义后的长度
    size_t escaped_len = 0;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '\"': case '\\': case '\b': 
            case '\f': case '\n': case '\r': 
            case '\t':
                escaped_len += 2;  // 转义序列，如 \n
                break;
            default:
                if (str[i] < 0x20) {
                    escaped_len += 6;  // Unicode转义，如 \u001f
                } else {
                    escaped_len++;
                }
        }
    }
    
    char* escaped = (char*)malloc(escaped_len + 1);
    if (!escaped) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '\"':
                escaped[j++] = '\\';
                escaped[j++] = '\"';
                break;
            case '\\':
                escaped[j++] = '\\';
                escaped[j++] = '\\';
                break;
            case '\b':
                escaped[j++] = '\\';
                escaped[j++] = 'b';
                break;
            case '\f':
                escaped[j++] = '\\';
                escaped[j++] = 'f';
                break;
            case '\n':
                escaped[j++] = '\\';
                escaped[j++] = 'n';
                break;
            case '\r':
                escaped[j++] = '\\';
                escaped[j++] = 'r';
                break;
            case '\t':
                escaped[j++] = '\\';
                escaped[j++] = 't';
                break;
            default:
                if (str[i] < 0x20) {
                    // 控制字符，转换为Unicode转义序列
                    snprintf(escaped + j, 7, "\\u%04x", (unsigned char)str[i]);
                    j += 6;
                } else {
                    escaped[j++] = str[i];
                }
        }
    }
    escaped[j] = '\0';
    
    return escaped;
}

// JSON反转义字符串
static char* json_unescape_string(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    char* unescaped = (char*)malloc(len + 1);
    if (!unescaped) return NULL;
    
    size_t i = 0, j = 0;
    while (i < len) {
        if (str[i] == '\\' && i + 1 < len) {
            i++;  // 跳过反斜杠
            switch (str[i]) {
                case '\"':
                    unescaped[j++] = '\"';
                    break;
                case '\\':
                    unescaped[j++] = '\\';
                    break;
                case '/':
                    unescaped[j++] = '/';
                    break;
                case 'b':
                    unescaped[j++] = '\b';
                    break;
                case 'f':
                    unescaped[j++] = '\f';
                    break;
                case 'n':
                    unescaped[j++] = '\n';
                    break;
                case 'r':
                    unescaped[j++] = '\r';
                    break;
                case 't':
                    unescaped[j++] = '\t';
                    break;
                case 'u':
                    // Unicode转义序列，简化处理
                    if (i + 4 < len) {
                        char hex[5];
                        hex[0] = str[i+1];
                        hex[1] = str[i+2];
                        hex[2] = str[i+3];
                        hex[3] = str[i+4];
                        hex[4] = '\0';
                        int code = (int)strtol(hex, NULL, 16);
                        unescaped[j++] = (char)code;
                        i += 4;
                    } else {
                        unescaped[j++] = 'u';
                    }
                    break;
                default:
                    unescaped[j++] = str[i];
            }
            i++;
        } else {
            unescaped[j++] = str[i++];
        }
    }
    unescaped[j] = '\0';
    
    return unescaped;
}

// 解析JSON值
static bool parse_json_value(const char* json_str, VarType* out_type, void* out_value) {
    if (!json_str || !out_type || !out_value) return false;
    
    // 去除前后空格
    while (isspace(*json_str)) json_str++;
    
    if (strcmp(json_str, "true") == 0) {
        *out_type = TYPE_BOOL;
        *(bool*)out_value = true;
        return true;
    } else if (strcmp(json_str, "false") == 0) {
        *out_type = TYPE_BOOL;
        *(bool*)out_value = false;
        return true;
    } else if (strcmp(json_str, "null") == 0) {
        *out_type = TYPE_STRING;
        *(char**)out_value = NULL;
        return true;
    } else if (json_str[0] == '"') {
        // 字符串
        *out_type = TYPE_STRING;
        char* end_quote = strchr(json_str + 1, '"');
        if (!end_quote) return false;
        
        size_t len = end_quote - (json_str + 1);
        char* temp_str = (char*)malloc(len + 1);
        if (!temp_str) return false;
        
        strncpy(temp_str, json_str + 1, len);
        temp_str[len] = '\0';
        
        char* unescaped = json_unescape_string(temp_str);
        free(temp_str);
        
        *(char**)out_value = unescaped;
        return (unescaped != NULL);
    } else {
        // 尝试解析为数字
        char* endptr;
        long int_val = strtol(json_str, &endptr, 10);
        if (*endptr == '\0' || isspace(*endptr)) {
            *out_type = TYPE_INT;
            *(int*)out_value = (int)int_val;
            return true;
        }
    }
    
    return false;
}

// ============ 文本文件操作函数 ============

// 保存变量到JSON文件
static bool save_to_json_file(VarStorage* storage) {
    if (!storage) return false;
    
    FILE* file = fopen(storage->filename, "w");
    if (!file) {
        fprintf(stderr, "Cannot open file for writing: %s\n", storage->filename);
        return false;
    }
    
    fprintf(file, "{\n");
    
    Variable* current = storage->variables;
    bool first = true;
    int saved_count = 0;
    
    while (current) {
        if (!first) {
            fprintf(file, ",\n");
        }
        first = false;
        
        // 缩进
        fprintf(file, "%*s\"%s\": ", JSON_INDENT, "", current->name);
        
        switch (current->type) {
            case TYPE_INT:
                fprintf(file, "%d", current->value.int_value);
                break;
                
            case TYPE_BOOL:
                fprintf(file, "%s", current->value.bool_value ? "true" : "false");
                break;
                
            case TYPE_STRING:
                if (current->value.string_value) {
                    char* escaped = json_escape_string(current->value.string_value);
                    if (escaped) {
                        fprintf(file, "\"%s\"", escaped);
                        free(escaped);
                    } else {
                        fprintf(file, "\"\"");
                    }
                } else {
                    fprintf(file, "null");
                }
                break;
        }
        saved_count++;
        current = current->next;
    }
    
    fprintf(file, "\n}\n");
    fclose(file);
    
    printf("Saved %d variables to JSON file: %s\n", saved_count, storage->filename);
    return true;
}

// 从JSON文件加载变量
static bool load_from_json_file(VarStorage* storage) {
    if (!storage) return false;
    
    long file_size = get_file_size(storage->filename);
    if (file_size <= 0 || file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Invalid file size: %s (%ld bytes)\n", storage->filename, file_size);
        return false;
    }
    
    FILE* file = fopen(storage->filename, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file for reading: %s\n", storage->filename);
        return false;
    }
    
    // 读取整个文件
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return false;
    }
    
    size_t read_size = fread(buffer, 1, file_size, file);
    buffer[read_size] = '\0';
    fclose(file);
    
    // 解析JSON
    char* pos = buffer;
    int loaded_count = 0;
    
    // 跳过开头的空格
    while (isspace(*pos)) pos++;
    
    // 检查是否以 { 开头
    if (*pos != '{') {
        fprintf(stderr, "Invalid JSON format: expected '{'\n");
        free(buffer);
        return false;
    }
    pos++;
    
    while (*pos) {
        // 跳过空格
        while (isspace(*pos)) pos++;
        
        // 检查是否结束
        if (*pos == '}') {
            break;
        }
        
        // 解析键（变量名）
        if (*pos != '"') {
            fprintf(stderr, "Invalid JSON: expected '\"'\n");
            break;
        }
        pos++;
        
        char* key_start = pos;
        while (*pos && *pos != '"') pos++;
        if (*pos != '"') {
            fprintf(stderr, "Invalid JSON: unterminated key\n");
            break;
        }
        
        size_t key_len = pos - key_start;
        char key[MAX_NAME_LEN + 1];
        safe_strcpy(key, key_start, key_len + 1);
        pos++;  // 跳过结束的引号
        
        // 跳过空格
        while (isspace(*pos)) pos++;
        
        // 检查冒号
        if (*pos != ':') {
            fprintf(stderr, "Invalid JSON: expected ':'\n");
            break;
        }
        pos++;
        
        // 跳过空格
        while (isspace(*pos)) pos++;
        
        // 解析值
        char* value_start = pos;
        
        // 移动到值的末尾
        if (*pos == '"') {
            // 字符串
            pos++;
            while (*pos && *pos != '"') {
                if (*pos == '\\' && *(pos + 1)) {
                    pos += 2;  // 跳过转义序列
                } else {
                    pos++;
                }
            }
            if (*pos == '"') {
                pos++;
            }
        } else if (*pos == 't') {
            // true
            if (strncmp(pos, "true", 4) == 0) {
                pos += 4;
            }
        } else if (*pos == 'f') {
            // false
            if (strncmp(pos, "false", 5) == 0) {
                pos += 5;
            }
        } else if (*pos == 'n') {
            // null
            if (strncmp(pos, "null", 4) == 0) {
                pos += 4;
            }
        } else {
            // 数字或其他
            while (*pos && *pos != ',' && *pos != '}' && !isspace(*pos)) {
                pos++;
            }
        }
        
        // 提取值字符串
        size_t value_len = pos - value_start;
        char value_str[256];
        safe_strcpy(value_str, value_start, value_len + 1);
        
        // 解析值
        VarType type;
        union {
            int int_val;
            bool bool_val;
            char* str_val;
        } value;
        
        if (parse_json_value(value_str, &type, &value)) {
            // 创建变量
            Variable* var = create_variable(key, type);
            if (var) {
                switch (type) {
                    case TYPE_INT:
                        var->value.int_value = value.int_val;
                        break;
                    case TYPE_BOOL:
                        var->value.bool_value = value.bool_val;
                        break;
                    case TYPE_STRING:
                        var->value.string_value = value.str_val;
                        break;
                }
                
                // 添加到链表
                var->next = storage->variables;
                storage->variables = var;
                storage->count++;
                loaded_count++;
            }
        }
        
        // 跳过逗号
        while (isspace(*pos)) pos++;
        if (*pos == ',') {
            pos++;
        }
    }
    
    free(buffer);
    printf("Loaded %d variables from JSON file: %s\n", loaded_count, storage->filename);
    return true;
}

// ============ 公共API函数 ============

// 创建存储
VarStorage* var_storage_create(const char* filename, bool auto_save) {
    if (!filename) {
        fprintf(stderr, "Filename is required\n");
        return NULL;
    }
    
    // 检查文件大小
    long file_size = get_file_size(filename);
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large: %s (%ld bytes)\n", filename, file_size);
        return NULL;
    }
    
    // 确保目录存在
    if (!ensure_directory_exists(filename)) {
        fprintf(stderr, "Cannot create directory for: %s\n", filename);
        return NULL;
    }
    
    // 分配内存
    VarStorage* storage = (VarStorage*)calloc(1, sizeof(VarStorage));
    if (!storage) {
        perror("Failed to allocate VarStorage");
        return NULL;
    }
    
    safe_strcpy(storage->filename, filename, sizeof(storage->filename));
    storage->variables = NULL;
    storage->count = 0;
    storage->auto_save = auto_save;
    
    // 尝试加载现有数据
    if (access(filename, F_OK) == 0) {
        if (!load_from_json_file(storage)) {
            fprintf(stderr, "Failed to load from JSON file, creating new storage\n");
        }
    } else {
        printf("Creating new storage file: %s\n", filename);
    }
    
    return storage;
}

// 销毁存储
void var_storage_destroy(VarStorage* storage) {
    if (!storage) return;
    
    // 自动保存
    if (storage->auto_save) {
        var_storage_save(storage);
    }
    
    // 释放所有变量
    Variable* current = storage->variables;
    while (current) {
        Variable* next = current->next;
        free_variable(current);
        current = next;
    }
    
    free(storage);
}

// 保存所有变量到文件
bool var_storage_save(VarStorage* storage) {
    if (!storage) return false;
    
    return save_to_json_file(storage);
}

// 设置int变量
bool var_storage_set_int(VarStorage* storage, const char* name, int value) {
    if (!storage || !name) {
        fprintf(stderr, "Invalid arguments\n");
        return false;
    }
    
    if (!is_valid_name(name)) {
        fprintf(stderr, "Invalid variable name: %s\n", name);
        return false;
    }
    
    // 查找是否已存在
    Variable* var = find_variable(storage, name);
    if (!var) {
        // 创建新变量
        var = create_variable(name, TYPE_INT);
        if (!var) {
            fprintf(stderr, "Failed to create variable: %s\n", name);
            return false;
        }
        
        // 添加到链表开头
        var->next = storage->variables;
        storage->variables = var;
        storage->count++;
    } else {
        // 检查类型
        if (var->type != TYPE_INT) {
            fprintf(stderr, "Variable '%s' is not an int\n", name);
            return false;
        }
    }
    
    var->value.int_value = value;
    
    // 自动保存
    if (storage->auto_save) {
        return var_storage_save(storage);
    }
    return true;
}

// 设置bool变量
bool var_storage_set_bool(VarStorage* storage, const char* name, bool value) {
    if (!storage || !name) {
        fprintf(stderr, "Invalid arguments\n");
        return false;
    }
    
    if (!is_valid_name(name)) {
        fprintf(stderr, "Invalid variable name: %s\n", name);
        return false;
    }
    
    // 查找是否已存在
    Variable* var = find_variable(storage, name);
    if (!var) {
        // 创建新变量
        var = create_variable(name, TYPE_BOOL);
        if (!var) {
            fprintf(stderr, "Failed to create variable: %s\n", name);
            return false;
        }
        
        // 添加到链表开头
        var->next = storage->variables;
        storage->variables = var;
        storage->count++;
    } else {
        // 检查类型
        if (var->type != TYPE_BOOL) {
            fprintf(stderr, "Variable '%s' is not a bool\n", name);
            return false;
        }
    }
    
    var->value.bool_value = value;
    
    // 自动保存
    if (storage->auto_save) {
        return var_storage_save(storage);
    }
    return true;
}

// 设置string变量
bool var_storage_set_string(VarStorage* storage, const char* name, const char* value) {
    if (!storage || !name) {
        fprintf(stderr, "Invalid arguments\n");
        return false;
    }
    
    if (!is_valid_name(name)) {
        fprintf(stderr, "Invalid variable name: %s\n", name);
        return false;
    }
    
    // 检查字符串长度
    if (value && strlen(value) > MAX_STRING_LEN) {
        fprintf(stderr, "String too long: %s\n", name);
        return false;
    }
    
    // 查找是否已存在
    Variable* var = find_variable(storage, name);
    if (!var) {
        // 创建新变量
        var = create_variable(name, TYPE_STRING);
        if (!var) {
            fprintf(stderr, "Failed to create variable: %s\n", name);
            return false;
        }
        
        // 分配并复制字符串
        if (value) {
            var->value.string_value = strdup(value);
            if (!var->value.string_value) {
                free_variable(var);
                fprintf(stderr, "Failed to allocate string for: %s\n", name);
                return false;
            }
        } else {
            var->value.string_value = NULL;
        }
        
        // 添加到链表开头
        var->next = storage->variables;
        storage->variables = var;
        storage->count++;
    } else {
        // 检查类型
        if (var->type != TYPE_STRING) {
            fprintf(stderr, "Variable '%s' is not a string\n", name);
            return false;
        }
        
        // 释放旧的字符串
        if (var->value.string_value) {
            free(var->value.string_value);
        }
        
        // 分配并复制新的字符串
        if (value) {
            var->value.string_value = strdup(value);
            if (!var->value.string_value) {
                fprintf(stderr, "Failed to allocate string for: %s\n", name);
                return false;
            }
        } else {
            var->value.string_value = NULL;
        }
    }
    
    // 自动保存
    if (storage->auto_save) {
        return var_storage_save(storage);
    }
    return true;
}

// 获取int变量 - 返回获取的值，参数为获取失败时的默认值
int var_storage_get_int(VarStorage* storage, const char* name, int default_value) {
    if (!storage || !name) {
        return default_value;
    }
    
    Variable* var = find_variable(storage, name);
    if (!var || var->type != TYPE_INT) {
        return default_value;
    }
    
    return var->value.int_value;
}

// 获取bool变量 - 返回获取的值，参数为获取失败时的默认值
bool var_storage_get_bool(VarStorage* storage, const char* name, bool default_value) {
    if (!storage || !name) {
        return default_value;
    }
    
    Variable* var = find_variable(storage, name);
    if (!var || var->type != TYPE_BOOL) {
        return default_value;
    }
    
    return var->value.bool_value;
}

// 获取string变量 - 返回获取的值（需要调用者释放），参数为获取失败时的默认值
char* var_storage_get_string(VarStorage* storage, const char* name, const char* default_value) {
    if (!storage || !name) {
        return default_value ? strdup(default_value) : NULL;
    }
    
    Variable* var = find_variable(storage, name);
    if (!var || var->type != TYPE_STRING) {
        return default_value ? strdup(default_value) : NULL;
    }
    
    if (!var->value.string_value) {
        return default_value ? strdup(default_value) : NULL;
    }
    
    return strdup(var->value.string_value);
}

// 获取string变量引用（不要修改） - 如果不存在返回NULL
const char* var_storage_get_string_ref(VarStorage* storage, const char* name) {
    if (!storage || !name) {
        return NULL;
    }
    
    Variable* var = find_variable(storage, name);
    if (!var || var->type != TYPE_STRING) {
        return NULL;
    }
    
    return var->value.string_value;
}

// 删除变量
bool var_storage_remove(VarStorage* storage, const char* name) {
    if (!storage || !name) {
        return false;
    }
    
    Variable* current = storage->variables;
    Variable* prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                storage->variables = current->next;
            }
            
            free_variable(current);
            storage->count--;
            
            // 自动保存
            if (storage->auto_save) {
                return var_storage_save(storage);
            }
            return true;
        }
        
        prev = current;
        current = current->next;
    }
    
    return false;
}

// 检查变量是否存在
bool var_storage_exists(VarStorage* storage, const char* name) {
    return find_variable(storage, name) != NULL;
}

// 获取变量类型
VarType var_storage_get_type(VarStorage* storage, const char* name) {
    Variable* var = find_variable(storage, name);
    if (!var) {
        return TYPE_INT;  // 默认返回int类型
    }
    return var->type;
}

// 获取变量数量
int var_storage_count(VarStorage* storage) {
    return storage ? storage->count : 0;
}

// 打印所有变量
void var_storage_print_all(VarStorage* storage) {
    if (!storage) {
        printf("Storage is NULL\n");
        return;
    }
    
    printf("\n=== Variable Storage (%s) ===\n", storage->filename);
    printf("Count: %d\n", storage->count);
    printf("Auto-save: %s\n\n", storage->auto_save ? "enabled" : "disabled");
    
    if (storage->count == 0) {
        printf("No variables stored.\n");
        return;
    }
    
    printf("%-20s %-10s %-20s\n", "Name", "Type", "Value");
    printf("%-20s %-10s %-20s\n", "----", "----", "-----");
    
    Variable* current = storage->variables;
    while (current) {
        printf("%-20s ", current->name);
        
        switch (current->type) {
            case TYPE_INT:
                printf("%-10s %-20d\n", "int", current->value.int_value);
                break;
            case TYPE_BOOL:
                printf("%-10s %-20s\n", "bool", 
                       current->value.bool_value ? "true" : "false");
                break;
            case TYPE_STRING:
                if (current->value.string_value) {
                    // 截断长字符串
                    char display[21];
                    safe_strcpy(display, current->value.string_value, sizeof(display));
                    if (strlen(current->value.string_value) > 20) {
                        strcpy(display + 17, "...");
                    }
                    printf("%-10s \"%s\"\n", "string", display);
                } else {
                    printf("%-10s (null)\n", "string");
                }
                break;
        }
        
        current = current->next;
    }
    printf("\n");
}

// 清除所有变量
void var_storage_clear(VarStorage* storage) {
    if (!storage) return;
    
    Variable* current = storage->variables;
    while (current) {
        Variable* next = current->next;
        free_variable(current);
        current = next;
    }
    
    storage->variables = NULL;
    storage->count = 0;
    
    // 清除文件
    FILE* file = fopen(storage->filename, "w");
    if (file) {
        fprintf(file, "{\n}\n");
        fclose(file);
    }
}

// 设置自动保存
void var_storage_set_auto_save(VarStorage* storage, bool enabled) {
    if (storage) {
        storage->auto_save = enabled;
    }
}

// 导出为JSON字符串
char* var_storage_export_json(VarStorage* storage) {
    if (!storage) return NULL;
    
    // 计算所需缓冲区大小
    size_t buffer_size = 1024;  // 初始大小
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) return NULL;
    
    int pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "{\n");
    
    Variable* current = storage->variables;
    bool first = true;
    
    while (current && pos < buffer_size - 100) {  // 保留100字节余量
        if (!first) {
            pos += snprintf(buffer + pos, buffer_size - pos, ",\n");
        }
        first = false;
        
        // 缩进
        pos += snprintf(buffer + pos, buffer_size - pos, "%*s\"%s\": ", JSON_INDENT, "", current->name);
        
        switch (current->type) {
            case TYPE_INT:
                pos += snprintf(buffer + pos, buffer_size - pos, "%d", current->value.int_value);
                break;
                
            case TYPE_BOOL:
                pos += snprintf(buffer + pos, buffer_size - pos, "%s", 
                               current->value.bool_value ? "true" : "false");
                break;
                
            case TYPE_STRING:
                if (current->value.string_value) {
                    char* escaped = json_escape_string(current->value.string_value);
                    if (escaped) {
                        pos += snprintf(buffer + pos, buffer_size - pos, "\"%s\"", escaped);
                        free(escaped);
                    } else {
                        pos += snprintf(buffer + pos, buffer_size - pos, "\"\"");
                    }
                } else {
                    pos += snprintf(buffer + pos, buffer_size - pos, "null");
                }
                break;
        }
        
        current = current->next;
    }
    
    if (pos < buffer_size - 2) {
        pos += snprintf(buffer + pos, buffer_size - pos, "\n}");
    } else {
        strcpy(buffer + buffer_size - 4, "...");
    }
    
    return buffer;
}