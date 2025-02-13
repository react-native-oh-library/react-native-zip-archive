//
// Created on 2024/5/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef MYAPPLICATION_MINIZIP_H
#define MYAPPLICATION_MINIZIP_H

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_split.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct minizip_opt_s {
    uint8_t     include_path;
    int16_t     compress_level;
    uint8_t     compress_method;
    uint8_t     overwrite;
    uint8_t     append;
    int64_t     disk_size;
    uint8_t     follow_links;
    uint8_t     store_links;
    uint8_t     zip_cd;
    int32_t     encoding;
    uint8_t     verbose;
    uint8_t     aes;
    const char *cert_path;
    const char *cert_pwd;
} minizip_opt;

typedef struct minizip_opt_other_s {
    char path[512];
    char password[512];
    char optBuf[512];
    char file_extract[512];
    char destination[512];
    char cert_pwd[256] = {0};
    char cert_path[256] = {0};
    uint32_t argc = 0;
    char *argv[128];
} minizip_opt_other;


int32_t minizip_add(const char *path, const char *password, minizip_opt *options, int32_t arg_count, const char **args, void (*)(float));

int32_t minizip_extract(const char *path, const char *pattern, const char *destination, const char *password, minizip_opt *options, void (*)(float, int));

int32_t minizip_list(const char *path);

bool minizip_is_password_protected(const char *path);

int32_t minizip_get_uncompressed_size(const char *path);

#ifdef __cplusplus
}
#endif
#endif //MYAPPLICATION_MINIZIP_H
