#include "lua-loader.c"

__attribute__((visibility("default"))) int dylib_hello_world() {
    printf("hello world %s\n", __func__);
    return 42;
}
