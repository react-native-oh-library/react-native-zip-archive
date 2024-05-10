//
// Created on 2024/5/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef MYAPPLICATION_MINIZIPNAPI_H
#define MYAPPLICATION_MINIZIPNAPI_H

#include "napi/native_api.h"

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_split.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
#include <js_native_api.h>
#include <js_native_api_types.h>

#define NAPI_CLASS_NAME "minizipNapiClass"

class minizipNapi {
public:
    minizipNapi() {}
    ~minizipNapi() {}

    static napi_value Create(napi_env env, napi_callback_info info);
    static napi_value Init(napi_env env, napi_value exports);

private:
    static napi_value Compress(napi_env env, napi_callback_info info);
    static napi_value Decompress(napi_env env, napi_callback_info info);
    static napi_value CompressWithPsd(napi_env env, napi_callback_info info);
    static napi_value DecompressWithPsd(napi_env env, napi_callback_info info);

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalize);

    static napi_ref sConstructor_;


    napi_env mEnv = nullptr;
    napi_ref mRef = nullptr;
};

#endif //MYAPPLICATION_MINIZIPNAPI_H
