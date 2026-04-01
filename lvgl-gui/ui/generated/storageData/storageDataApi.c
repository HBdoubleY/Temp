#include "storageDataApi.h"
#include <stdio.h>

VarStorage *storageData = NULL;


bool storageData_init(const char* base_path, bool auto_save) {
    if (storageData) {
        var_storage_destroy(storageData);
    }
    
    storageData = var_storage_create(base_path, auto_save);
    return storageData != NULL;
}

void storageData_clear(void) {
    if (storageData) {
        var_storage_clear(storageData);
        storageData = NULL;
    }
}

bool storageData_save(void){
    return var_storage_save(storageData);
}
