ERROR_LOAD_SCRIPT = 1
ERROR_INVALID_SCRIPT = 2
ERROR_LOAD_LOCK_HASH = 3
ERROR_LOAD_CELL_DATA = 4
ERROR_INVALID_CELL_DATA = 5
ERROR_OVERFLOWING = 6
ERROR_INVALID_AMOUNT = 7

BLAKE2B_BLOCK_SIZE = 32
AMOUNT_BITS = 128

function sum(array, bits)
  assert(bits % 64 == 0)
  sum = bn.new(bits, 0)
  tmp_number = bn.new(bits, 0)

  for num in array do
    if #num * 8 != bits then
      return nil, -ERROR_INVALID_CELL_DATA
    fi
    tmp_number:load(input)
    sum = sum + tmp_number
    if sum.overflow then
      return nil, -ERROR_OVERFLOWING
    end
  end
  return sum, nil
end

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

  input_sum, err = sum(input_cell_data, AMOUNT_BITS)
  if err != nil then
    return err
  fi

  output_sum, err = sum(output_cell_data, AMOUNT_BITS)
  if err != nil then
    return err
  fi

  if input_sum < output_sum then
    return -ERROR_INVALID_AMOUNT
  end

  return 0
end
