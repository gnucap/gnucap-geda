# this file is part of gnucap-geda
# (c) 2012-13 Felix Salfelder
# license: GPLv3+

MODULE = ../src/.libs/lang_geda.so
GNUCAP_ENV = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs;
GNUCAP = $(shell echo gnucap | sed -e '$(transform)')
GNUCAP_GEDA = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs; ../gnucap-geda
EXTRA_DIST = \
    nets.sch nets.ref spice_foo.sp \
    dut.sch \
    bogus.sch bogus.ref \
	 gafrc \
	 $(EG_TESTS:%.sh=%.sch) \
	 $(EG_TESTS:%.sh=%.ref) \
	 $(ER_TESTS:%.sh=%.ref)
#
#AM_TESTS_ENVIRONMENT = \
#    export LD_LIBRARY_PATH=../src/.libs \
#	 GUILE_WARN_DEPRECATED=no;
#
TEST_EXTENSIONS = .sh
AM_TESTS_FD_REDIRECT = 9>&2
EG_TESTS = eg1.sh \
           eg1a.sh \
           eg15.sh \
           eg2.sh \
           eg2b.sh \
           eg25.sh \
           eg26.sh \
           eg3.sh \
           eg34.sh \
           eg34b.sh \
           eg4.sh \
           eg42.sh \
           eg43.sh \
           eg5.sh
ER_TESTS = eg1_recon.sh \
           eg15_recon.sh \
           eg2_recon.sh \
           eg2b_recon.sh \
           eg25_recon.sh \
           eg26_recon.sh \
           eg3_recon.sh \
           eg34_recon.sh \
           eg4_recon.sh \
           eg42_recon.sh \
           eg43_recon.sh \
           eg5_recon.sh

DISTCLEANFILES = $(EG_TESTS:%.sh=%.gc) $(ER_TESTS:%.sh=%.gc)

XFAIL_TESTS =

# something wrong with lm741 symbol (pinseq?)
# need workaround.
XFAIL_TESTS += eg25_recon.sh

# need to think about pinlabels and pinnumbers
XFAIL_TESTS += eg26_recon.sh

# a Text element starting with a component name confuses find_type_in_string
XFAIL_TESTS += eg34b.sh

GC_TESTS = res_net.sh short.sh rail.sh spice.sh \
        testbench.sh res.sh notfound.sh convert.sh \
        verilog.sh verilog_place.sh
CUSTOM_TESTS = nets.sh bogus.sh
TESTS = $(EG_TESTS) \
        $(ER_TESTS) \
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

$(ER_TESTS) $(GC_TESTS) $(EG_TESTS) $(CUSTOM_TESTS): %.sh: %.ref
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

$(ER_TESTS:%.sh=%.gc): %_recon.gc: %.out
	@echo creating $@
	@echo verilog > $@
	@echo include $(srcdir)/$*.ref >> $@
	@echo gschem >> $@
	@echo list >> $@
	@echo end >> $@

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
VLIB_FILES =

$(EG_TESTS:%.sh=%_recon.out): %_recon.out: %.out
$(EG_TESTS:%.sh=%.log): %.log: %.out

$(ER_TESTS:%.sh=%.log): %.log: %.out

$(GC_TESTS:%.sh=%.out): %.out: %.gc
	$(GNUCAP_ENV) $(GNUCAP) -a $(MODULE) $(GC_INCLUDES) < $< | \
		sed -e '/^default plugins/d' | \
		sed -e '/^parameter /d' $(EXTRA_SED_ARG)| \
		grep -v ^@ > $@

testbench.out: EXTRA_SED_ARG=-e '/^parameter /d'
$(GC_TESTS:%.sh=%.out): EXTRA_SED_ARG=-e '1,/^\/\/v /d'
spice.out: EXTRA_SED_ARG=-e '1,/^core-lib version/d'
verilog.out: EXTRA_SED_ARG=-e '1,/^stashing as/d'
verilog_place.out: EXTRA_SED_ARG=-e '1,/^stashing as/d'
notfound.out: EXTRA_SED_ARG=-e '1,/^core-lib version/d'
bogus.out: EXTRA_SED_ARG=

$(ER_TESTS:%.sh=%.out): GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v
$(EG_TESTS:%.sh=%.out): GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v

eg2b.out eg2b_recon.out: VLIB_FILES+= transistor.v diode.v
eg3.out eg3_recon.out: VLIB_FILES += transistor.v 74.v
eg4_recon.out: VLIB_FILES += transistor.v connector.v
eg5_recon.out: VLIB_FILES = transistor.v connector.v analog.v
eg25_recon.out: VLIB_FILES = 74.v linear.v
eg25.out: VLIB_FILES = 74.v linear.v
eg26_recon.out: VLIB_FILES = connector.v
eg42.out: VLIB_FILES = transistor.v linear.v switcap.v diode.v analog.v
eg42_recon.out: VLIB_FILES = transistor.v linear.v switcap.v diode.v analog.v
eg43_recon.out: VLIB_FILES += asic.v analog.v

# vile hack. maybe should not wrap comments...?
$(ER_TESTS:%.sh=%.out): EXTRA_SED_ARGS=-e '1,/^stashing /d' -e 's%^//v %v %' -e '/^\/\/.*/d'

$(EG_TESTS:%.sh=%.out) $(ER_TESTS:%.sh=%.out): %.out: %.gc
	-@echo "$(GNUCAP) $<"; $(GNUCAP_ENV) $(GNUCAP) -a $(MODULE) $(GC_INCLUDES) \
		$(VLIB_FILES:%=-i $(abs_top_srcdir)/include/%) < $< | \
		sed -e '/^default plugins/d' $(EXTRA_SED_ARGS) | \
		grep -v ^@ > $@

$(EG_TESTS:%.sh=%.out): EXTRA_SED_ARGS= | sed -n '/^\/\/v /,$$p' | sed -e '/^parameter /d'

eg_tests: $(EG_TESTS:%.sh=%.log)
er_tests: $(ER_TESTS:%.sh=%.log)
gc_tests: $(GC_TESTS:%.sh=%.log)

.PHONY: $(TESTS:%.sh=%.out)

CLEANFILES = $(TESTS) $(TESTS:%.sh=%.out)
