#include <stdlib.h>
#include <string.h>

#include "ckb_cell_fs.h"

int get_file(const CellFileSystem *fs, const char *filename, FSFile **f) {
    FSFile *file = malloc(sizeof(FSFile));
    if (file ==0) {
      return -1;
    }
    for (uint32_t i = 0; i < fs->count; i++) {
        FSEntry entry = fs->files[i];
        if (strcmp(filename, fs->start + entry.filename.offset) == 0) {
            file->filename = filename;
            file->size = entry.content.length;
            file->content = fs->start + entry.content.offset;
            file->rc = 1;
            *f = file;
            return 0;
        }
    }
    return -1;
}

int ckb_get_file(const char *filename, FSFile **file) {
    return get_file(&CELL_FILE_SYSTEM, filename, file);
}
