#pragma once

#include "nobuild_log.h"
#include "nobuild_cstr.h"
#include "nobuild_io.h"
#include "nobuild_cmd.h"
#include "nobuild_path.h"

#define FOREACH_ARRAY(type, elem, array, body)                                  \
    for (size_t elem_##index = 0; elem_##index < array.count; ++elem_##index) { \
        type *elem = &array.elems[elem_##index];                                \
        body;                                                                   \
    }

#ifndef REBUILD_URSELF
#	if _WIN32
#		if defined(__GNUC__)
#			define REBUILD_URSELF(binary_path, source_path) CMD("gcc", "-o", binary_path, source_path)
#		elif defined(__clang__)
#			define REBUILD_URSELF(binary_path, source_path) CMD("clang", "-o", binary_path, source_path)
#		elif defined(_MSC_VER)
#			define REBUILD_URSELF(binary_path, source_path) CMD("cl.exe", source_path)
#		endif
#	else
#		define REBUILD_URSELF(binary_path, source_path) CMD("cc", "-o", binary_path, source_path)
#	endif
#endif

// Go Rebuild Urself™ Technology
//
//   How to use it:
//     int main(int argc, char** argv) {
//         GO_REBUILD_URSELF(argc, argv);
//         // actual work
//         return 0;
//     }
//
//   After your added this macro every time you run ./nobuild it will detect
//   that you modified its original source code and will try to rebuild itself
//   before doing any actual work. So you only need to bootstrap your build system
//   once.
//
//   The modification is detected by comparing the last modified times of the executable
//   and its source code. The same way the make utility usually does it.
//
//   The rebuilding is done by using the REBUILD_URSELF macro which you can redefine
//   if you need a special way of bootstraping your build system. (which I personally
//   do not recommend since the whole idea of nobuild is to keep the process of bootstrapping
//   as simple as possible and doing all of the actual work inside of the nobuild)
//
#define GO_REBUILD_URSELF(argc, argv)                                  \
    do {                                                               \
        const char *source_path = __FILE__;                            \
        assert(argc >= 1);                                             \
        const char *binary_path = argv[0];                             \
                                                                       \
        if (IS_NEWER(source_path, binary_path)) { \
            RENAME(binary_path, CONCAT(binary_path, ".old"));          \
            REBUILD_URSELF(binary_path, source_path);                  \
            Cmd cmd = {                                                \
                .line = {                                              \
                    .elems = (Cstr*) argv,                             \
                    .count = argc,                                     \
                },                                                     \
            };                                                         \
            INFO("CMD: %s", cmd_show(cmd));                            \
            cmd_run_sync(cmd);                                         \
            exit(0);                                                   \
        }                                                              \
    } while(0)

char *shift_args(int *argc, char ***argv);

void file_to_c_array(Cstr path, Cstr out_path, Cstr array_type,  Cstr array_name, int null_term);
#define FILE_TO_C_ARRAY(path, out_path, array_name) file_to_c_array(path, out_path, "unsigned char", array_name, 1)