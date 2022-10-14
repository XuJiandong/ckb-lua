#define CKB_C_STDLIB_MALLOC 1
#define CKB_C_STDLIB_PRINTF 1

#define lua_c

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lprefix.h"
#include "lua.h"
#include "lualib.h"

#include "lua-ckb.h"
#include "lua-ckb.c"

#include "lua-cell-fs.c"

#include "blockchain.h"
#include "ckb_syscalls.h"

#define LUA_LOADER_ARGS_SIZE 2
#define BLAKE2B_BLOCK_SIZE 32

int exit(int c) {
    ckb_exit(c);
    return 0;
}
void enable_local_access(int b);

void abort() { ckb_exit(-1); }

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME "lua"
#endif

#if !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR "LUA_INIT"
#endif

#define LUA_INITVARVERSION LUA_INIT_VAR LUA_VERSUFFIX

static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;

static void print_usage(const char *badoption) {
    lua_writestringerror("%s: ", progname);
    if (badoption[1] == 'e')
        lua_writestringerror("'%s' needs argument\n", badoption);
    else
        lua_writestringerror("unrecognized option '%s'\n", badoption);
    lua_writestringerror(
        "usage: %s [options] [script [args]]\n"
        "Available options are:\n"
        "  -e stat   execute string 'stat'\n",
        progname);
}

/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message(const char *pname, const char *msg) {
    if (pname) lua_writestringerror("%s: ", pname);
    lua_writestringerror("%s\n", msg);
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int report(lua_State *L, int status) {
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        l_message(progname, msg);
        lua_pop(L, 1); /* remove message */
    }
    return status;
}

/*
** Message handler used to run all chunks
*/
static int msghandler(lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL) { /* is error object not a string? */
        if (luaL_callmeta(L, 1, "__tostring") && /* does it have a metamethod */
            lua_type(L, -1) == LUA_TSTRING)      /* that produces a string? */
            return 1;                            /* that is the message */
        else
            msg = lua_pushfstring(L, "(error object is a %s value)",
                                  luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1); /* append a standard traceback */
    return 1;                     /* return the traceback */
}

/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static int docall(lua_State *L, int narg, int nres) {
    int status;
    int base = lua_gettop(L) - narg;  /* function index */
    lua_pushcfunction(L, msghandler); /* push message handler */
    lua_insert(L, base);              /* put it under function and args */
    globalL = L;                      /* to be available to 'laction' */
    // setsignal(SIGINT, laction);  /* set C-signal handler */
    status = lua_pcall(L, narg, nres, base);
    // setsignal(SIGINT, SIG_DFL); /* reset C-signal handler */
    lua_remove(L, base); /* remove message handler from the stack */
    return status;
}

static void print_version(void) {
    lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
    lua_writeline();
}

/*
** Create the 'arg' table, which stores all arguments from the
** command line ('argv'). It should be aligned so that, at index 0,
** it has 'argv[script]', which is the script name. The arguments
** to the script (everything after 'script') go to positive indices;
** other arguments (before the script name) go to negative indices.
** If there is no script name, assume interpreter's name as base.
*/
static void createargtable(lua_State *L, char **argv, int argc, int script) {
    int i, narg;
    if (script == argc) script = 0; /* no script name? */
    narg = argc - (script + 1);     /* number of positive indices */
    // Our argv does not contain the program name, so argc may be zero
    // narg thus may be less than zero.
    if (narg < 0) {
        narg = 0;
    }

    lua_createtable(L, narg, argc - narg);
    // Push progname to table as our argv does not have a program name.
    lua_pushstring(L, progname);
    lua_rawseti(L, -2, -script);
    for (i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        // We have pushed progname
        lua_rawseti(L, -2, 1 + i - script);
    }
    lua_setglobal(L, "arg");
}

int dochunk(lua_State *L, int status) {
    if (status == LUA_OK) status = docall(L, 0, 0);
    return report(L, status);
}

static int dostring(lua_State *L, const char *s, const char *name) {
    return dochunk(L, luaL_loadbuffer(L, s, strlen(s), name));
}

int load_lua_code_with_hash(lua_State *L, uint16_t lua_loader_args,
                            const uint8_t *code_hash, uint8_t hash_type) {
    size_t index = 0;
    int ret = ckb_look_for_dep_with_hash2(code_hash, hash_type, &index);
    if (ret) {
        return ret;
    }
    char *buf = NULL;
    size_t buflen = 0;
    ret = ckb_load_cell_data(NULL, &buflen, 0, index, CKB_SOURCE_CELL_DEP);
    if (ret) {
        return ret;
    }
    buf = malloc(buflen);
    if (buf == NULL) {
        return LUA_ERROR_OUT_OF_MEMORY;
    }
    ret = ckb_load_cell_data(buf, &buflen, 0, index, CKB_SOURCE_CELL_DEP);
    if (ret) {
        return ret;
    }
    return dochunk(L, luaL_loadbuffer(L, buf, buflen, __func__));
}

#define SCRIPT_SIZE 32768
int load_lua_code_from_cell_data(lua_State *L) {
    unsigned char script[SCRIPT_SIZE];
    uint64_t len = SCRIPT_SIZE;
    int ret = ckb_load_script(script, &len, 0);
    if (ret) {
        return ret;
    }
    if (len > SCRIPT_SIZE) {
        return LUA_ERROR_SCRIPT_TOO_LONG;
    }
    mol_seg_t script_seg;
    script_seg.ptr = (uint8_t *)script;
    script_seg.size = len;

    if (MolReader_Script_verify(&script_seg, false) != MOL_OK) {
        return LUA_ERROR_ENCODING;
    }

    // The script arguments are in the following format
    // <lua loader args, 2 bytes> <code hash of lua code, 32 bytes>
    // <hash type of lua code, 1 byte> <lua script args, variable length>
    // TOOD: parse lua script args
    mol_seg_t args_seg = MolReader_Script_get_args(&script_seg);
    mol_seg_t args_bytes_seg = MolReader_Bytes_raw_bytes(&args_seg);
    if (args_bytes_seg.size < LUA_LOADER_ARGS_SIZE + BLAKE2B_BLOCK_SIZE + 1) {
        return -LUA_ERROR_INVALID_ARGUMENT;
    }
    uint16_t lua_loader_args = *(args_bytes_seg.ptr);
    uint8_t *code_hash = args_bytes_seg.ptr + LUA_LOADER_ARGS_SIZE;
    uint8_t hash_type =
        *(args_bytes_seg.ptr + LUA_LOADER_ARGS_SIZE + BLAKE2B_BLOCK_SIZE);
    return load_lua_code_with_hash(L, lua_loader_args, code_hash, hash_type);
}

/*
** Receives 'globname[=modname]' and runs 'globname = require(modname)'.
*/
static int dolibrary(lua_State *L, char *globname) {
    int status;
    char *modname = strchr(globname, '=');
    if (modname == NULL)    /* no explicit name? */
        modname = globname; /* module name is equal to global name */
    else {
        *modname = '\0'; /* global name ends here */
        modname++;       /* module name starts after the '=' */
    }
    lua_getglobal(L, "require");
    lua_pushstring(L, modname);
    status = docall(L, 1, 1); /* call 'require(modname)' */
    if (status == LUA_OK)
        lua_setglobal(L, globname); /* globname = require(modname) */
    return report(L, status);
}

/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static int pushargs(lua_State *L) {
    int i, n;
    if (lua_getglobal(L, "arg") != LUA_TTABLE)
        luaL_error(L, "'arg' is not a table");
    n = (int)luaL_len(L, -1);
    luaL_checkstack(L, n + 3, "too many arguments to script");
    for (i = 1; i <= n; i++) lua_rawgeti(L, -i, i);
    lua_remove(L, -i); /* remove table from the stack */
    return n;
}

/* bits of various argument indicators in 'args' */
#define has_error 1 /* bad option */
#define has_e 8     /* -e */
#define has_r 4     /* -r */
#define has_f 16    /* -f, for file system test*/
/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code (or an error code if it finds
** any invalid argument). 'first' returns the first not-handled argument
** (either the script name or a bad argument in case of error).
*/
static int collectargs(char **argv, int *first) {
    int args = 0;
    int i;
    for (i = 0; argv[i] != NULL; i++) {
        *first = i;
        if (argv[i][0] != '-') /* not an option? */
            return args;       /* stop handling options */
        switch (argv[i][1]) {  /* else check option */
            case 'e':
                args |= has_e;            /* FALLTHROUGH */
                if (argv[i][2] == '\0') { /* no concatenated argument? */
                    i++;                  /* try next 'argv' */
                    if (argv[i] == NULL || argv[i][0] == '-')
                        return has_error; /* no next argument or it is another
                                             option */
                }
                break;
            case 'r':
                args |= has_r;
                break;
            case 'f':
                args |= has_f;
                break;
            default: /* invalid option */
                return has_error;
        }
    }
    *first = i; /* no script name */
    return args;
}

/*
** Processes options 'e' and 'l', which involve running Lua code, and
** 'W', which also affects the state.
** Returns 0 if some code raises an error.
*/
static int runargs(lua_State *L, char **argv, int n) {
    int i;
    for (i = 1; i < n; i++) {
        int option = argv[i][1];
        lua_assert(argv[i][0] == '-'); /* already checked */
        switch (option) {
            case 'e': {
                int status;
                char *extra = argv[i] + 2; /* both options need an argument */
                if (*extra == '\0') extra = argv[++i];
                lua_assert(extra != NULL);
                status = (option == 'e') ? dostring(L, extra, "=(command line)")
                                         : dolibrary(L, extra);
                if (status != LUA_OK) return 0;
                break;
            }
        }
    }
    return 1;
}

int read_file(char *buf, int size) {
    int ret = syscall(9000, buf, size, 0, 0, 0, 0);
    return ret;
}

static int run_from_file(lua_State *L) {
    enable_local_access(1);
    char buf[1024 * 512];
    int count = read_file(buf, sizeof(buf));
    if (count < 0 || count == sizeof(buf)) {
        printf("error reading from file");
        return 0;
    }
    buf[count] = 0;
    int status = dochunk(L, luaL_loadbuffer(L, buf, count, "=(read file)"));
    if (status != LUA_OK)
        return 0;
    else
        return 1;
}

/*
** {==================================================================
** Read-Eval-Print Loop (REPL)
** ===================================================================
*/
#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT 512
#endif
/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 0) { /* any result to be printed? */
        luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
        lua_getglobal(L, "print");
        lua_insert(L, 1);
        if (lua_pcall(L, n, 0, 0) != LUA_OK)
            l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                                lua_tostring(L, -1)));
    }
}

/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static int pmain(lua_State *L) {
    int argc = (int)lua_tointeger(L, 1);
    char **argv = (char **)lua_touserdata(L, 2);
    int script;
    int args = collectargs(argv, &script);
    luaL_checkversion(L);    /* check that interpreter has correct version */
    if (args == has_error) { /* bad arg? */
        print_usage(argv[script]); /* 'script' has index of bad arg. */
        return 0;
    }
    luaL_openlibs(L); /* open standard libraries */
    luaopen_ckb(L);
    createargtable(L, argv, argc, script); /* create table 'arg' */
    lua_gc(L, LUA_GCGEN, 0, 0);            /* GC in generational mode */
    if (!runargs(L, argv, script))         /* execute arguments -e and -l */
        return 0;                          /* something failed */
    if (args & has_f) {
        if (!run_from_file_system(L)) return 0;
    }
    if (args & has_r) {
        if (!run_from_file(L)) return 0;
    }
    // No arguments found, trying to load lua code from cell data
    // Note we don't push program name to argv, thus we check argc == 0 instead
    // of argc = 1
    if (argc == 0) {
        if (load_lua_code_from_cell_data(L)) return 0;
    }
    lua_pushboolean(L, 1); /* signal no errors */
    return 1;
}

int main(int argc, char **argv) {
    int status, result;
    lua_State *L = luaL_newstate(); /* create state */
    if (L == NULL) {
        l_message(argv[0], "cannot create state: not enough memory");
        return -1;
    }
    lua_pushcfunction(L, &pmain);   /* to call 'pmain' in protected mode */
    lua_pushinteger(L, argc);       /* 1st argument */
    lua_pushlightuserdata(L, argv); /* 2nd argument */
    status = lua_pcall(L, 2, 1, 0); /* do the call */
    result = lua_toboolean(L, -1);  /* get result */
    report(L, status);
    lua_close(L);
    return (result && status == LUA_OK) ? 0 : -1;
}
