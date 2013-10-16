# this file is part of gnucap-geda
# (c) 2012-13 Felix Salfelder
# license: GPLv3+

GNUCAP = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs; $(shell echo gnucap | sed -e '$(transform)')
GNUCAP_GEDA = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs; ../gnucap-geda
REG_TEST = res res_net short rail spice testbench
MISC_TEST = bogus
EXTRA_DIST = $(REG_TEST:%=%.ref) \
             $(REG_TEST:%=%.gc) \
             $(REG_TEST:%=%.sch) \
             nets.sch nets.ref spice_foo.sp \
				 dut.sch \
             bogus.sch bogus.ref \
				 gafrc

# where is this module in the generic case?
MODULE = lang_geda.so

nets.test: %.test: %.sch
	$(GNUCAP_GEDA) -a $(MODULE) $< |grep ^net | wc -l > $@

bogus.test: %.test: %.sch
	$(GNUCAP_GEDA) -a $(MODULE) $< | grep -v ^v > $@

%.reg: %.test %.ref
	diff $+

%.test: %.gc
	$(GNUCAP) -a ../src/.libs/lang_geda.so $< | grep '^[a-zA-Z0-9]*=' > $@

check: $(REG_TEST:%=%.reg) nets.reg $(MISC_TEST:%=%.reg)

clean: clean-test

clean-test:
	rm -f $(REG_TEST:%=%.test)

.PHONY: %.reg check
