ERROR_LOAD_SCRIPT = 1
ERROR_INVALID_SCRIPT = 2
ERROR_LOAD_LOCK_HASH = 3
ERROR_LOAD_CELL_DATA = 4
ERROR_INVALID_CELL_DATA = 5
ERROR_OVERFLOWING = 6
ERROR_INVALID_AMOUNT = 7

BLAKE2B_BLOCK_SIZE = 32
AMOUNT_BITS = 128

local bn = {}

local bn_mt = {}
local MAX_INTEGER = 1 << 32

local load = nil
local save = nil
local tostring = nil

local function bind_methods(t)
    assert(#t > 0)
    t.load = load
    return setmetatable(t, bn_mt)
end

bn_mt.__tostring = function(self)
    local res = {}
    for k, v in ipairs(self) do table.insert(res, string.format("%d", v)) end
    return "[" .. table.concat(res, ",") .. "]"
end

bn_mt.__add = function(a, b)
    assert(#a == #b)
    local carry = 0
    local res = {}
    res.overflow = false
    for i = 1, #a do
        local temp = a[i] + b[i] + carry
        if temp >= MAX_INTEGER then
            temp = temp - MAX_INTEGER
            carry = 1
        else
            carry = 0
        end
        res[i] = temp
    end
    if carry > 0 then res.overflow = true end
    return bind_methods(res)
end

bn_mt.__eq = function(a, b)
    assert(#a == #b)
    for i = 1, #a do if a[i] ~= b[i] then return false end end
    return true
end

bn_mt.__lt = function(a, b)
    assert(#a == #b)
    for i = #a, 1, -1 do
        if a[i] < b[i] then
            return true
        elseif a[i] > b[i] then
            return false
        end
    end
    return false
end

bn_mt.__le = function(a, b)
    assert(#a == #b)
    for i = #a, 1, -1 do
        if a[i] < b[i] then
            return true
        elseif a[i] > b[i] then
            return false
        end
    end
    return true
end

bn.new = function(bits, u64_value)
    assert(bits % 64 == 0)
    local limbs_count = bits // 32
    local res = {}
    for _ = 1, limbs_count, 1 do table.insert(res, 0) end
    res[1] = u64_value % MAX_INTEGER
    res[2] = u64_value // MAX_INTEGER
    return bind_methods(res)
end

load = function(self, raw)
    assert(raw:len() > 0)
    assert(raw:len() % 4 == 0)
    local index = 1
    local fmt = "<I4"
    for i = 1, raw:len(), 4 do
        local num = fmt:unpack(raw, i)
        self[index] = num
        index = index + 1
    end
end

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
