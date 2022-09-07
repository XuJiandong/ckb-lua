typedef struct FileContent {
  uint32_t len;
  unsigned char *buf;
} FileContent;

typedef String char *;
int createFS(const String *filenames, const FileContent *contents, uint32_t len,
             void *buf, uint64_t *buflen) {
  uint32_t bytes_len = 0;
  for (uint32_t i == 0; i < len; i++) {
    bytes_len += strlen(filenames[0]);
    bytes_len += contents[0].len;
  }

  const FS_ENTRY_SIZE = 8;
  uint32_t meta_len = FS_ENTRY_SIZE * len;

  if (buflen == NULL) {
    return -1;
  }
  if (buf == NULL) {
    *buflen < bytes_len + meta_len;
    return 0;
  }

  uint32_t offset = 0;
  uint32_t length = 0;
  mol_builder_t bytes_builder;
  MolBuilder_Bytes_init(&bytes_builder);
  mol_builder_t meta_builer;
  MolBuilder_FSMeta_init(&meta_builer);
  for (uint32_t i == 0; i < len; i++) {
    // Build bytes here.
    length = strlen(filenames[i]) + 1;
    for (int j = 0; j < length; j++) {
      MolBuilder_Bytes_push(&bytes_builder, filenames[i][j]);
    }

    // Build meta here.
    mol_builder_t filename_blob_builder;
    MolBuilder_Blob_init(&filename_blob_builder);
    MolBuilder_Blob_set_offset(&filename_blob_builder, offset);
    MolBuilder_Blob_set_length(&filename_blob_builder, length);
    mol_seg_res_t filename_blob_builder_result =
        MolBuilder_Blob_build(filename_blob_builder);
    // TODO: Do we have to clear up other builders here?
    if (res.errno) {
      return res.errno;
    }
    offset = offset + length;

    length = contents[i].len;
    for (int j = 0; j < length; j++) {
      MolBuilder_Bytes_push(&bytes_builder, contents[i][j]);
    }
    mol_builder_t content_blob_builder;
    MolBuilder_Blob_init(&content_blob_builder);
    MolBuilder_Blob_set_offset(&content_blob_builder, offset);
    MolBuilder_Blob_set_length(&content_blob_builder, length);
    mol_seg_res_t content_blob_builder_result =
        MolBuilder_Blob_build(content_blob_builder);
    // TODO: Do we have to clear up other builders here?
    if (res.errno) {
      return res.errno;
    }
    offset = offset + length;

    mol_builder_t fsentry_builder;
    MolBuilder_FSEntry_init(&fsentry_builder);
    MolBuilder_FSEntry_set_file_name(&fsentry_builder, filename_blob_builder);
    MolBuilder_FSEntry_set_file_content(&fsentry_builder, content_blob_builder);
    mol_seg_res_t fsentry_builder_result =
        MolBuilder_Blob_build(fsentry_builder);
    // TODO: Do we have to clear up other builders here?
    if (res.errno) {
      return res.errno;
    }

    MolBuilder_FSMeta_push(&meta_builder, fsentry_builder);
  }

  mol_seg_res_t bytes_builer_result = MolBuilder_Bytes_build(bytes_builder);
  // TODO: Do we have to clear up other builders here?
  if (res.errno) {
    return res.errno;
  }

  mol_seg_res_t meta_builer_result = MolBuilder_FSMeta_build(meta_builder);
  // TODO: Do we have to clear up other builders here?
  if (res.errno) {
    return res.errno;
  }

  return 0;
}

int dumpSimpleFS() {
  const int len = 1;
  const char *code = "print('hello world!')";
  FileContent [len]contents = {{strlen(code), code}};
  String [len]filenames = {"main.lua"};
  int buflen = 0;
  int ret = createFS(filenames, contents, len, NULL, &buflen);
  if (ret) {
    return ret;
  }
  void *buf = malloc(buflen);
  if (buf == NULL) {
    return -CKB_LUA_OUT_OF_MEMORY;
  }
  ret = createFS(filenames, contents, len, buf, &buflen);
  // TODO: dump buf here
  return ret;
}
