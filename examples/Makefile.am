REG_TEST=res res_net short rail nets

nets.test: nets.sch
	gnucap-geda nets.sch  |grep ^net | wc -l > $@

%.reg: %.test %.ref
	diff $+

%.test: %.gc
	GUILE_WARN_DEPRECATED=no \
	gnucap $< | grep '^[a-zA-Z0-9]*=' > $@

check: $(REG_TEST:%=%.reg)

clean: clean-test

clean-test:
	rm -f $(REG_TEST:%=%.test)

.PHONY: %.reg check
