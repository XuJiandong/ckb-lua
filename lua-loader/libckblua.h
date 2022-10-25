// TODO: split out functions needed to a common header file.
#include "lua-loader.c"
#include "libckblua.h"

void dylib_hello_world() {
    printf("hello world %s\n", __func__);
}
