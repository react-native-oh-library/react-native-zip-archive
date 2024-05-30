//
// Created on 2024/5/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include <bits/alltypes.h>
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
#include <hilog/log.h>

#include "uv.h"

using namespace std;

const unsigned int LOG_PRINT_DOMAIN = 0XFF00;
static napi_threadsafe_function g_callbackFunc = nullptr;
static int g_stat = 2;
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
napi_env minizipNapi::mEnv = nullptr;
napi_ref minizipNapi::mRef = nullptr;
napi_ref minizipNapi::mRefFuncProgress = nullptr;


struct minizipWorkData {
    napi_async_work asyncwork = nullptr;
    napi_deferred deferred = nullptr;

    
    int parCount;
    napi_value arrpar;
    int result = 0;
    int (*funcZip)(napi_env env, minizip_opt *opt, minizip_opt_other_s *optother);
    minizip_opt opt;
    minizip_opt_other_s optother;
};


void ProgressCallback(float progress, int uncompressSize) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0522", "ProgressCallback, log out,file=%{public}s,line=%{public}d", __FILE__, __LINE__);
    // 创建一个int，作为ArkTS的入参
    napi_value parValue[2];
    napi_create_double(minizipNapi::mEnv, progress, &parValue[0]);
    napi_create_int64(minizipNapi::mEnv, uncompressSize, &parValue[1]);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "minizipNapi::mEnv1111 = %{public}d", minizipNapi::mEnv);

    // 调用 JavaScript 函数
    napi_value jsFunction;
    auto status = napi_get_reference_value(minizipNapi::mEnv, minizipNapi::mRef, &jsFunction);
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, "test-0522", "status=%{public}d", status);
    
    // 调用传入的callback，并将其结果返回
    status = napi_call_function(minizipNapi::mEnv, nullptr, jsFunction, 2, parValue, nullptr);
}

void ProgressCallbackzip(float progress) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528",
                 "ProgressCallback, log out,file=%{public}s,line=%{public}d", __FILE__, __LINE__);
    // 创建一个int，作为ArkTS的入参
    napi_value parValue = nullptr;
    napi_create_double(minizipNapi::mEnv, progress, &parValue);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528",
                 "ProgressCallback, log out,file=%{public}s,line=%{public}d", __FILE__, __LINE__);

    // 调用 JavaScript 函数
    napi_value jsFunction;
    auto status = napi_get_reference_value(minizipNapi::mEnv, minizipNapi::mRef, &jsFunction);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528",
                 "ProgressCallback, log out,file=%{public}s,line=%{public}d", __FILE__, __LINE__);


    // 调用传入的callback，并将其结果返回
    status = napi_call_function(minizipNapi::mEnv, nullptr, jsFunction, 1, &parValue, nullptr);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528",
                 "ProgressCallback, log out,file=%{public}s,line=%{public}d", __FILE__, __LINE__);
}


//后台工作，执行实际的异步
void minizipNapi::DoDecompressWork(napi_env env, napi_value js_callback, void *context, void *data) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "DoDecompressWork");
    minizipWorkData* async_data = static_cast<minizipWorkData*>(context);
    async_data->result = async_data->funcZip(env, &async_data->opt, &async_data->optother);
    
    napi_value result, undefined;
    // 返回结果给js
    napi_create_int64(env, async_data->result, &result);
    napi_resolve_deferred(env, async_data->deferred, result);
    napi_delete_async_work(env, async_data->asyncwork);
    delete async_data;
    async_data = nullptr;
}

//异步完成后回调
static void AfterDecompressWork(napi_env env, napi_status status, void* data) {
    minizipWorkData* async_data = static_cast<minizipWorkData*>(data);
    napi_value result, undefined;
    
    //返回结果给js
    napi_create_int64(env, async_data->result, &result);
    napi_resolve_deferred(env, async_data->deferred, result);
    napi_delete_async_work(env, async_data->asyncwork);
    delete async_data;
    async_data = nullptr;
}

static int minizip_getObjectPropetry(napi_env env, napi_value object, string key, int keyType, void *retValue) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                 "start minizip_getObjectPropetry");
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
        char buf[2048] = {0};
        if (napi_get_value_string_utf8(env, result, buf, sizeof(buf), &s) != napi_ok) {
            return ret;
        }
        strncpy((char *)retValue, buf, strlen(buf));
        ret = 0;
    }

    return ret;
}

int minizip_parser_params_get(napi_env env, napi_value object, minizip_opt *opt, minizip_opt_other_s *optother) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "start minizip_parser_params_get");
    optother->argc = 0;
    int32_t err = MZ_OK;

    int status = minizip_getObjectPropetry(env, object, "path", STRING, (void *)optother->path);
    minizip_getObjectPropetry(env, object, "password", STRING, (void *)optother->password);
    if (minizip_getObjectPropetry(env, object, "operate", STRING, (void *)optother->optBuf) < 0) {
        return -1;
    }
    minizip_getObjectPropetry(env, object, "file_extract", STRING, (void *)optother->file_extract);
    minizip_getObjectPropetry(env, object, "directory", STRING, (void *)optother->destination);
    // for options
    napi_value option = nullptr;
    if (napi_get_named_property(env, object, "option", &option) != napi_ok) {
        return -1;
    }
    minizip_getObjectPropetry(env, option, "include_path", NUMBER, (void *)&opt->include_path);
        
    minizip_getObjectPropetry(env, option, "compress_level", NUMBER, (void *)&opt->compress_level);
    minizip_getObjectPropetry(env, option, "compress_method", NUMBER, (void *)&opt->compress_method);
    minizip_getObjectPropetry(env, option, "overwrite", NUMBER, (void *)&opt->overwrite);
    minizip_getObjectPropetry(env, option, "append", NUMBER, (void *)&opt->append);
    minizip_getObjectPropetry(env, option, "disk_size", NUMBER, (void *)opt->disk_size);
    minizip_getObjectPropetry(env, option, "follow_links", NUMBER, (void *)&opt->follow_links);
    minizip_getObjectPropetry(env, option, "store_links", NUMBER, (void *)&opt->store_links);
    minizip_getObjectPropetry(env, option, "zip_cd", NUMBER, (void *)&opt->zip_cd);
    minizip_getObjectPropetry(env, option, "encoding", NUMBER, (void *)&opt->encoding);
    minizip_getObjectPropetry(env, option, "verbose", NUMBER, (void *)&opt->verbose);
    minizip_getObjectPropetry(env, option, "aes", NUMBER, (void *)&opt->aes);
    char cert_path[256] = {0};
    if (minizip_getObjectPropetry(env, option, "cert_path", STRING, (void *)cert_path) == 0) {
        opt->cert_path = (const char *)cert_path;
    }
    char cert_pwd[256] = {0};
    if (minizip_getObjectPropetry(env, option, "cert_pwd", NUMBER, (void *)cert_pwd) == 0) {
        opt->cert_pwd = (const char *)cert_pwd;
    }
    // file arrays
    napi_value files = nullptr;
    if (napi_get_named_property(env, object, "files", &files) == napi_ok) {
        bool flag = false;
        if (napi_is_array(env, files, &flag) == napi_ok && flag == true) {
            napi_get_array_length(env, files, &optother->argc);
            int pos = 0;
            // HILOGI("optother->argc = %{public}d", optother->argc);
            for (int i = 0; i < optother->argc; i++) {
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
                    optother->argv[i - pos] = buf;
                }
            }
            optother->argc -= pos;
            optother->argv[optother->argc] = nullptr;
        }
    }
    
    return err;
}

int minizip_parser_params2(napi_env env, minizip_opt *opt, minizip_opt_other_s *optother) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "minizip_parser_params2");
    int32_t err = MZ_OK;
    int cmd = COMPRESS;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528", "cmd = %{public}d", cmd);
    
    if (strcmp(optother->optBuf, "decompress") == 0) {
        cmd = DECOMPRESS;
    }
    else if (strcmp(optother->optBuf, "erase") == 0) {
        cmd = ERASE;
    }
    else if (strcmp(optother->optBuf, "list") == 0) {
        cmd = SHOW_LIST;
    }

    if (cmd == SHOW_LIST) {
        err = minizip_list((const char *)optother->path);
    } else if (cmd == DECOMPRESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "path = %{public}s file_extract = %{public}s file_extract = %{public}s file_extract = %{public}s", 
            optother->path, optother->file_extract, optother->destination, optother->password);
        err = minizip_extract((const char *)optother->path,
                              strlen(optother->file_extract) > 0 ? (const char *)optother->file_extract : nullptr,
                              strlen(optother->destination) > 0 ? (const char *)optother->destination : nullptr,
                              strlen(optother->password) > 0 ? (const char *)optother->password : nullptr, opt, ProgressCallback);
    } else if (cmd == COMPRESS) {
        err = minizip_add((const char *)optother->path,
                          strlen(optother->password) > 0 ? (const char *)optother->password : nullptr, opt,
                          optother->argc, (const char **)optother->argv, ProgressCallbackzip);
    }

    for (int i = 0; i < optother->argc; i++) {
        if (optother->argv[i] != nullptr) {
            free(optother->argv[i]);
        }
    }

    return err;
}

int minizip_parser_params(napi_env env, napi_value object) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "minizip_parser_params");

        minizip_opt opt;
        memset(&opt, 0x00, sizeof(minizip_opt));

        minizip_opt_other_s optother;
        memset(&optother, 0x00, sizeof(minizip_opt_other_s));

        minizip_parser_params_get(env, object, &opt, &optother);

        return minizip_parser_params2(env, &opt, &optother);
}

napi_value minizipNapi::Compress(napi_env env, napi_callback_info info) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0527", "Compress111");
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        napi_value value[2];
        size_t argc = 2;
        napi_value jsCallback;

        if (napi_get_cb_info(env, info, &argc, value, &jsCallback, nullptr) != napi_ok) {
        return result;
        }
        napi_create_reference(env, value[1], 1, &mRefFuncProgress);
        minizipNapi::mRef = mRefFuncProgress;
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0527", "Compress222");
        // 创建Promise对象, promise用于原生方法返回 deferred传入异步工作项的上下文数据
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        napi_create_promise(env, &deferred, &promise);

        // 异步工作项上下文用户数据，传递到异步工作项的execute、complete之间传递数据
        minizipWorkData *async_data = new minizipWorkData{
            .asyncwork = nullptr,
            .deferred = deferred,
            .parCount = 1,
            .arrpar = value[0],
            .funcZip = minizip_parser_params2,
        };
        memset(&async_data->opt, 0x00, sizeof(minizip_opt));
        memset(&async_data->optother, 0x00, sizeof(minizip_opt_other_s));

        minizip_parser_params_get(env, value[0], &async_data->opt, &async_data->optother);

        // 异步执行压缩操作，创建async work，创建成功后通过最后一个参数返回async work的handle
        napi_value work_name;
        napi_create_string_utf8(env, "napi_create_threadsafe_function", NAPI_AUTO_LENGTH, &work_name);

        minizipNapi::mEnv = env;
        napi_create_threadsafe_function(env, value[1], nullptr, work_name, 0, 1, nullptr, nullptr, (void *)async_data,
                                        DoDecompressWork, &g_callbackFunc);
        napi_call_threadsafe_function(g_callbackFunc, nullptr, napi_tsfn_nonblocking);

        if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
        }

        return promise;
}


napi_value minizipNapi::Decompress(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value[2];
    size_t argc = 2;
    napi_value jsCallback = nullptr;

    if (napi_get_cb_info(env, info, &argc, value, &jsCallback, nullptr) != napi_ok) {
        return result;
    }
    napi_create_reference(env, value[1], 1, &mRefFuncProgress);
    minizipNapi::mRef = mRefFuncProgress;
    
    //创建Promise对象,promise用于原生方法返回，deferred传入异步工作项的上下文数据
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    
    minizipWorkData *async_data = new minizipWorkData{
        .asyncwork = nullptr,
        .deferred = deferred,
        .parCount = 1,
        .arrpar = value[0],
        .funcZip = minizip_parser_params2,
    };
    memset(&async_data->opt, 0x00, sizeof(minizip_opt));
    memset(&async_data->optother, 0x00, sizeof(minizip_opt_other_s));
    
    minizip_parser_params_get(env, value[0], &async_data->opt, &async_data->optother);

    // 异步执行解压操作,创建async work，创建成功后通过最后一个参数(addonData->asyncWork)返回async work的handle
    napi_value work_name;
    napi_create_string_utf8(env, "napi_create_threadsafe_function", NAPI_AUTO_LENGTH, &work_name);
    
    minizipNapi::mEnv = env;
    napi_create_threadsafe_function(env, value[1], nullptr, work_name, 0, 1, nullptr, nullptr, (void*)async_data, DoDecompressWork, &g_callbackFunc);
    napi_call_threadsafe_function(g_callbackFunc, nullptr, napi_tsfn_nonblocking);

    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return promise;
}

napi_value minizipNapi::CompressWithPsd(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value[2];
    size_t argc = 2;
    napi_value jsCallback = nullptr;
    if (napi_get_cb_info(env, info, &argc, value, nullptr, nullptr) != napi_ok) {
        return result;
    }
    napi_create_reference(env, value[1], 1, &mRefFuncProgress);
    minizipNapi::mRef = mRefFuncProgress;

    // 创建Promise对象,promise用于原生方法返回，deferred传入异步工作项的上下文数据
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    // 异步工作项上下文用户数据，传递到异步工作项的execute、complete之间传递数据
    minizipWorkData *async_data = new minizipWorkData{
        .asyncwork = nullptr,
        .deferred = deferred,
        // env = env,
        .parCount = 1,
        .arrpar = value[0],
        //.refZip = refZip,
        .funcZip = minizip_parser_params2,
        //.par2 = value[1],
    };
    memset(&async_data->opt, 0x00, sizeof(minizip_opt));
    memset(&async_data->optother, 0x00, sizeof(minizip_opt_other_s));

    minizip_parser_params_get(env, value[0], &async_data->opt, &async_data->optother);

    // 异步执行解压操作,创建async work，创建成功后通过最后一个参数(addonData->asyncWork)返回async work的handle
    napi_value work_name;
    napi_create_string_utf8(env, "napi_create_threadsafe_function", NAPI_AUTO_LENGTH, &work_name);
    minizipNapi::mEnv = env;
    napi_create_threadsafe_function(env, value[1], nullptr, work_name, 0, 1, nullptr, nullptr, (void *)async_data,
                                    DoDecompressWork, &g_callbackFunc);
    napi_call_threadsafe_function(g_callbackFunc, nullptr, napi_tsfn_nonblocking);
    
    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return promise;
}

napi_value minizipNapi::DecompressWithPsd(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value[2];
    size_t argc = 2;
    napi_value jsCallback = nullptr;

    if (napi_get_cb_info(env, info, &argc, value, nullptr, nullptr) != napi_ok) {
        return result;
    }
    napi_create_reference(env, value[1], 1, &mRefFuncProgress);
    minizipNapi::mRef = mRefFuncProgress;

    // 创建Promise对象,promise用于原生方法返回，deferred传入异步工作项的上下文数据
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    // 异步工作项上下文用户数据，传递到异步工作项的execute、complete之间传递数据
    minizipWorkData *async_data = new minizipWorkData{
        .asyncwork = nullptr,
        .deferred = deferred,
        // env = env,
        .parCount = 1,
        .arrpar = value[0],
        //.refZip = refZip,
        .funcZip = minizip_parser_params2,
        //.par2 = value[1],
    };
    memset(&async_data->opt, 0x00, sizeof(minizip_opt));
    memset(&async_data->optother, 0x00, sizeof(minizip_opt_other_s));

    minizip_parser_params_get(env, value[0], &async_data->opt, &async_data->optother);
    // 异步执行解压操作,创建async work，创建成功后通过最后一个参数(addonData->asyncWork)返回async work的handle
    napi_value work_name;
    napi_create_string_utf8(env, "napi_create_threadsafe_function", NAPI_AUTO_LENGTH, &work_name);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0527", "napi_create_threadsafe_function111");

    minizipNapi::mEnv = env;
    napi_create_threadsafe_function(env, value[1], nullptr, work_name, 0, 1, nullptr, nullptr, (void *)async_data,
                                    DoDecompressWork, &g_callbackFunc);
    napi_call_threadsafe_function(g_callbackFunc, nullptr, napi_tsfn_nonblocking);
    
    if (napi_create_int64(env, 0, &result) != napi_ok) {
        std::cout << "napi_create_int64" << std::endl;
    }

    return promise;
}

napi_value minizipNapi::IsPasswordProtected(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value;
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, &value, nullptr, nullptr) != napi_ok) {
        return result;
    }
    minizipWorkData *async_data = new minizipWorkData{
        .asyncwork = nullptr,
        .deferred = nullptr,
        .parCount = 1,
        .arrpar = value,
        .funcZip = minizip_parser_params2,
    };
    memset(&async_data->opt, 0x00, sizeof(minizip_opt));
    memset(&async_data->optother, 0x00, sizeof(minizip_opt_other_s));
    minizip_parser_params_get(env, value, &async_data->opt, &async_data->optother);
    if (minizip_getObjectPropetry(env, value, "path", STRING, (void *)async_data->optother.path) < 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                         "minizip_getObjectPropetry fail");
    }
    if (napi_get_boolean(env, minizip_is_password_protected((const char *)async_data->optother.path), &result) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                     "napi_coerce_to_bool fail");
        return nullptr;
    }
    return result;
}

napi_value minizipNapi::GetUnCompressedSize(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value value;
    size_t argc = 1;

    if (napi_get_cb_info(env, info, &argc, &value, nullptr, nullptr) != napi_ok) {
        return result;
    }
    minizipWorkData *async_data = new minizipWorkData{
        .asyncwork = nullptr,
        .deferred = nullptr,
        // env = env,
        .parCount = 1,
        .arrpar = value,
        //.refZip = refZip,
        .funcZip = minizip_parser_params2,
        //.par2 = value[1],
    };
    memset(&async_data->opt, 0x00, sizeof(minizip_opt));
    memset(&async_data->optother, 0x00, sizeof(minizip_opt_other_s));
    minizip_parser_params_get(env, value, &async_data->opt, &async_data->optother);

    if (napi_create_int32(env, minizip_get_uncompressed_size((const char *)async_data->optother.path), &result) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::GetUnCompressSize", "int32_t_create_to_napi fail");
        return nullptr;
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
        {"decompressWithPsd", nullptr, minizipNapi::DecompressWithPsd, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isPasswordProtected", nullptr, minizipNapi::IsPasswordProtected, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getUnCompressedSize", nullptr, minizipNapi::GetUnCompressedSize, nullptr, nullptr, nullptr, napi_default, nullptr}};

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
