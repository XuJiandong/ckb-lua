#include "lua-loader.c"

__attribute__((visibility("default"))) void dylib_hello_world() {
    printf("hello world %s\n", __func__);
}
