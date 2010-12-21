#include "chain.h"

bool dt_prop_iterate(char **dt, dt_prop_iterate_cb callback) {
    uint32_t n_properties = *((uint32_t *) *dt); *dt += 4;
    *dt += 4; // n_children
    while(n_properties--) {
        char *name = *dt; *dt += 32;
        uint32_t length = *((uint32_t *) *dt); *dt += 4;
        char *value = *dt;
        if(!callback(name, value, length)) {
            return false;
        }
        *dt += (length + 3) & ~3;
    }
    return true;
}

static bool dt_iterate_(char **dt, char *path, size_t path_len, dt_iterate_cb callback) {
    if(path_len != 0 && path[path_len - 1] != '/') path[path_len++] = '/';

    char *entry = *dt;
    uint32_t n_children = *((uint32_t *) (entry + 4));
    bool got_name = false;
   
    size_t *path_len_ = &path_len;
    bool *got_name_ = &got_name;
    dt_prop_iterate(dt, ^bool(char *name, char *value, size_t l) {
        if(!*got_name_ && !my_strcmp(name, "name")) {
            if(!my_memcmp(value, "device-tree", strlen("device-tree")+1)) {
                value = "IODeviceTree:/";
                l = strlen("IODeviceTree:/") + 1;
            }
            my_memcpy(path + *path_len_, value, l);
            *path_len_ += l - 1;
            *got_name_ = true;
        }
        return true;
    });
    if(!callback(path, path_len, entry)) {
        // break
        return false;
    }
    while(n_children--) {
        if(!dt_iterate_(dt, path, path_len, callback)) {
            return false;
        }
    }
    return true;
}

void dt_iterate(char **dt, dt_iterate_cb callback) {
    char buf[256];
    dt_iterate_(dt, buf, 0, callback);
}

char *dt_get_entry(char **dt, const char *desired) {
    char *result = NULL;

    char **result_ = &result;
    dt_iterate(dt, ^bool(const char *path, size_t path_len, char *entry) {
        if(!my_memcmp(desired, path, path_len) && desired[path_len] == '\0') {
            *result_ = entry;
            return false;
        } else {
            return true;
        }
    });
    return result;
}

bool dt_entry_set_prop(char *entry, const char *key, const char *replacement_key /* could be NULL */, const void *replacement_value, size_t replacement_value_len) {
    return !dt_prop_iterate(&entry, ^bool(char *name, char *value, size_t l) {
        if(!my_strcmp(name, key)) {
            if(replacement_key) {
                my_memcpy(name, replacement_key, 32);
            }
            my_memcpy(value, replacement_value, replacement_value_len);
            return false;
        }
        return true;
    });
}

void dt_super_iterate(char **dt) {
    dt_iterate(dt, ^bool(const char *path, size_t path_len, char *entry) {
        void *regentry = IORegistryEntry_fromPath(path, NULL, NULL, NULL, NULL);
        if(regentry) {
            dt_prop_iterate(&entry, ^bool(char *name, char *value, size_t l) {
                void *data = IORegistryEntry_getProperty(regentry, name);
                if(!data) {
                    IOLog("No data for %s.%s\n", path, name);
                    return true;
                }
                void *bytes; unsigned int length;
                if(OSMetaClassBase_safeMetaCast(data, OSData_metaClass)) {
                    bytes = OSData_getBytesNoCopy(data);
                    if(!bytes) {
                        if(l) {
                            IOLog("NULL 'bytes' for %s.%s\n", path, name);
                        }
                        return true;
                    }
                    length = OSData_getLength(data);
                } else if(OSMetaClassBase_safeMetaCast(data, OSNumber_metaClass)) {
                    uint64_t value = OSNumber_unsigned64BitValue(data);
                    length = l;
                    bytes = &value;
                } else {
                    IOLog("I don't know what kind of thing %s.%s is (vt=%p)\n", path, name, *((void **) data));
                    return true;
                }
                if(length != l) { 
                    IOLog("! size mismatch ! for %s.%s\n", path, name);
                    // this seems to come from IODARTMapper is not serializable
                    return true;
                }
                bool is_all_zeroes = true;
                for(size_t i = 0; i < l; i++) { if(value[i]) { is_all_zeroes = false; break; } }
                if(!is_all_zeroes && my_memcmp(value, bytes, l)) {
                    //IOLog("weird: %s.%s differs but not zero in DT\n", path, name);
                    /*serial_puthexbuf(value, l);
                    serial_putstring("\n");
                    serial_puthexbuf(bytes, l);
                    serial_putstring("\n");*/
                }
                my_memcpy(value, bytes, l);
                return true;
            });
        } else {
            IOLog("No regentry for %s\n", path);
        }
        return true;
    });
}
