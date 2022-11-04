#define CKB_C_STDLIB_PRINTF
#include <stdio.h>

// it's used by blockchain-api2.h, the behavior when panic
#ifndef MOL2_EXIT
#define MOL2_EXIT ckb_exit
#endif
int ckb_exit(signed char);

#include <stdbool.h>
#include <string.h>

#include "blake2b.h"
#include "blockchain.h"
#include "ckb_syscalls.h"
#include "ckb_dlfcn.h"
#include "ckb_consts.h"

#define MAX_CODE_SIZE (1024 * 1024)
#define SCRIPT_SIZE (32 * 1024)

#define RESERVED_ARGS_SIZE 2
#define BLAKE2B_BLOCK_SIZE 32
#define HASH_TYPE_SIZE 1

enum ErrorCode {
    // 0 is the only success code. We can use 0 directly.

    // inherit from simple_udt
    ERROR_ARGUMENTS_LEN = -1,
    ERROR_ENCODING = -2,
    ERROR_SYSCALL = -3,
    ERROR_SCRIPT_TOO_LONG = -21,
    ERROR_OVERFLOWING = -51,
    ERROR_AMOUNT = -52,

    // error code is starting from 40, to avoid conflict with
    // common error code in other scripts.
    ERROR_CANT_LOAD_LIB = 40,
    ERROR_LIB_MALFORMED,
    ERROR_CANT_FIND_SYMBOL,
    ERROR_INVALID_ARGS_FORMAT,
};

uint8_t code_buff[MAX_CODE_SIZE] __attribute__((aligned(RISCV_PGSIZE)));

int get_dylib_handle(void** handle) {
    unsigned char script[SCRIPT_SIZE];
    uint64_t len = SCRIPT_SIZE;
    printf("loading script\n");
    int err = ckb_load_script(script, &len, 0);
    if (err != 0) {
        printf("loading script error %d\n", err);
        goto exit;
    }
    if (len > SCRIPT_SIZE) {
        err = ERROR_SCRIPT_TOO_LONG;
        goto exit;
    }

    mol_seg_t script_seg;
    script_seg.ptr = (uint8_t*)script;
    script_seg.size = len;

    printf("verifiying script\n");
    if (MolReader_Script_verify(&script_seg, false) != MOL_OK) {
        err = ERROR_ENCODING;
        goto exit;
    }

    // The script arguments are in the following format
    // <lua loader args, 2 bytes> <code hash of lua code, 32 bytes>
    // <hash type of lua code, 1 byte> <lua script args, variable length>
    mol_seg_t args_seg = MolReader_Script_get_args(&script_seg);
    mol_seg_t args_bytes_seg = MolReader_Bytes_raw_bytes(&args_seg);

    if (args_bytes_seg.size <
        RESERVED_ARGS_SIZE + BLAKE2B_BLOCK_SIZE + HASH_TYPE_SIZE) {
        err = ERROR_INVALID_ARGS_FORMAT;
        goto exit;
    }

    uint8_t* code_hash = args_bytes_seg.ptr + RESERVED_ARGS_SIZE;
    uint8_t hash_type =
        *(args_bytes_seg.ptr + RESERVED_ARGS_SIZE + BLAKE2B_BLOCK_SIZE);

    size_t code_buff_size = MAX_CODE_SIZE;
    size_t consumed_size = 0;

    printf("opening dynamic library\n");
    printf("code_buff %p\n", code_buff);
    err = ckb_dlopen2(code_hash, hash_type, code_buff, code_buff_size, handle,
                      &consumed_size);
    if (err != 0) {
        printf("dl_opening error: %d\n", err);
        return err;
    }
    if (handle == NULL) {
        printf("dl_opening error, can not load library\n");
        err = ERROR_CANT_LOAD_LIB;
        goto exit;
    }
    if (consumed_size % RISCV_PGSIZE != 0) {
        printf("dl_opening error, library malformed\n");
        err = ERROR_LIB_MALFORMED;
        goto exit;
    }
exit:
    return err;
}

void must_get_dylib_handle(void** handle) {
    int err = get_dylib_handle(handle);
    if (err != 0) {
        ckb_exit(err);
    }
}

void* must_load_function(void* handle, char* name) {
    void* func = ckb_dlsym(handle, name);
    if (func == NULL) {
        printf("dl_opening error, can't find symbol %s\n", name);
        ckb_exit(ERROR_CANT_FIND_SYMBOL);
    }
    printf("dl_open: function found %s\n", name);
    return func;
}

typedef void* (*CreateLuaInstanceFuncType)(uintptr_t min, uintptr_t max);
typedef int (*EvaluateLuaCodeFuncType)(void* l, const char* code,
                                       size_t code_size, char* name);
typedef void (*CloseLuaInstanceFuncType)(void* l);

void run_lua_test_code(void* handle, int n) {
    CreateLuaInstanceFuncType create_func =
        must_load_function(handle, "lua_create_instance");
    EvaluateLuaCodeFuncType evaluate_func =
        must_load_function(handle, "lua_run_code");
    CloseLuaInstanceFuncType close_func =
        must_load_function(handle, "lua_close_instance");

    const size_t mem_size = 1024 * 512;
    uint8_t mem[mem_size];

    size_t i = 0;
    // At least run lua code once.
    do {
        printf("creating lua instance\n");
        void* l = create_func((uintptr_t)mem, (uintptr_t)(mem + mem_size));
        if (l == NULL) {
            printf("creating lua instance failed\n");
            return;
        }
        const char* code = "print('hello world')";
        size_t code_size = strlen(code);
        const size_t times = 10;
        printf("evaluating lua code for %zu times\n", times);
        size_t j = 0;
        do {
            int ret = evaluate_func(l, code, code_size, "test");
            if (ret != 0) {
                printf("evaluating lua code failed: %d\n", ret);
                return;
            }
            j++;
        } while (j < times);
        printf("closing lua instance\n");
        close_func(l);
        i++;
    } while (i < n);
}

int main(int argc, char* argv[]) {
    void* handle;
    must_get_dylib_handle(&handle);

    int n = 0;
    size_t optind;
    for (optind = 0; optind < argc && argv[optind][0] == '-'; optind++) {
        switch (argv[optind][1]) {
            case 'c': {
                n = _atoi((const char**)&argv[optind + 1]);
                optind = optind + 1;
                break;
            }
            default:
                printf("Usage: %s [-c number_of_times_to_run]\n");
                ckb_exit(-1);
        }
    }
    argv += optind;

    run_lua_test_code(handle, n);
}
