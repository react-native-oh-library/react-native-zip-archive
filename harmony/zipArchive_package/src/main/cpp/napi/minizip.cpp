//
// Created on 2024/5/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include <bits/alltypes.h>
#include <js_native_api.h>
#include <js_native_api_types.h>

#include <stdio.h>

#include "minizip.h"
#include <hilog/log.h>
const unsigned int LOG_PRINT_DOMAIN = 0XFF00;
/***************************************************************************/

int32_t minizip_add_entry_cb(void *handle, void *userdata, mz_zip_file *file_info);
int32_t minizip_add_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position, float &refProgress);
int32_t minizip_add_overwrite_cb(void *handle, void *userdata, const char *path);

int32_t minizip_extract_entry_cb(void *handle, void *userdata, mz_zip_file *file_info, const char *path);
int32_t minizip_extract_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position, float &refProgress, int &uncompressedSize);
int32_t minizip_extract_overwrite_cb(void *handle, void *userdata, mz_zip_file *file_info, const char *path);

/***************************************************************************/

int32_t minizip_list(const char *path) {
    mz_zip_file *file_info = NULL;
    uint32_t ratio = 0;
    int32_t err = MZ_OK;
    struct tm tmu_date;
    const char *method = NULL;
    char crypt = ' ';
    void *reader = NULL;


    reader = mz_zip_reader_create();
    err = mz_zip_reader_open_file(reader, path);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive %s\n", err, path);
        mz_zip_reader_delete(&reader);
        return err;
    }

    err = mz_zip_reader_goto_first_entry(reader);

    if (err != MZ_OK && err != MZ_END_OF_LIST) {
        printf("Error %" PRId32 " going to first entry in archive\n", err);
        mz_zip_reader_delete(&reader);
        return err;
    }

    printf("      Packed     Unpacked Ratio Method   Attribs Date     Time  CRC-32     Name\n");
    printf("      ------     -------- ----- ------   ------- ----     ----  ------     ----\n");

    /* Enumerate all entries in the archive */
    do {
        err = mz_zip_reader_entry_get_info(reader, &file_info);

        if (err != MZ_OK) {
            printf("Error %" PRId32 " getting entry info in archive\n", err);
            break;
        }

        ratio = 0;
        if (file_info->uncompressed_size > 0)
            ratio = (uint32_t)((file_info->compressed_size * 100) / file_info->uncompressed_size);

        /* Display a '*' if the file is encrypted */
        if (file_info->flag & MZ_ZIP_FLAG_ENCRYPTED)
            crypt = '*';
        else
            crypt = ' ';

        method = mz_zip_get_compression_method_string(file_info->compression_method);
        mz_zip_time_t_to_tm(file_info->modified_date, &tmu_date);

        /* Print entry information */
        printf("%12" PRId64 " %12" PRId64 "  %3" PRIu32 "%% %6s%c %8" PRIx32 " %2.2" PRIu32 "-%2.2" PRIu32
               "-%2.2" PRIu32 " %2.2" PRIu32 ":%2.2" PRIu32 " %8.8" PRIx32 "   %s\n",
               file_info->compressed_size, file_info->uncompressed_size, ratio, method, crypt, file_info->external_fa,
               (uint32_t)tmu_date.tm_mon + 1, (uint32_t)tmu_date.tm_mday, (uint32_t)tmu_date.tm_year % 100,
               (uint32_t)tmu_date.tm_hour, (uint32_t)tmu_date.tm_min, file_info->crc, file_info->filename);

        err = mz_zip_reader_goto_next_entry(reader);

        if (err != MZ_OK && err != MZ_END_OF_LIST) {
            printf("Error %" PRId32 " going to next entry in archive\n", err);
            break;
        }
    } while (err == MZ_OK);

    mz_zip_reader_delete(&reader);

    if (err == MZ_END_OF_LIST)
        return MZ_OK;

    return err;
}

/***************************************************************************/

int32_t minizip_add_entry_cb(void *handle, void *userdata, mz_zip_file *file_info) {
    MZ_UNUSED(handle);
    MZ_UNUSED(userdata);

    /* Print the current file we are trying to compress */
    printf("Adding %s\n", file_info->filename);
    return MZ_OK;
}

int32_t minizip_add_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position, float &refProgress) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528", "start minizip_add_progress_cb");
    minizip_opt *options = (minizip_opt *)userdata;
    double progress = 0;
    uint8_t raw = 0;

    MZ_UNUSED(userdata);

    mz_zip_writer_get_raw(handle, &raw);

    if (raw && file_info->compressed_size > 0)
        progress = ((double)position / file_info->compressed_size) * 100;
    else if (!raw && file_info->uncompressed_size > 0)
        progress = ((double)position / file_info->uncompressed_size) * 100;

    refProgress = progress;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528", "refProgress111 = %{public}f", refProgress);
    /* Print the progress of the current compress operation */
    if (options->verbose)
        printf("%s - %" PRId64 " / %" PRId64 " (%.02f%%)\n", file_info->filename, position,
               file_info->uncompressed_size, progress);
    return MZ_OK;
}

int32_t minizip_add_overwrite_cb(void *handle, void *userdata, const char *path) {
    minizip_opt *options = (minizip_opt *)userdata;

    MZ_UNUSED(handle);

    if (options->overwrite == 0) {
        /* If ask the user what to do because append and overwrite args not set */
        char rep = 0;
        do {
            char answer[128];
            printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ", path);
            if (scanf("%1s", answer) != 1)
                exit(EXIT_FAILURE);
            rep = answer[0];

            if ((rep >= 'a') && (rep <= 'z'))
                rep -= 0x20;
        } while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));

        if (rep == 'A') {
            return MZ_EXIST_ERROR;
        } else if (rep == 'N') {
            return MZ_INTERNAL_ERROR;
        }
    }

    return MZ_OK;
}

int32_t minizip_add(const char *path, const char *password, minizip_opt *options, int32_t arg_count,
                    const char **args,void (*funcProgressCallbackzip)(float progress)) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528", "start minizip_add");
    void *writer = NULL;
    int32_t err = MZ_OK;
    int32_t err_close = MZ_OK;
    int32_t i = 0;
    const char *filename_in_zip = NULL;

    //   HILOGI("Archive %{public}s\n", path);
    printf("Archive %s\n", path);

    /* Create zip writer */
    writer = mz_zip_writer_create();
    mz_zip_writer_set_password(writer, password);
    mz_zip_writer_set_aes(writer, options->aes);
    mz_zip_writer_set_compress_method(writer, options->compress_method);
    mz_zip_writer_set_compress_level(writer, options->compress_level);
    mz_zip_writer_set_follow_links(writer, options->follow_links);
    mz_zip_writer_set_store_links(writer, options->store_links);
    mz_zip_writer_set_overwrite_cb(writer, options, minizip_add_overwrite_cb);
    mz_zip_writer_set_progress_cb(writer, options, minizip_add_progress_cb, funcProgressCallbackzip);
    mz_zip_writer_set_entry_cb(writer, options, minizip_add_entry_cb);
    mz_zip_writer_set_zip_cd(writer, options->zip_cd);
    // HILOGI("cert_path : %{public}s", options->cert_path);
    if (options->cert_path != NULL)
        mz_zip_writer_set_certificate(writer, options->cert_path, options->cert_pwd);
    err = mz_zip_writer_open_file(writer, path, options->disk_size, options->append);

    if (err == MZ_OK) {
        for (i = 0; i < arg_count; i += 1) {
            filename_in_zip = args[i];
            /* Add file system path to archive */
            err = mz_zip_writer_add_path(writer, filename_in_zip, NULL, options->include_path, 1);
            if (err != MZ_OK) {
                printf("Error %" PRId32 " adding path to archive %s\n", err, filename_in_zip);
                //            HILOGE("[%{public}d]Error %{public}" PRId32 " adding path to archive %{public}s,
                //            errcode:%{public}d(%{public}s)\n", __LINE__, err, filename_in_zip,errno, strerror(errno));
            }
        }
    } else {
        printf("Error %" PRId32 " opening archive for writing\n", err);
        //    HILOGE("[%{public}d]Error %{public}" PRId32 " opening archive for writing[%{public}s]\n", __LINE__, err,
        //    strerror(errno));
    }

    err_close = mz_zip_writer_close(writer);
    if (err_close != MZ_OK) {
        printf("Error %" PRId32 " closing archive for writing %s\n", err_close, path);
        //    HILOGE("[%{public}d]Error %{public}" PRId32 " adding path to archive %{public}s\n", __LINE__, err_close,
        //    path);
        err = err_close;
    }

    mz_zip_writer_delete(&writer);
    return err;
}

/***************************************************************************/

int32_t minizip_extract_entry_cb(void *handle, void *userdata, mz_zip_file *file_info, const char *path) {
    MZ_UNUSED(handle);
    MZ_UNUSED(userdata);
    MZ_UNUSED(path);

    /* Print the current entry extracting */
    printf("Extracting %s\n", file_info->filename);
    return MZ_OK;
}

int32_t minizip_extract_progress_cb(void *handle, void *userdata, mz_zip_file *file_info, int64_t position, float &refProgress, int &uncompressedSize) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "start minizip_extract_progress_cb");
    minizip_opt *options = (minizip_opt *)userdata;
    double progress = 0;
    uint8_t raw = 0;

    MZ_UNUSED(userdata);

    mz_zip_reader_get_raw(handle, &raw);

    if (raw && file_info->compressed_size > 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "position1 = %{public}f", position);
        progress = ((double)position / file_info->compressed_size) * 100;
    }
    else if (!raw && file_info->uncompressed_size > 0) {
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "position2 = %{public}f", position);
            progress = ((double)position / file_info->uncompressed_size) * 100;
    }
        

    refProgress = progress;
    uncompressedSize = file_info->uncompressed_size - position;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "refProgress = %{public}f, uncompressedSize11111 = %{public}lld", refProgress, uncompressedSize);
    /* Print the progress of the current extraction */
    if (options->verbose)
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "start options->verbose");
        printf("%s - %" PRId64 " / %" PRId64 " (%.02f%%)\n", file_info->filename, position,
               file_info->uncompressed_size, progress);

    return MZ_OK;
}

int32_t minizip_extract_overwrite_cb(void *handle, void *userdata, mz_zip_file *file_info, const char *path) {
    minizip_opt *options = (minizip_opt *)userdata;

    MZ_UNUSED(handle);
    MZ_UNUSED(file_info);

    /* Verify if we want to overwrite current entry on disk */
    if (options->overwrite == 0) {
        char rep = 0;
        do {
            char answer[128];
            printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ", path);
            if (scanf("%1s", answer) != 1)
                exit(EXIT_FAILURE);
            rep = answer[0];
            if ((rep >= 'a') && (rep <= 'z'))
                rep -= 0x20;
        } while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));

        if (rep == 'N')
            return MZ_EXIST_ERROR;
        if (rep == 'A')
            options->overwrite = 1;
    }

    return MZ_OK;
}

int32_t minizip_extract(const char *path, const char *pattern, const char *destination, const char *password,
                        minizip_opt *options, void (*funcProgressCallback)(float progress, int uncompressSize)) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "start minizip_extract");
    void *reader = NULL;
    int32_t err = MZ_OK;
    int32_t err_close = MZ_OK;


    printf("Archive %s\n", path);
    //   HILOGI("path:%{public}s, pattern:%{public}s, destination:%{public}s, password:%{public}s \n", path, pattern,
    //   destination, password);
    /* Create zip reader */
    reader = mz_zip_reader_create();
    mz_zip_reader_set_pattern(reader, pattern, 1);
    mz_zip_reader_set_password(reader, password);
    mz_zip_reader_set_encoding(reader, options->encoding);
    mz_zip_reader_set_entry_cb(reader, options, minizip_extract_entry_cb);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "start mz_zip_reader_set_progress_cb000");
    mz_zip_reader_set_progress_cb(reader, options, minizip_extract_progress_cb, funcProgressCallback);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0514", "start mz_zip_reader_set_progress_cb");
    mz_zip_reader_set_overwrite_cb(reader, options, minizip_extract_overwrite_cb);

    err = mz_zip_reader_open_file(reader, path);

    if (err != MZ_OK) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0522", "start err111111111111");
        printf("Error %" PRId32 " opening archive %s\n", err, path);
        //    HILOGE("Error %{public}" PRId32 " opening archive %{public}s\n", err, path);
    } else {
        /* Save all entries in archive to destination directory */
        err = mz_zip_reader_save_all(reader, destination);

        if (err == MZ_END_OF_LIST) {
            if (pattern != NULL) {
                printf("Files matching %s not found in archive\n", pattern);
                //     HILOGE("Files matching %{public}s not found in archive\n", pattern);
            } else {
                printf("No files in archive\n");
                //       HILOGE("No files in archive\n");
                err = MZ_OK;
            }
        } else if (err != MZ_OK) {
            printf("Error %" PRId32 " saving entries to disk %s\n", err, path);
        }
    }
    err_close = mz_zip_reader_close(reader);
    if (err_close != MZ_OK) {
        printf("Error %" PRId32 " closing archive for reading\n", err_close);
        err = err_close;
    }

    mz_zip_reader_delete(&reader);
    return err;
}

/***************************************************************************/

int32_t minizip_erase(const char *src_path, const char *target_path, int32_t arg_count, const char **args) {
    mz_zip_file *file_info = NULL;
    const char *filename_in_zip = NULL;
    const char *target_path_ptr = target_path;
    void *reader = NULL;
    void *writer = NULL;
    int32_t skip = 0;
    int32_t err = MZ_OK;
    int32_t i = 0;
    uint8_t zip_cd = 0;
    char bak_path[256];
    char tmp_path[256];

    if (target_path == NULL) {
        /* Construct temporary zip name */
        strncpy(tmp_path, src_path, sizeof(tmp_path) - 1);
        tmp_path[sizeof(tmp_path) - 1] = 0;
        strncat(tmp_path, ".tmp.zip", sizeof(tmp_path) - strlen(tmp_path) - 1);
        target_path_ptr = tmp_path;
    }

    reader = mz_zip_reader_create();
    writer = mz_zip_writer_create();

    /* Open original archive we want to erase an entry in */
    err = mz_zip_reader_open_file(reader, src_path);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive for reading %s\n", err, src_path);
        mz_zip_reader_delete(&reader);
        return err;
    }

    /* Open temporary archive */
    err = mz_zip_writer_open_file(writer, target_path_ptr, 0, 0);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive for writing %s\n", err, target_path_ptr);
        mz_zip_reader_delete(&reader);
        mz_zip_writer_delete(&writer);
        return err;
    }

    err = mz_zip_reader_goto_first_entry(reader);

    if (err != MZ_OK && err != MZ_END_OF_LIST)
        printf("Error %" PRId32 " going to first entry in archive\n", err);

    while (err == MZ_OK) {
        err = mz_zip_reader_entry_get_info(reader, &file_info);
        if (err != MZ_OK) {
            printf("Error %" PRId32 " getting info from archive\n", err);
            break;
        }

        /* Copy all entries from original archive to temporary archive
           except the ones we don't want */
        for (i = 0, skip = 0; i < arg_count; i += 1) {
            filename_in_zip = args[i];

            if (mz_path_compare_wc(file_info->filename, filename_in_zip, 1) == MZ_OK)
                skip = 1;
        }

        if (skip) {
            printf("Skipping %s\n", file_info->filename);
        } else {
            printf("Copying %s\n", file_info->filename);
            err = mz_zip_writer_copy_from_reader(writer, reader);
        }

        if (err != MZ_OK) {
            printf("Error %" PRId32 " copying entry into new zip\n", err);
            break;
        }

        err = mz_zip_reader_goto_next_entry(reader);

        if (err != MZ_OK && err != MZ_END_OF_LIST)
            printf("Error %" PRId32 " going to next entry in archive\n", err);
    }

    mz_zip_reader_get_zip_cd(reader, &zip_cd);
    mz_zip_writer_set_zip_cd(writer, zip_cd);

    mz_zip_reader_close(reader);
    mz_zip_reader_delete(&reader);

    mz_zip_writer_close(writer);
    mz_zip_writer_delete(&writer);

    if (err == MZ_END_OF_LIST) {
        if (target_path == NULL) {
            /* Swap original archive with temporary archive, backup old archive if possible */
            strncpy(bak_path, src_path, sizeof(bak_path) - 1);
            bak_path[sizeof(bak_path) - 1] = 0;
            strncat(bak_path, ".bak", sizeof(bak_path) - strlen(bak_path) - 1);

            if (mz_os_file_exists(bak_path) == MZ_OK)
                mz_os_unlink(bak_path);

            if (mz_os_rename(src_path, bak_path) != MZ_OK)
                printf("Error backing up archive before replacing %s\n", bak_path);

            if (mz_os_rename(tmp_path, src_path) != MZ_OK)
                printf("Error replacing archive with temp %s\n", tmp_path);
        }

        return MZ_OK;
    }

    return err;
}

bool minizip_is_password_protected(const char *path) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                 "password_protected const char *path:%{public}s", path); // 沙箱路径
    if (path == NULL) {
        return false;
    }
    mz_zip_file *file_info = NULL;
    void *reader = NULL;
    int32_t err = MZ_OK;

    reader = mz_zip_reader_create();
    if (!reader) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                     "password_protected mz_zip_reader_create fail and return false");
        return false;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                 "before mz_zip_reader_open_file");
    err = mz_zip_reader_open_file(reader, path);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528",
                 "lmm in mz_zip_reader_open_file first err = %{public}d",
                 err); // -107 MZ_EXIST_ERROR
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive %s\n", err, path);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                     "password_protected mz_zip_reader_open_file fail and return false");
        mz_zip_reader_delete(&reader);
        return false;
    }

    err = mz_zip_reader_goto_first_entry(reader);
    if (err != MZ_OK && err != MZ_END_OF_LIST) {
        printf("Error %" PRId32 " going to first entry in archive\n", err);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::IsPasswordProtected",
                     "password_protected mz_zip_reader_goto_first_entry fail and return false");
        mz_zip_reader_delete(&reader);
        return false;
    }

    do {
        err = mz_zip_reader_entry_get_info(reader, &file_info);
        if (err != MZ_OK) {
            printf("Error %" PRId32 " getting entry info in archive\n", err);
            break;
        }
        int32_t encrypted = file_info->flag & MZ_ZIP_FLAG_ENCRYPTED;

        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528", "encrypted = %{public}d", encrypted);
        if (file_info->flag & MZ_ZIP_FLAG_ENCRYPTED) {
            return true;
        }
        err = mz_zip_reader_goto_next_entry(reader);
        if (err != MZ_OK && err != MZ_END_OF_LIST) {
            printf("Error %" PRId32 " going to next entry in archive\n", err);
            break;
        }
    } while (err == MZ_OK);

    mz_zip_reader_delete(&reader);
    return false;
}

int32_t minizip_get_uncompressed_size(const char *path) {
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::GetUnCompressSize",
                 "getUncompressedsize const char *path:%{public}s", path);
    if (path == NULL) {
        return false;
    }
    mz_zip_file *file_info = NULL;
    void *reader = NULL;
    int32_t err = MZ_OK;

    reader = mz_zip_reader_create();
    if (!reader) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::GetUnCompressSize",
                     "mz_zip_reader_create fail and return -1");
        return -1;
    }

    err = mz_zip_reader_open_file(reader, path);
    if (err != MZ_OK) {
        printf("Error %" PRId32 " opening archive %s\n", err, path);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::GetUnCompressSize",
                     "mz_zip_reader_open_file fail and return -1");
        mz_zip_reader_delete(&reader);
        return -1;
    }

    err = mz_zip_reader_goto_first_entry(reader);
    if (err != MZ_OK && err != MZ_END_OF_LIST) {
        printf("Error %" PRId32 " going to first entry in archive\n", err);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "minizipNapi::GetUnCompressSize",
                     "mz_zip_reader_goto_first_entry fail and return false");
        mz_zip_reader_delete(&reader);
        return -1;
    }

    do {
        err = mz_zip_reader_entry_get_info(reader, &file_info);
        if (err != MZ_OK) {
            printf("Error %" PRId32 " getting entry info in archive\n", err);
            break;
        }
        int32_t unCompressedSize = file_info->uncompressed_size;
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "test-0528", "unCompressedSize = %{public}d",
                     unCompressedSize);
        if (unCompressedSize > 0) {
            return unCompressedSize;
        }
        err = mz_zip_reader_goto_next_entry(reader);
        if (err != MZ_OK && err != MZ_END_OF_LIST) {
            printf("Error %" PRId32 " going to next entry in archive\n", err);
            break;
        }
    } while (err == MZ_OK);

    mz_zip_reader_delete(&reader);
    return -1;
}
