local witness, error = ckb.load_witness(0, 0, ckb.source.INPUT)
assert(not error)
assert(witness == "witnessfoobar")
