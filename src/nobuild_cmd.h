#pragma once

#include "nobuild_cstr.h"

#ifndef _WIN32
#	include <sys/types.h>
typedef pid_t Pid;
typedef int Fd;
#else
#	define WIN32_MEAN_AND_LEAN
#	include <windows.h>
typedef HANDLE Pid;
typedef HANDLE Fd;
#endif

typedef struct {
    Cstr_Array line;
} Cmd;

Cstr cmd_show(Cmd cmd);
Pid cmd_run_async(Cmd cmd, Fd *fdin, Fd *fdout);
void cmd_run_sync(Cmd cmd);

// TODO(#1): no way to disable echo in nobuild scripts
// TODO(#2): no way to ignore fails
#define CMD(...)                                        \
    do {                                                \
        Cmd cmd = {                                     \
            .line = cstr_array_make(__VA_ARGS__, NULL)  \
        };                                              \
        INFO("CMD: %s", cmd_show(cmd));                 \
        cmd_run_sync(cmd);                              \
    } while (0)

typedef struct {
    Cmd *elems;
    size_t count;
} Cmd_Array;

typedef enum {
    CHAIN_TOKEN_END = 0,
    CHAIN_TOKEN_IN,
    CHAIN_TOKEN_OUT,
    CHAIN_TOKEN_CMD
} Chain_Token_Type;

// A single token for the CHAIN(...) DSL syntax
typedef struct {
    Chain_Token_Type type;
    Cstr_Array args;
} Chain_Token;

#define CHAIN_IN(path)                      \
    (Chain_Token) {                         \
        .type = CHAIN_TOKEN_IN,             \
        .args = cstr_array_make(path, NULL) \
    }

#define CHAIN_OUT(path)                     \
    (Chain_Token) {                         \
        .type = CHAIN_TOKEN_OUT,            \
        .args = cstr_array_make(path, NULL) \
    }

#define CHAIN_CMD(...)                             \
    (Chain_Token) {                                \
        .type = CHAIN_TOKEN_CMD,                   \
        .args = cstr_array_make(__VA_ARGS__, NULL) \
    }

// TODO(#20): pipes do not allow redirecting stderr
typedef struct {
    Cstr input_filepath;
    Cmd_Array cmds;
    Cstr output_filepath;
} Chain;

Chain chain_build_from_tokens(Chain_Token first, ...);
void chain_run_sync(Chain chain);
void chain_echo(Chain chain);

// TODO(#15): PIPE does not report where exactly a syntactic error has happened
#define CHAIN(...)                                                             \
    do {                                                                       \
        Chain chain = chain_build_from_tokens(__VA_ARGS__, (Chain_Token) {0}); \
        chain_echo(chain);                                                     \
        chain_run_sync(chain);                                                 \
    } while(0)

