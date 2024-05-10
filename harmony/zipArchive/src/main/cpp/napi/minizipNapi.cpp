//
// Created on 2024/5/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include <cstdio>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "minizipNapi.h"
#include "minizip.h"

using namespace std;

enum {
    SHOW_LIST = 0,
    DECOMPRESS,
    ERASE,
    COMPRESS,
};

typedef enum {
    OBJECT = 0,
    NUMBER,
    STRING,
    ARRAY,

} ValueType;

napi_ref minizipNapi::sConstructor_ = nullptr;

static int minizip_getObjectPropetry(napi_env env, napi_value object, string key, int keyType, void *retValue) {
    napi_value property = nullptr;
    napi_value result = nullptr;
    bool flag = false;
    int ret = -1;

    if (napi_create_string_utf8(env, key.c_str(), strlen(key.c_str()), &property) != napi_ok) {
        return ret;
    }

    if (napi_has_property(env, object, property, &flag) != napi_ok) {
        return ret;
    }

    if (napi_get_property(env, object, property, &result) != napi_ok) {
        return ret;
    }

    if (keyType == NUMBER) {
        int64_t value = 0;
        if (napi_get_value_int64(env, result, &value) != napi_ok) {
            return ret;
        }
        *(int *)retValue = value;
        ret = 0;
    } else if (keyType == STRING) {
        size_t s = 0;
        char buf[256] = {0};
        if (napi_get_value_string_utf8(env, result, buf, sizeof(buf), &s) != napi_ok) {
            return ret;
        }
        strncpy((char *)retValue, buf, strlen(buf));
        ret = 0;
    }

    return ret;
}

int minizip_parser_params(napi_env env, napi_value object) {
    int32_t err = MZ_OK;
    int cmd = COMPRESS;
    char path[256] = {0};
    char password[64] = {0};
    char destination[256] = {0};
    char file_extract[256] = {0};
    char optBuf[256] = {0};

    minizip_opt opt;
    memset(&opt, 0x00, sizeof(minizip_opt));

    uint32_t argc = 0;
    char *argv[128];

    if (minizip_getObjectPropetry(env, object, "path", STRING, (void *)path) < 0) {
        return -1;
    }

    minizip_getObjectPropetry(env, object, "password", STRING, (void *)password);
    if (minizip_getObjectPropetry(env, object, "operate", STRING, (void *)optBuf) < 0) {
        return -1;
    }
    if (strcmp(optBuf, "decompress") == 0) {
        cmd = DECOMPRESS;
    } else if (strcmp(optBuf, "erase") == 0) {
        cmd = ERASE;
    } else if (strcmp(optBuf, "list") == 0) {
        cmd = SHOW_LIST;
    }

    minizip_getObjectPropetry(env, object, "file_extract", STRING, (void *)file_extract);
    minizip_getObjectPropetry(env, object, "directory", STRING, (void *)destination);

    // for options
    napi_value option = nullptr;
    if (napi_get_named_property(env, object, "option", &option) != napi_ok) {
        return -1;
    }
    minizip_getObjectPropetry(env, option, "include_path", NUMBER, (void *)&opt.include_path);
    minizip_getObjectPropetry(env, option, "compress_level", NUMBER, (void *)&opt.compress_level);
    minizip_getObjectPropetry(env, option, "compress_method", NUMBER, (void *)&opt.compress_method);
    minizip_getObjectPropetry(env, option, "overwrite", NUMBER, (void *)&opt.overwrite);
    minizip_getObjectPropetry(env, option, "append", NUMBER, (void *)&opt.append);
    minizip_getObjectPropetry(env, option, "disk_size", NUMBER, (void *)&opt.disk_size);
    minizip_getObjectPropetry(env, option, "follow_links", NUMBER, (void *)&opt.follow_links);
    minizip_getObjectPropetry(env, option, "store_links", NUMBER, (void *)&opt.store_links);
    minizip_getObjectPropetry(env, option, "zip_cd", NUMBER, (void *)&opt.zip_cd);
    minizip_getObjectPropetry(env, option, "encoding", NUMBER, (void *)&opt.encoding);
    minizip_getObjectPropetry(env, option, "verbose", NUMBER, (void *)&opt.verbose);
    minizip_getObjectPropetry(env, option, "aes", NUMBER, (void *)&opt.aes);
    char cert_path[256] = {0};
    if (minizip_getObjectPropetry(env, option, "cert_path", STRING, (void *)cert_path) == 0) {
        opt.cert_path = (const char *)cert_path;
    }
    char cert_pwd[256] = {0};
    if (minizip_getObjectPropetry(env, option, "cert_pwd", NUMBER, (void *)cert_pwd) == 0) {
        opt.cert_pwd = (const char *)cert_pwd;
    }

    // file arrays
    napi_value files = nullptr;
    if (napi_get_named_property(env, object, "files", &files) == napi_ok) {
        bool flag = false;
        if (napi_is_array(env, files, &flag) == napi_ok && flag == true) {
            napi_get_array_length(env, files, &argc);
            int pos = 0;
            // HILOGI("argc = %{public}d", argc);
            for (int i = 0; i < argc; i++) {
                napi_value file = nullptr;
                char *buf = (char *)malloc(512);
                size_t s = 0;
                if (napi_get_element(env, files, i, &file) != napi_ok) {
                    pos++;
                    free(buf);
                    continue;
                }
                if (napi_get_value_string_utf8(env, file, buf, 512, &s) != napi_ok) {
                    free(buf);
                    pos++;
                } else {
                    argv[i - pos] = buf;
                }
            }

            argc -= pos;
            argv[argc] = nullptr;
        }
    }

    if (cmd == SHOW_LIST) {
        err = minizip_list((const char *)path);
    } else if (cmd == DECOMPRESS) {
        err = minizip_extract((const char *)path, strlen(file_extract) > 0 ? (const char *)file_extract : nullptr,
                              strlen(destination) > 0 ? (const char *)destination : nullptr,
                              strlen(password) > 0 ? (const char *)password : nullptr, &opt);
    } else if (cmd == COMPRESS) {
        err = minizip_add((const char *)path, strlen(password) > 0 ? (const char *)password : nullptr, &opt, argc,
                          (const char **)argv);
    }

    for (int i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            free(argv[i]);
        }
    }

    return err;
}

napi_value minizipNapi::Compress(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value;
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, &value, nullptr, nullptr) != napi_ok) {
        return result;
    }

    if (minizip_parser_params(env, value) < 0) {
        return result;
    }

    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return result;
}

napi_value minizipNapi::Decompress(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value;
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, &value, nullptr, nullptr) != napi_ok) {
        return result;
    }

    if (minizip_parser_params(env, value) < 0) {
        return result;
    }

    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return result;
}

napi_value minizipNapi::CompressWithPsd(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value;
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, &value, nullptr, nullptr) != napi_ok) {
        return result;
    }

    if (minizip_parser_params(env, value) < 0) {
        return result;
    }

    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return result;
}

napi_value minizipNapi::DecompressWithPsd(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value;
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, &value, nullptr, nullptr) != napi_ok) {
        return result;
    }

    if (minizip_parser_params(env, value) < 0) {
        return result;
    }

    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return result;
}

napi_value minizipNapi::Create(napi_env env, napi_callback_info info) {
    napi_status status;
    napi_value constructor = nullptr, result = nullptr;

    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (status != napi_ok) {
        return result;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        return result;
    }

    auto mini = new minizipNapi();
    if (napi_wrap(env, result, reinterpret_cast<void *>(mini), Destructor, nullptr, &(mini->mRef)) == napi_ok) {
        return result;
    }

    return nullptr;
}

napi_value minizipNapi::Constructor(napi_env env, napi_callback_info info) {
    napi_value undefineVar = nullptr, thisVar = nullptr;
    napi_get_undefined(env, &undefineVar);

    if (napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr) == napi_ok && thisVar != nullptr) {
        minizipNapi *reference = new minizipNapi();
        if (napi_wrap(env, thisVar, reinterpret_cast<void *>(reference), minizipNapi::Destructor, nullptr,
                      &(reference->mRef)) == napi_ok) {
            return thisVar;
        }

        return thisVar;
    }

    return undefineVar;
}

void minizipNapi::Destructor(napi_env env, void *nativeObject, void *finalize) {
    minizipNapi *mini = reinterpret_cast<minizipNapi *>(nativeObject);
    mini->~minizipNapi();
}

napi_value minizipNapi::Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"compress", nullptr, minizipNapi::Compress, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"decompress", nullptr, minizipNapi::Decompress, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"compressWithPsd", nullptr, minizipNapi::CompressWithPsd, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"decompressWithPsd", nullptr, minizipNapi::DecompressWithPsd, nullptr, nullptr, nullptr, napi_default,
         nullptr}};

    napi_value constructor = nullptr;

    if (napi_define_class(env, NAPI_CLASS_NAME, NAPI_AUTO_LENGTH, minizipNapi::Constructor, nullptr,
                          sizeof(desc) / sizeof(desc[0]), desc, &constructor) != napi_ok) {
        return nullptr;
    }

    if (napi_create_reference(env, constructor, 1, &sConstructor_) != napi_ok) {
        return nullptr;
    }

    if (napi_set_named_property(env, exports, NAPI_CLASS_NAME, constructor) != napi_ok) {
        return nullptr;
    }

    return exports;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {

    napi_property_descriptor desc[] = {
        {"create", nullptr, minizipNapi::Create, nullptr, nullptr, nullptr, napi_default, nullptr}};

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

    return minizipNapi::Init(env, exports);
}
EXTERN_C_END

static napi_module nativi_minizip = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "nativi_minizip",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterHelloModule(void) { napi_module_register(&nativi_minizip); }
