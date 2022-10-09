#ifndef _CKB_CELL_FS_H
#define _CKB_CELL_FS_H 1

typedef struct FSBlob {
    uint32_t offset;
    uint32_t length;
} FSBlob;

typedef struct FSEntry {
    FSBlob filename;
    FSBlob content;
} FSEntry;

typedef struct CellFileSystem {
    uint32_t count;
    FSEntry *files;
    void *start;
} CellFileSystem;

static CellFileSystem CELL_FILE_SYSTEM;

typedef struct FSFile {
    char *filename;
    void *content;
    uint32_t size;
} FSFile;

#endif
