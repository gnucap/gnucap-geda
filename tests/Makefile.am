# this file is part of gnucap-geda
# (c) 2012-13 Felix Salfelder
# license: GPLv3+

MODULE = ../src/.libs/lang_geda.so
GNUCAP = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs; $(shell echo gnucap | sed -e '$(transform)')
GNUCAP_GEDA = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs; ../gnucap-geda
EXTRA_DIST = \
    nets.sch nets.ref spice_foo.sp \
    dut.sch \
    bogus.sch bogus.ref \
    gafrc

TEST_EXTENSIONS = .sh
AM_TESTS_FD_REDIRECT = 9>&2
EG_TESTS = eg1.sh \
           eg15.sh \
           eg2.sh \
           eg25.sh \
           eg26.sh \
           eg3.sh \
           eg34.sh \
           eg4.sh \
           eg42.sh \
           eg43.sh \
           eg5.sh

GC_TESTS = res_net.sh short.sh rail.sh spice.sh \
        testbench.sh res.sh notfound.sh convert.sh \
        verilog.sh verilog_place.sh
CUSTOM_TESTS = nets.sh bogus.sh
TESTS = $(EG_TESTS) \
        $(GC_TESTS) \
        $(CUSTOM_TESTS)
EXTRA_DIST+= $(TESTS:%.sh=%.ref) $(GC_TESTS:%.sh=%.gc)

# these require schematics
SCHEMATICS = testbench.sch spice.sch rail.sch res_net.sch short.sch res.sch
$(SCHEMATICS:%.sch=%.out): %.out: $(PWD)/%.sch
EXTRA_DIST+= $(SCHEMATICS)

# this includes an extra netlist
spice.out: $(PWD)/spice_foo.sp
testbench.out: $(PWD)/dut.sch

$(GC_TESTS) $(EG_TESTS) $(CUSTOM_TESTS): %.sh: %.ref
	@echo "#!/bin/sh" > $@
	@echo "set -e" >> $@
	@echo "exec 2>&9" >> $@
	@echo "diff -wrup $< $*.out || exit 1" >> $@
	@echo "rm $*.out" >> $@
	@chmod +x $@

EXTRA_CMD=

# BUG: srcdir hardwired.
$(EG_TESTS:%.sh=%.gc): %.gc:
	echo $(EXTRA_CMD) > $@
	echo gschem >> $@
	echo include $(srcdir)/$*.sch >> $@
	echo verilog >> $@
	echo list >> $@
	echo end >> $@

# gnucap-uf warns about pinlable collision.
eg34.gc: EXTRA_CMD=options nowarn
eg26.gc: EXTRA_CMD=options nowarn

nets.out: nets.sch
	$(GNUCAP_GEDA) -a $(MODULE) $< |grep ^net | wc -l > $@

$(TESTS:%.sh=%.log): %.log: %.out

bogus.out: $(PWD)/bogus.sch
	$(GNUCAP_GEDA) -a $(MODULE) $< > $@

%.reg: %.test %.ref
	diff $+

$(PWD)/%.sch: $(abs_srcdir)/%.sch
	$(LN_S) $< .

$(PWD)/%.sp: $(abs_srcdir)/%.sp
	$(LN_S) $< .

%.test: %.gc
	$(GNUCAP) -a ../src/.libs/lang_geda.so < $< | grep '^[a-zA-Z0-9]*=' > $@

res_net.out testbench.out: GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v

EXTRA_SED_ARG =
EXTRA_DIFF_ARG =

$(GC_TESTS:%.sh=%.out): %.out: %.gc
	$(GNUCAP) -a $(MODULE) $(GC_INCLUDES) < $< | \
		sed -e '1,/^core-lib version/d' | \
		sed -e '/^default plugins/d' | \
		sed -e '/^parameter /d' $(EXTRA_SED_ARG)| \
		grep -v ^@ > $@

testbench.out: EXTRA_SED_ARG=-e '/^parameter /d'

$(EG_TESTS:%.sh=%.out): %.out: %.gc
	$(GNUCAP) -a $(MODULE) $(GC_INCLUDES) < $< | \
		sed -e '1,/^core-lib version/d' | \
		sed -e '1,/^stashing as/d' | \
		sed -e '/^default plugins/d' $(EXTRA_SED_ARGS) | \
		grep -v ^@ > $@

$(EG_TESTS:%.sh=%.out): EXTRA_SED_ARGS=-e '/^parameter /d'

eg_tests: $(EG_TESTS:%.sh=%.log)
gc_tests: $(GC_TESTS:%.sh=%.log)

.PHONY: $(TESTS:%.sh=%.out)

CLEANFILES = $(TESTS) $(TESTS:%.sh=%.out)
