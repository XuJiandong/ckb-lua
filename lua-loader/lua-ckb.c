#include "ckb_syscalls.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdlib.h>

typedef const char *string;
typedef int syscall_v2(void *, uint64_t *, size_t);
typedef int syscall_v4(void *, uint64_t *, size_t, size_t, size_t);
typedef int syscall_v5(void *, uint64_t *, size_t, size_t, size_t, size_t);

typedef enum {
  STRING = 1 << 0,
  UINT64 = 1 << 1,
  SIZE_T = 1 << 2,
  INTEGER = 1 << 3,
} FIELD_TYPE;

typedef union {
  string str;
  uint64_t u64;
  size_t size;
  int integer;
} FIELD_ARG;

typedef struct {
  string name;
  FIELD_TYPE type;
  FIELD_ARG arg;
} FIELD;

/////////////////////////////////////////////////////
// Utilities
/////////////////////////////////////////////////////

#define THROW_ERROR(L, s, ...)                                                 \
  char _error[256];                                                            \
  snprintf_(_error, 256, s, __VA_ARGS__);                                      \
  lua_pushstring(L, _error);                                                   \
  lua_error(L);

#define CALL_SYSCALL_PUSH_RESULT(L, f, l, ...)                                 \
  int _ret = 0;                                                                \
  uint8_t *_buf = NULL;                                                        \
  if (l == 0) {                                                                \
    /* just get buffer length */                                               \
    _ret = f(NULL, &l, __VA_ARGS__);                                           \
  } else {                                                                     \
    _buf = malloc(l);                                                          \
  }                                                                            \
  if (_ret == 0) {                                                             \
    if (_buf == NULL) {                                                        \
      /* malloc for an empty buffer */                                         \
      _buf = malloc(l);                                                        \
    }                                                                          \
    _ret = f(_buf, &l, __VA_ARGS__);                                           \
  }                                                                            \
  if (_ret != 0) {                                                             \
    THROW_ERROR(L, "Invalid CKB syscall response: %d", _ret)                   \
  }                                                                            \
  lua_newtable(L);                                                             \
  for (int i = 0; i < l; ++i) {                                                \
    lua_pushinteger(L, _buf[i]);                                               \
    lua_rawseti(L, -2, i + 1);                                                 \
  }                                                                            \
  free(_buf);

#define SET_FIELD(L, v, n)                                                     \
  lua_pushinteger(L, v);                                                       \
  lua_setfield(L, -2, n);

int GET_FIELDS_WITH_CHECK(lua_State *L, FIELD *fields, int count,
                          int minimal_count) {
  int args_count = lua_gettop(L);
  if (lua_gettop(L) < minimal_count) {
    THROW_ERROR(L, "Invalid arguements count: expected %d got %d",
                minimal_count, lua_gettop(L))
  }
  for (int i = 0; i < count; ++i) {
    FIELD *field = &fields[i];
    switch (field->type) {
    case STRING: {
      if (i < minimal_count && lua_isstring(L, i + 1) == 0) {
        THROW_ERROR(L, "Invalid arguement \"%s\" at %d: need a string",
                    field->name, i + 1)
      }
      field->arg.str = lua_tostring(L, i + 1);
    } break;

    case UINT64: {
      if (i < minimal_count && lua_isinteger(L, i + 1) == 0) {
        THROW_ERROR(L, "Invalid arguement \"%s\" at %d: need an integer",
                    field->name, i + 1)
      }
      field->arg.u64 = lua_tointeger(L, i + 1);
    } break;

    case SIZE_T: {
      if (i < minimal_count && lua_isinteger(L, i + 1) == 0) {
        THROW_ERROR(L, "Invalid arguement \"%s\" at %d: need an integer",
                    field->name, i + 1)
      }
      field->arg.size = lua_tointeger(L, i + 1);
    } break;
    case INTEGER: {
      if (i < minimal_count && lua_isinteger(L, i + 1) == 0) {
        THROW_ERROR(L, "Invalid arguement \"%s\" at %d: need an integer",
                    field->name, i + 1)
      }
      field->arg.integer = lua_tointeger(L, i + 1);
    } break;
    }
  }
  return args_count;
}

int CKB_LOAD_V2(lua_State *L, syscall_v2 f) {
  FIELD fields[] = {{"offset", SIZE_T}, {"length?", UINT64}};
  GET_FIELDS_WITH_CHECK(L, fields, 2, 1);
  CALL_SYSCALL_PUSH_RESULT(L, f, fields[1].arg.u64, fields[0].arg.size)
  return 1;
}

int CKB_LOAD_V4(lua_State *L, syscall_v4 f) {
  FIELD fields[] = {{"offset", SIZE_T},
                    {"index", SIZE_T},
                    {"source", SIZE_T},
                    {"length?", UINT64}};
  GET_FIELDS_WITH_CHECK(L, fields, 4, 3);
  CALL_SYSCALL_PUSH_RESULT(L, f, fields[3].arg.u64, fields[0].arg.size,
                           fields[1].arg.size, fields[2].arg.size)
  return 1;
}

int CKB_LOAD_V5(lua_State *L, syscall_v5 f) {
  FIELD fields[] = {{"offset", SIZE_T},
                    {"index", SIZE_T},
                    {"source", SIZE_T},
                    {"field", SIZE_T},
                    {"length?", UINT64}};
  GET_FIELDS_WITH_CHECK(L, fields, 5, 4);
  CALL_SYSCALL_PUSH_RESULT(L, f, fields[4].arg.u64, fields[0].arg.size,
                           fields[1].arg.size, fields[2].arg.size,
                           fields[3].arg.size)
  return 1;
}

int lua_ckb_exit(lua_State *L) {
  FIELD fields[] = {{"code", INTEGER}};
  int code = 0;
  int args_count = GET_FIELDS_WITH_CHECK(L, fields, 1, 0);
  if (args_count == 1) {
    code = fields[0].arg.integer;
  }
  ckb_exit(code);
  return 0;
}

int lua_ckb_debug(lua_State *L) {
  FIELD fields[] = {{"msg", STRING}};
  GET_FIELDS_WITH_CHECK(L, fields, 1, 1);
  ckb_debug(fields[0].arg.str);
  return 0;
}

int lua_ckb_load_tx_hash(lua_State *L) {
  uint64_t len = 0;
  CALL_SYSCALL_PUSH_RESULT(L, ckb_load_tx_hash, len, 0)
  if (len != 32) {
    THROW_ERROR(L, "Invalid CKB hash length: %ld", len)
  }
  return 1;
}

int lua_ckb_load_script_hash(lua_State *L) {
  uint64_t len = 0;
  CALL_SYSCALL_PUSH_RESULT(L, ckb_load_script_hash, len, 0)
  if (len != 32) {
    THROW_ERROR(L, "Invalid CKB hash length: %ld", len)
  }
  return 1;
}

int lua_ckb_load_script(lua_State *L) {
  return CKB_LOAD_V2(L, ckb_load_script);
}

int lua_ckb_load_transaction(lua_State *L) {
  return CKB_LOAD_V2(L, ckb_load_transaction);
}

int lua_ckb_load_cell(lua_State *L) { return CKB_LOAD_V4(L, ckb_load_cell); }

int lua_ckb_load_input(lua_State *L) { return CKB_LOAD_V4(L, ckb_load_input); }

int lua_ckb_load_header(lua_State *L) {
  return CKB_LOAD_V4(L, ckb_load_header);
}

int lua_ckb_load_witness(lua_State *L) {
  return CKB_LOAD_V4(L, ckb_load_witness);
}

int lua_ckb_load_cell_data(lua_State *L) {
  return CKB_LOAD_V4(L, ckb_load_cell_data);
}

int lua_ckb_load_cell_by_field(lua_State *L) {
  return CKB_LOAD_V5(L, ckb_load_cell_by_field);
}

int lua_ckb_load_input_by_field(lua_State *L) {
  return CKB_LOAD_V5(L, ckb_load_input_by_field);
}

static const luaL_Reg ckb_syscall[] = {
    {"exit", lua_ckb_exit},
    {"debug", lua_ckb_debug},
    {"load_tx_hash", lua_ckb_load_tx_hash},
    {"load_script_hash", lua_ckb_load_script_hash},
    {"load_script", lua_ckb_load_script},
    {"load_transaction", lua_ckb_load_transaction},

    {"load_cell", lua_ckb_load_cell},
    {"load_input", lua_ckb_load_input},
    {"load_header", lua_ckb_load_header},
    {"load_witness", lua_ckb_load_witness},
    {"load_cell_data", lua_ckb_load_cell_data},
    {"load_cell_by_field", lua_ckb_load_cell_by_field},
    {"load_input_by_field", lua_ckb_load_input_by_field},
    {NULL, NULL}};

LUAMOD_API int luaopen_ckb(lua_State *L) {
  // save current stack top
  int stack_top = lua_gettop(L);

  // create ckb table
  luaL_newlib(L, ckb_syscall);

  // create ckb.code table
  lua_newtable(L);
  SET_FIELD(L, CKB_SUCCESS, "SUCCESS")
  SET_FIELD(L, -CKB_INDEX_OUT_OF_BOUND, "INDEX_OUT_OF_BOUND")
  SET_FIELD(L, -CKB_ITEM_MISSING, "ITEM_MISSING")
  lua_setfield(L, stack_top + 1, "code");

  // create ckb.source table
  lua_newtable(L);
  SET_FIELD(L, CKB_SOURCE_INPUT, "INPUT")
  SET_FIELD(L, CKB_SOURCE_OUTPUT, "OUTPUT")
  SET_FIELD(L, CKB_SOURCE_CELL_DEP, "CELL_DEP")
  SET_FIELD(L, CKB_SOURCE_HEADER_DEP, "HEADER_DEP")
  SET_FIELD(L, CKB_SOURCE_GROUP_INPUT, "GROUP_INPUT")
  SET_FIELD(L, CKB_SOURCE_GROUP_OUTPUT, "GROUP_OUTPUT")
  lua_setfield(L, stack_top + 1, "source");

  // create ckb.cell table
  lua_newtable(L);
  SET_FIELD(L, CKB_CELL_FIELD_CAPACITY, "CAPACITY")
  SET_FIELD(L, CKB_CELL_FIELD_DATA_HASH, "DATA_HASH")
  SET_FIELD(L, CKB_CELL_FIELD_LOCK, "LOCK")
  SET_FIELD(L, CKB_CELL_FIELD_LOCK_HASH, "LOCK_HASH")
  SET_FIELD(L, CKB_CELL_FIELD_TYPE, "TYPE")
  SET_FIELD(L, CKB_CELL_FIELD_TYPE_HASH, "TYPE_HASH")
  SET_FIELD(L, CKB_CELL_FIELD_OCCUPIED_CAPACITY, "OCCUPIED_CAPACITY")
  lua_setfield(L, stack_top + 1, "cell");

  // create ckb.input table
  lua_newtable(L);
  SET_FIELD(L, CKB_INPUT_FIELD_OUT_POINT, "OUT_POINT")
  SET_FIELD(L, CKB_INPUT_FIELD_SINCE, "SINCE")
  lua_setfield(L, stack_top + 1, "input");

  // move ckb table to global
  lua_setglobal(L, "ckb");
  return 1;
}
