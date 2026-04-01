#ifndef STORAGEDATAAPI_H
#define STORAGEDATAAPI_H

#if defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

#include "storageData.h"

#define STORAGE_DATA_PATH "/opt/work/storage_data.txt"

extern VarStorage *storageData;

bool storageData_init(const char* base_path, bool auto_save);
void storageData_clear(void);
bool storageData_save(void);

#define storageData_setInt(key, defaultVal) \
    var_storage_set_int(storageData, key, defaultVal)

#define storageData_setBool(key, defaultVal) \
    var_storage_set_bool(storageData, key, defaultVal)

#define storageData_setString(key, defaultVal) \
    var_storage_set_string(storageData, key, defaultVal)

#define storageData_getInt(key, defaultVal) \
    var_storage_get_int(storageData, key, defaultVal)

#define storageData_getBool(key, defaultVal) \
    var_storage_get_bool(storageData, key, defaultVal)

#define storageData_getString(key, defaultVal) \
    var_storage_get_string(storageData, key, defaultVal)

#define  storageData_remove(key)   \
    var_storage_remove(storageData, key)




#if defined(__cplusplus)||defined(c_plusplus)
}
#endif 

#endif