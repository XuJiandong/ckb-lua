#include "../include/ckb_cell_fs.h"

int dochunk(lua_State *L, int status);

int exit(int c);

int load_fs(CellFileSystem *fs, void *buf, uint64_t buflen) {
    if (buf == NULL || buflen < sizeof(fs->count)) {
        return -1;
    }

    fs->count = *(uint32_t *)buf;
    if (fs->count == 0) {
        fs->files = NULL;
        fs->start = NULL;
        return 0;
    }

    fs->files = (FSEntry *)malloc(sizeof(FSEntry) * fs->count);
    if (fs->files == NULL) {
        return -1;
    }
    fs->start = buf + sizeof(fs->count) + (sizeof(FSEntry) * fs->count);

    FSEntry *entries = (FSEntry *)((char *)buf + sizeof(fs->count));
    for (uint32_t i = 0; i < fs->count; i++) {
        FSEntry entry = entries[i];
        fs->files[i] = entry;
    }

    return 0;
}

int get_file(const CellFileSystem *fs, char *filename, FSFile *file) {
    for (uint32_t i = 0; i < fs->count; i++) {
        FSEntry entry = fs->files[i];
        if (strcmp(filename, fs->start + entry.filename.offset) == 0) {
            file->filename = filename;
            file->size = entry.content.length;
            file->content = fs->start + entry.content.offset;
            return 0;
        }
    }
    return 1;
}

FSFile ckb_must_get_file(char *filename) {
    FSFile file;
    if (get_file(&CELL_FILE_SYSTEM, filename, &file) != 0) {
        exit(-1);
    }
    return file;
}

// TODO: write to an IO stream instead of copying to a buffer.
int ckb_save_fs(const FSFile *files, uint32_t count, void *buf,
                uint64_t *buflen) {
    if (buflen == NULL && buf == NULL) {
        return -1;
    }

    uint32_t bytes_len = 0;

    bytes_len += sizeof(uint32_t);
    bytes_len += count * sizeof(FSEntry);

    uint32_t *metadata_start = buf;
    char *data_start = buf + bytes_len;

    for (uint32_t i = 0; i < count; i++) {
        FSFile file = files[i];
        bytes_len += strlen(file.filename);
        bytes_len += file.size;
    }

    if (buflen != NULL && buf != NULL && *buflen < bytes_len) {
        return -1;
    }

    if (buflen != NULL) {
        if (*buflen < bytes_len && buf != NULL) {
            *buflen = bytes_len;
            return -1;
        }
        *buflen = bytes_len;
    }

    if (buf == NULL) {
        return 0;
    }

    metadata_start[0] = count;
    uint32_t offset = 0;
    uint32_t size;
    FSEntry *entries = (FSEntry *)((char *)metadata_start + sizeof(count));
    for (uint32_t i = 0; i < count; i++) {
        FSFile file = files[i];
        FSEntry *entry = entries + i;

        size = strlen(file.filename) + 1;
        entry->filename.offset = offset;
        entry->filename.length = size;
        memcpy(data_start + offset, file.filename, size);
        offset = offset + size;

        size = file.size;
        entry->content.offset = offset;
        entry->content.length = size;
        memcpy(data_start + offset, file.content, size);
        offset = offset + size;
    }
    return 0;
}

int evaluate_file(lua_State *L, const FSFile *file) {
    return dochunk(L, luaL_loadbuffer(L, file->content, file->size,
                                      "=(read file system)"));
}

int test_make_file_system_evaluate_code(lua_State *L, FSFile *files,
                                          uint32_t count) {
    int ret;

    uint64_t buflen = 0;
    ret = ckb_save_fs(files, count, NULL, &buflen);
    if (ret) {
        return ret;
    }

    void *buf = malloc(buflen);
    if (buf == NULL) {
        return -CKB_LUA_OUT_OF_MEMORY;
    }

    ret = ckb_save_fs(files, count, buf, &buflen);
    if (ret) {
        return ret;
    }

    ret = load_fs(&CELL_FILE_SYSTEM, buf, buflen);
    if (ret) {
        return ret;
    }

    FSFile f = ckb_must_get_file("main.lua");
    return evaluate_file(L, &f);
}

int run_sample_code_file_system(lua_State *L) {
    const char *code = "print('hello world!')";
    FSFile file = {"main.lua", (void *)code, (uint64_t)strlen(code)};
    return test_make_file_system_evaluate_code(L, &file, 1);
}

static int run_from_file_system(lua_State *L) {
    int status;
    status = run_sample_code_file_system(L);
    if (status != LUA_OK) {
        return 0;
    }
    return 1;
}
