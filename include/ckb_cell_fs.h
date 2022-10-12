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

// TODO: use a linked list of CellFileSystem so that we can use files from
// multiple cells
static CellFileSystem CELL_FILE_SYSTEM;

typedef struct FSFile {
    const char *filename;
    const void *content;
    uint32_t size;
    // indicate how many active users there are, used to avoid excessive opening
    // of the same file currently the only valid values are 1 and 0.
    uint8_t rc;
} FSFile;

int get_file(const CellFileSystem *fs, const char *filename, FSFile **f);

int ckb_get_file(const char *filename, FSFile **file);

#endif
