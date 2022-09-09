-- tool functions
function read(v)
end

function Script(v)
    return v
end

function Cell(v)
    return v
end

-- convert this lua file to json file used by ckb-debugger
tx = {}

tx.cell_deps = {
    name1 = Cell {data = read("./build/risc-v-binary")},
    -- full name
    name2 = Cell {
        capacity = 0x4b9f96b00,
        lock = Script {
            args = "0x",
            code_hash = "0x0000000000000000000000000000000000000000000000000000000000000000",
            hash_type = "data"
        },
        data = "0x616263"
    }
}

tx.inputs = {
    -- "main" code_alias is a special one. It will be replace by binary specified from ckb-debugger command "--bin" option.
    [1] = Cell {
        capacity = 1000000000,
        lock = Script {code_alias = "main", args = "0x00"}
    },
    -- "name1" is defined in cell_deps. The hash_type, outpoint are generated automatically.
    [2] = Cell {
        capacity = 200000000,
        lock = Script {code_alias = "name1", args = "0x01"}
    },
    -- a full example of input cell
    [3] = Cell {
        capacity = 300000,
        lock = Script {
            code_hash = "0x01...2030F0",
            hash_type = "type",
            args = "0x00...000000"
        },
        type = Script {
            code_hash = "0x01...2030F0",
            hash_type = "data1",
            args = "0x00...000000"
        },
        -- use an array as data
        data = {0x11, 0x22, 0x33}
    }
}

tx.outputs = {
    [1] = Cell {
        -- everything can be omitted if not used.
        -- capacity = 100
        -- output lock script, omited
        -- type, optional
    }
}

tx.witnesses = {[1] = {lock = "0x", input_type = "0x", output_type = "0x"}}

return tx
