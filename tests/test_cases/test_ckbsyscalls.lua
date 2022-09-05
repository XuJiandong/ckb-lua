local witness, error = ckb.load_witness(0, 0, ckb.source.INPUT)
assert(not error)
assert(witness == "witnessfoobar")

local buf, error = ckb.load_tx_hash(0)
print(error, #buf)
ckb.dump(buf)
assert(not error)
assert(buf == "\xc5\xc7\x16\x62\x93\x96\x45\x56\x4e\xef\xeb\x3e\x10\x94\x6a\xd5\xb7\xd2\x7e\x17\x9d\x69\x97\x30\x1a\x38\xf8\xa7\xae\x29\xbf\x41")
