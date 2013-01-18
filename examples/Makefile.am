REG_TEST=res res_net short rail spice testbench
EXTRA_DIST = $(REG_TEST:%=%.ref) \
             $(REG_TEST:%=%.gc) \
             $(REG_TEST:%=%.sch) \
             nets.sch nets.ref spice_foo.sp \
				 dut.sch \
				 gafrc

nets.test: nets.sch
	../gnucap-geda nets.sch  |grep ^net | wc -l > $@

%.reg: %.test %.ref
	diff $+

%.test: %.gc
	GUILE_WARN_DEPRECATED=no \
	gnucap -a ../src/.libs/lang_geda.so $< | grep '^[a-zA-Z0-9]*=' > $@

check: $(REG_TEST:%=%.reg) nets.reg

clean: clean-test

clean-test:
	rm -f $(REG_TEST:%=%.test)

.PHONY: %.reg check
