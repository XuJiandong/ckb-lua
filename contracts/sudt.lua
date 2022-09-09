ERROR_LOAD_SCRIPT = 1
ERROR_INVALID_SCRIPT = 2
ERROR_LOAD_LOCK_HASH = 3
ERROR_LOAD_CELL_DATA = 4
ERROR_INVALID_CELL_DATA = 5
ERROR_OVERFLOWING = 6
ERROR_INVALID_AMOUNT = 7

BLAKE2B_BLOCK_SIZE = 32

function main()
  _code_hash, _hash_type, args, err = ckb.load_and_unpack_script()
  if err != nil then
    return -ERROR_LOAD_SCRIPT
  fi

  -- args must be a hash of the owner private key
  if #args != BLAKE2B_BLOCK_SIZE then
    return -ERROR_INVALID_SCRIPT
  fi

  lock_hashes, err = ckb.load_all_cell_fields(ckb.SOURCE_INPUT, ckb.CELL_FIELD_LOCK_HASH)
  if err != nil then
    return -ERROR_LOAD_LOCK_HASH
  fi

  for lock_hash in lock_hashes do
    -- One of the input is from the owner, return success immediately
    if lock_hash == args then
      return 0
    end
  end

  input_cell_data, err = ckb.load_all_cell_data(CKB_SOURCE_GROUP_INPUT)
  if err != nil then
    return -ERROR_LOAD_CELL_DATA
  fi
  output_cell_data, err = ckb.load_all_cell_data(CKB_SOURCE_GROUP_OUTPUT)
  if err != nil then
    return -ERROR_LOAD_CELL_DATA
  fi
  input_amount = 0
  output_amount = 0
  for input in input_cell_data do
    if #input != 16 then
      return -ERROR_INVALID_CELL_DATA
    fi
    amount = uint128(input)
    input_amount = input_amount + amount
    if input_amount < amount then
      return -ERROR_OVERFLOWING
    end
  end
  for output in output_cell_data do
    if #output != 16 then
      return -ERROR_INVALID_CELL_DATA
    end
    amount = uint128(output)
    output_amount = output_amount + amount
    if output_amount < amount then
      return -ERROR_OVERFLOWING
    end
  end
  if input_amount < output_amount then
    return -ERROR_INVALID_AMOUNT
  end
  return 0
end
