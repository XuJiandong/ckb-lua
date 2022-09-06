-- The results have been compared with results from https://github.com/contrun/ckb-x64-simulator/tree/dump-simulator-results

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT)
assert(not error)
assert(buf == "witnessfoobar")

local length, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 0)
assert(not error)
assert(length == 13)

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 0, 1)
assert(not error)
assert(buf == 12)

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 0, 14)
assert(not error)
assert(buf == 0)

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 13)
assert(not error)
assert(buf == "witnessfoobar")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 14)
assert(not error)
assert(buf == "witnessfoobar")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 10)
assert(not error)
assert(buf == "witnessfoo")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 13, 0)
assert(not error)
assert(buf == "witnessfoobar")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 10, 0)
assert(not error)
assert(buf == "witnessfoo")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 2, 10)
assert(not error)
assert(buf == "ba")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 3, 10)
assert(not error)
assert(buf == "bar")

local buf, error = ckb.load_witness(0, ckb.SOURCE_INPUT, 4, 10)
assert(not error)
assert(buf == "bar")

local buf, error = ckb.load_tx_hash()
assert(not error)
assert(buf == "\xc5\xc7\x16\x62\x93\x96\x45\x56\x4e\xef\xeb\x3e\x10\x94\x6a\xd5\xb7\xd2\x7e\x17\x9d\x69\x97\x30\x1a\x38\xf8\xa7\xae\x29\xbf\x41")

local buf, error = ckb.load_transaction()
assert(not error)
assert(buf == "\x3e\x01\x00\x00\x0c\x00\x00\x00\x25\x01\x00\x00\x19\x01\x00\x00\x1c\x00\x00\x00\x20\x00\x00\x00\x49\x00\x00\x00\x4d\x00\x00\x00\x7d\x00\x00\x00\x0d\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xfc\xd1\xb3\xdd\xcc\xa9\x2b\x1e\x49\x78\x37\x69\xe9\xbf\x60\x61\x12\xb3\xf8\xcf\x36\xb9\x6c\xac\x05\xbf\x44\xed\xcf\x53\x77\xe6\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa9\x8c\x57\x13\x58\x30\xe1\xb9\x13\x45\x94\x8d\xf6\xc4\xb8\x87\x08\x28\x19\x9a\x78\x6b\x26\xf0\x9f\x7d\xec\x4b\xc2\x7a\x73\xda\x00\x00\x00\x00\x90\x00\x00\x00\x08\x00\x00\x00\x88\x00\x00\x00\x10\x00\x00\x00\x18\x00\x00\x00\x4d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x35\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x3b\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\xfa\x93\x98\x2d\x58\x2a\x0f\x33\x02\xa9\x6a\xc3\x49\x44\xd1\x4b\x41\xd5\x35\x49\xb9\xfb\x2a\xb2\x84\xea\xfc\x1d\x02\x15\x88\xad\x02\x06\x00\x00\x00\x66\x6f\x6f\x62\x61\x72\x0c\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x19\x00\x00\x00\x08\x00\x00\x00\x0d\x00\x00\x00\x77\x69\x74\x6e\x65\x73\x73\x66\x6f\x6f\x62\x61\x72")

local buf, error = ckb.load_script()
assert(not error)
assert(buf == "\x3b\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\xfa\x93\x98\x2d\x58\x2a\x0f\x33\x02\xa9\x6a\xc3\x49\x44\xd1\x4b\x41\xd5\x35\x49\xb9\xfb\x2a\xb2\x84\xea\xfc\x1d\x02\x15\x88\xad\x02\x06\x00\x00\x00\x66\x6f\x6f\x62\x61\x72")

local buf, error = ckb.load_script_hash()
assert(not error)
assert(buf == "\xca\x50\x5b\xee\x92\xc3\x4a\xc4\x52\x2d\x15\xda\x2c\x91\xf0\xe4\x06\x0e\x45\x40\xf9\x0a\x28\xd7\x20\x2d\xf8\xfe\x8c\xe9\x30\xba")

local buf, error = ckb.load_cell(0, ckb.SOURCE_INPUT)
assert(not error)
assert(buf == "\x4d\x00\x00\x00\x10\x00\x00\x00\x18\x00\x00\x00\x4d\x00\x00\x00\x00\x6b\xf9\xb9\x04\x00\x00\x00\x35\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00")

local buf, error = ckb.load_cell(0, ckb.SOURCE_OUTPUT)
assert(not error)
assert(buf == "\x88\x00\x00\x00\x10\x00\x00\x00\x18\x00\x00\x00\x4d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x35\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x3b\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\xfa\x93\x98\x2d\x58\x2a\x0f\x33\x02\xa9\x6a\xc3\x49\x44\xd1\x4b\x41\xd5\x35\x49\xb9\xfb\x2a\xb2\x84\xea\xfc\x1d\x02\x15\x88\xad\x02\x06\x00\x00\x00\x66\x6f\x6f\x62\x61\x72")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_CAPACITY)
assert(not error)
assert(buf == "\x00\x6b\xf9\xb9\x04\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_DATA_HASH)
assert(not error)
assert(buf == "\x52\x1c\x60\x4c\xc0\x9b\x81\x4b\x0a\x91\x06\x30\x53\x95\xde\xf3\x5d\x02\x11\xb9\x99\x6a\x3e\x0f\x32\x6a\xe4\xd6\x71\xbd\x8f\xc2")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_LOCK)
assert(not error)
assert(buf == "\x35\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_LOCK_HASH)
assert(not error)
assert(buf == "\x8f\x59\xe3\x40\xcf\xbe\xa0\x88\x72\x02\x65\xce\xf0\xfd\x9a\xfa\x4e\x42\x0b\xf2\x7c\x7b\x3d\xc8\xae\xbf\x6c\x6e\xda\x45\x3e\x57")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_TYPE)
assert(error)

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_TYPE_HASH)
assert(error)

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_INPUT, ckb.CELL_FIELD_OCCUPIED_CAPACITY)
assert(not error)
assert(buf == "\x00\xac\x42\x06\x01\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_CAPACITY)
print(error, #buf)
ckb.dump(buf)
assert(not error)
assert(buf == "\x00\x00\x00\x00\x00\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_DATA_HASH)
assert(not error)
assert(buf == "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_LOCK)
assert(not error)
assert(buf == "\x35\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_LOCK)
assert(not error)
assert(buf == "\x35\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_LOCK_HASH)
assert(not error)
assert(buf == "\x8f\x59\xe3\x40\xcf\xbe\xa0\x88\x72\x02\x65\xce\xf0\xfd\x9a\xfa\x4e\x42\x0b\xf2\x7c\x7b\x3d\xc8\xae\xbf\x6c\x6e\xda\x45\x3e\x57")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_TYPE)
assert(not error)
assert(buf == "\x3b\x00\x00\x00\x10\x00\x00\x00\x30\x00\x00\x00\x31\x00\x00\x00\xfa\x93\x98\x2d\x58\x2a\x0f\x33\x02\xa9\x6a\xc3\x49\x44\xd1\x4b\x41\xd5\x35\x49\xb9\xfb\x2a\xb2\x84\xea\xfc\x1d\x02\x15\x88\xad\x02\x06\x00\x00\x00\x66\x6f\x6f\x62\x61\x72")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_TYPE_HASH)
assert(not error)
assert(buf == "\xca\x50\x5b\xee\x92\xc3\x4a\xc4\x52\x2d\x15\xda\x2c\x91\xf0\xe4\x06\x0e\x45\x40\xf9\x0a\x28\xd7\x20\x2d\xf8\xfe\x8c\xe9\x30\xba")

local buf, error = ckb.load_cell_by_field(0, ckb.SOURCE_OUTPUT, ckb.CELL_FIELD_OCCUPIED_CAPACITY)
assert(not error)
assert(buf == "\x00\x50\xd6\xdc\x01\x00\x00\x00")

local buf, error = ckb.load_cell_data(0, ckb.SOURCE_INPUT)
assert(not error)
assert(buf == "\x61\x62\x63")

local buf, error = ckb.load_cell_data(0, ckb.SOURCE_OUTPUT)
assert(not error)
assert(buf == "")

local buf, error = ckb.load_input(0, ckb.SOURCE_INPUT)
assert(not error)
assert(buf == "\x00\x00\x00\x00\x00\x00\x00\x00\xa9\x8c\x57\x13\x58\x30\xe1\xb9\x13\x45\x94\x8d\xf6\xc4\xb8\x87\x08\x28\x19\x9a\x78\x6b\x26\xf0\x9f\x7d\xec\x4b\xc2\x7a\x73\xda\x00\x00\x00\x00")

local buf, error = ckb.load_input_by_field(0, ckb.SOURCE_INPUT, ckb.INPUT_FIELD_OUT_POINT)
assert(not error)
assert(buf == "\xa9\x8c\x57\x13\x58\x30\xe1\xb9\x13\x45\x94\x8d\xf6\xc4\xb8\x87\x08\x28\x19\x9a\x78\x6b\x26\xf0\x9f\x7d\xec\x4b\xc2\x7a\x73\xda\x00\x00\x00\x00")

local buf, error = ckb.load_input_by_field(0, ckb.SOURCE_INPUT, ckb.INPUT_FIELD_SINCE)
print(error, #buf)
ckb.dump(buf)
assert(not error)
assert(buf == "\x00\x00\x00\x00\x00\x00\x00\x00")

local sources = {ckb.SOURCE_INPUT, ckb.SOURCE_GROUP_INPUT, ckb.SOURCE_CELL_DEP, ckb.SOURCE_HEADER_DEP}
local header_fields = {ckb.HEADER_FIELD_EPOCH_NUMBER, ckb.HEADER_FIELD_EPOCH_START_BLOCK_NUMBER, ckb.HEADER_FIELD_EPOCH_LENGTH}

for _, source in pairs(sources) do
  local buf, error = ckb.load_header(0, source)
  assert(error)
end

for _, source in pairs(sources) do
  for _, field in pairs(header_fields) do
    local buf, error = ckb.load_header_by_field(0, source, field)
    assert(error)
  end
end
