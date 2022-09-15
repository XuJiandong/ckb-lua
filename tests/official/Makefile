CKB-DEBUGGER ?= ckb-debugger
MAX-CYCLES ?= 2000000000
TEST-FILE ?= 

define run
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) --read-file $(1) --bin ../../build/lua-loader.debug -- lua-loader -r   
endef

define run_ci
	RUST_LOG=debug $(CKB-DEBUGGER) --max-cycles $(MAX-CYCLES) ---read-file $(1) --bin ../../build/lua-loader.debug -- lua-loader -r  2>&1 | fgrep 'Run result: 0'
endef

all:
	$(call run, locals.lua)	
	$(call run, literals.lua)
	$(call run, sort.lua)
	$(call run, strings.lua)
	$(call run, math.lua)
	$(call run, api.lua)
	$(call run, all.lua)
	$(call run, api.lua)
	$(call run, bwcoersion.lua)
	$(call run, calls.lua)
	$(call run, closure.lua)
	$(call run, code.lua)
	$(call run, coroutine.lua)
	$(call run, events.lua)
	$(call run, gengc.lua)
	$(call run, goto.lua)
	$(call run, heavy.lua)
	$(call run, tpack.lua)
	$(call run, tracegc.lua)
	$(call run, utf8.lua)
	$(call run, vararg.lua)
	$(call run, gc.lua)
	$(call run, bitwise.lua)
	$(call run, constructs.lua)
	$(call run, db.lua)

# failed tests: attrib big cstack errors files main nextvar pm verybig
ci:
	$(call run_ci, locals.lua)
	$(call run_ci, literals.lua)
	$(call run_ci, sort.lua)
	$(call run_ci, strings.lua)
	$(call run_ci, math.lua)
	$(call run_ci, all.lua)
	$(call run_ci, api.lua)
	$(call run_ci, bwcoersion.lua)
	$(call run_ci, calls.lua)
	$(call run_ci, closure.lua)
	$(call run_ci, code.lua)
	$(call run_ci, coroutine.lua)
	$(call run_ci, events.lua)
	$(call run_ci, gengc.lua)
	$(call run_ci, goto.lua)
	$(call run_ci, heavy.lua)
	$(call run_ci, tpack.lua)
	$(call run_ci, tracegc.lua)
	$(call run_ci, utf8.lua)
	$(call run_ci, vararg.lua)
	$(call run_ci, gc.lua)
	$(call run_ci, bitwise.lua)
	$(call run_ci, constructs.lua)
	$(call run_ci, db.lua)

test-file:
	$(call run, $(TEST-FILE).lua)	
