REG_TEST=res.reg res_net.reg short.reg rail.reg

%.reg: %.test %.ref
	diff $+

%.test: %.gc
	gnucap $< | grep '^[a-zA-Z0-9]*=' > $@

check: $(REG_TEST)

.PHONY: %.reg
