#pragma once

#include "nobuild_cstr.h"

#ifndef NOBUILD__DEPRECATED
#	if defined(__GNUC__) || (defined(__clang__) && !defined(_MSC_VER))
#		define NOBUILD__DEPRECATED(func) __attribute__ ((deprecated)) func
#	elif defined(_MSC_VER)
#		define NOBUILD__DEPRECATED(func) __declspec (deprecated) func
#	endif
#endif

#ifndef _WIN32
#	define PATH_SEP "/"
#else
#	define PATH_SEP "\\"
#endif

#define PATH(...) JOIN(PATH_SEP, __VA_ARGS__)

Cstr path_no_ext(Cstr path);
#define NOEXT(path) path_no_ext(path)

Cstr path_dirname(Cstr path);
#define DIRNAME(path) path_dirname(path)

Cstr path_basename(Cstr path);
#define BASENAME(path) path_basename(path)

int path_is_dir(Cstr path);
#define IS_DIR(path) path_is_dir(path)

int path_is_file(Cstr path);
#define IS_FILE(path) path_is_file(path)

int path_exists(Cstr path);
#define PATH_EXISTS(path) path_exists(path)

NOBUILD__DEPRECATED(int is_path1_modified_after_path2(Cstr path1, Cstr path2));
int path_is_newer(Cstr path1, Cstr path2);
#define IS_NEWER(path1, path2) path_is_newer(path1, path2)

void path_mkdirs(Cstr_Array path);
#define MKDIRS(...)                                             \
    do {                                                        \
        Cstr_Array path = cstr_array_make(__VA_ARGS__, NULL);   \
        INFO("MKDIRS: %s", cstr_array_join(PATH_SEP, path));    \
        path_mkdirs(path);                                      \
    } while (0)

void path_rename(Cstr old_path, Cstr new_path);
#define RENAME(old_path, new_path)                    \
    do {                                              \
        INFO("RENAME: %s -> %s", old_path, new_path); \
        path_rename(old_path, new_path);              \
    } while (0)

void path_copy(Cstr old_path, Cstr new_path);
#define COPY(old_path, new_path)                    \
    do {                                            \
        INFO("COPY: %s -> %s", old_path, new_path); \
        path_copy(old_path, new_path);              \
    } while(0)

void path_rm(Cstr path);
#define RM(path)                                \
    do {                                        \
        INFO("RM: %s", path);                   \
        path_rm(path);                          \
    } while(0)

#define FOREACH_FILE_IN_DIR(file, dirpath, body)        \
    do {                                                \
        struct dirent *dp = NULL;                       \
        DIR *dir = opendir(dirpath);                    \
        if (dir == NULL) {                              \
            PANIC("could not open directory %s: %s",    \
                  dirpath, nobuild__strerror(errno));   \
        }                                               \
        errno = 0;                                      \
        while ((dp = readdir(dir))) {                   \
            const char *file = dp->d_name;              \
            body;                                       \
        }                                               \
                                                        \
        if (errno > 0) {                                \
            PANIC("could not read directory %s: %s",    \
                  dirpath, nobuild__strerror(errno));   \
        }                                               \
                                                        \
        closedir(dir);                                  \
    } while(0)