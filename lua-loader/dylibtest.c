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
#define EXPORTED_FUNC_NAME "dylib_hello_world"
#define SCRIPT_SIZE (32 * 1024)

#define RESERVED_ARGS_SIZE 2
#define BLAKE2B_BLOCK_SIZE 32
#define HASH_TYPE_SIZE 1

typedef int (*HelloWorldFuncType)();

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

// functions
int load_validate_func(uint8_t* code_buff, uint32_t* code_buff_size,
                       const uint8_t* hash, uint8_t hash_type,
                       HelloWorldFuncType* func) {
    int err = 0;
    void* handle = NULL;
    size_t consumed_size = 0;

    printf("opening dynamic library\n");
    err = ckb_dlopen2(hash, hash_type, code_buff, *code_buff_size, &handle,
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
    *code_buff_size = consumed_size;

    printf("finding symbol %s\n", EXPORTED_FUNC_NAME);
    *func = (HelloWorldFuncType)ckb_dlsym(handle, EXPORTED_FUNC_NAME);
    if (*func == NULL) {
        printf("dl_opening error, can't find symbol\n");
        err = ERROR_CANT_FIND_SYMBOL;
        goto exit;
    }

    err = 0;
exit:
    return err;
}

int main() {
    unsigned char script[SCRIPT_SIZE];
    uint64_t len = SCRIPT_SIZE;
    printf("loading script\n");
    int err = ckb_load_script(script, &len, 0);
    if (err != 0) {
        printf("loading script error %d\n", err);
        goto exit;
    }
    if (len > SCRIPT_SIZE) {
      err =  ERROR_SCRIPT_TOO_LONG;
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

    // don't move code_buff into global variable. It doesn't work.
    // it's a ckb-vm bug: the global variable will be freezed:
    // https://github.com/nervosnetwork/ckb-vm/blob/d43f58d6bf8cc6210721fdcdb6e5ecba513ade0c/src/machine/elf_adaptor.rs#L28-L32
    // The code can't be loaded into freezed memory.
    uint8_t code_buff[MAX_CODE_SIZE] __attribute__((aligned(RISCV_PGSIZE)));
    uint32_t code_buff_size = MAX_CODE_SIZE;

    HelloWorldFuncType func;
    printf("loading validate function\n");
    err = load_validate_func(code_buff, &code_buff_size, code_hash, hash_type,
                             &func);
    if (err != 0) {
        goto exit;
    }
    printf("running validate function\n");
    int result = func();
    printf("running function result %d\n", result);
exit:
    return err;
}
