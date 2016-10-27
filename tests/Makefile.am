# this file is part of gnucap-geda
# (c) 2012-13 Felix Salfelder
# license: GPLv3+

MODULE = ../src/.libs/lang_geda.so
GNUCAP_ENV = export GUILE_WARN_DEPRECATED=no LD_LIBRARY_PATH=../src/.libs$(LD_LIBRARY_PATH:%=:%);
GNUCAP = $(shell echo gnucap | sed -e '$(transform)')
GNUCAP_GEDA = ../gnucap-geda

AM_TESTS_ENVIRONMENT = \
    export PATH='../src:$(PATH)'\
           GNUCAP='$(GNUCAP)'\
           REDIRECT='exec 2>&9'\
           MAKEFLAGS=--no-print-directory \
           srcdir='$(srcdir)';
#	 GUILE_WARN_DEPRECATED=no;

TEST_EXTENSIONS = .sh .sch .gc
GC_LOG_COMPILER = $(top_srcdir)/tests/gc_log_compiler
SCH_LOG_COMPILER = $(top_srcdir)/tests/sch_log_compiler
AM_TESTS_FD_REDIRECT = 9>&2
EG_TESTS = \
	comment.1.sch \
	connect.1.sch \
	connect.2.sch \
	connect.3.sch \
	connect.4.sch \
	connect.5.sch \
	connect.6.sch \
	connect.7.sch \
	D_Latch.sch \
	eg1.sch \
	eg1a.sch \
	eg15.sch \
	eg2.sch \
	eg2b.sch \
	eg25.sch \
	eg26.sch \
	eg3.sch \
	eg34.sch \
	eg34b.sch \
	eg4.sch \
	eg42.sch \
	eg43.sch \
	eg5.sch

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

DISTCLEANFILES = $(EG_TESTS:%.sch=%.gc) $(ER_TESTS:%.sh=%.gc)

XFAIL_TESTS =

# empty line in comment, breaks counter.
XFAIL_TESTS += comment.1.sch

# ground in submodules causes trouble
XFAIL_TESTS += auto_top.1g.gc

# promiscuous incomplete
XFAIL_TESTS += auto_top.4.gc

# something wrong with lm741 symbol (pinseq?)
# need workaround.
XFAIL_TESTS += eg25_recon.gc

# need to think about pinlabels and pinnumbers
XFAIL_TESTS += eg26_recon.gc

# a Text element starting with a component name confuses find_type_in_string
XFAIL_TESTS += eg34b.sch

GC_TESTS = \
	$(SCHEMATICS:%.sch=%.gc) \
	convert.gc \
	dlatch_tr.gc \
	inv_tr.gc \
	divide_by_two.gc \
	invtb.gc \
	d_gedasckt.1.gc \
	error.1.gc \
	notfound.gc \
	verilog.gc \
	verilog_place.gc

CUSTOM_TESTS = nets.sh bogus.sh
TESTS = $(EG_TESTS) \
        $(ER_TESTS) \
        $(GC_TESTS) \
        $(CUSTOM_TESTS)

# each of these induce a test.
SCHEMATICS = \
	auto_top.1.sch \
	auto_top.1s.sch \
	auto_top.1g.sch \
	auto_top.1ge.sch \
	auto_top.2.sch \
	auto_top.3.sch \
	auto_top.3a.sch \
	auto_top.3b.sch \
	auto_top.4.sch \
	buf.1.sch \
	buf.2.sch \
	d_gedasckt.2.sch \
	d_gedasckt.3.sch \
	d_gedasckt.4.sch \
	d_gedasckt.5.sch \
	d_gedasckt.6.sch \
	d_gedasckt.7.sch \
	nesting.sch \
	spice.sch \
	rail.sch \
	short.sch \
	res_net.sch \
	rc.2.sch \
	rc.sch \
	res.sch \
	testbench.sch

$(SCHEMATICS:%.sch=%.out): %.out: $(PWD)/%.sch

# FIXME: these include extra files, find them where they are.
inv_tr.out: $(PWD)/CMOS_Inverter.sch
nesting.out: $(PWD)/nesting.sch
spice.out: $(PWD)/spice_foo.sp
testbench.out: $(PWD)/dut.sch
dlatch_tr.out: $(PWD)/D_Latch.sch
divide_by_two.out inv_tr.out: $(PWD)/CMOS_Inverter.sch

auto_top.1.out: $(PWD)/auto_dut.1.sch
auto_top.1.out: $(PWD)/auto_top.1.sch

auto_dut.1s.out: $(PWD)/auto_dut.1s.sch
auto_top.1s.out: $(PWD)/auto_top.1s.sch

auto_top.2.out: $(PWD)/auto_dut.2.sch
auto_top.3.out: $(PWD)/auto_dut.3.sch
auto_top.3a.out: $(PWD)/auto_dut.3a.sch
auto_top.3b.out: $(PWD)/auto_dut.3b.sch
auto_top.4.out: $(PWD)/auto_dut.3.sch

$(ER_TESTS) $(CUSTOM_TESTS): %.sh: %.ref
	@echo "#!/bin/sh" > $@
	@echo "set -e" >> $@
	@echo "exec 2>&9" >> $@
	@echo "diff -wrup $< $*.out || exit 1" >> $@
	@echo "rm $*.out" >> $@
	@chmod +x $@

EXTRA_CMD=

# BUG: srcdir hardwired.
$(EG_TESTS:%=%.out): %.sch.out: %.sch
	echo -e $(EXTRA_CMD)\\n \
	gschem\\n \
	include $(srcdir)/$*.sch\\n \
	verilog\\n \
	list\\n \
	end | $(GNUCAP) -a ../src/.libs/lang_geda.so \
	                -i $(top_srcdir)/include/analog.v \
	                $(VLIB_FILES:%=-i $(abs_top_srcdir)/include/%) | \
	   sed -e '/^default plugins/d' -e '/^lang_geda/d'  $(EXTRA_SED_ARG) | \
	   sed -e '/^parameter /d' $(EXTRA_SED_ARG) | \
	   sed -e 's/ $$//' $(EXTRA_SED_ARGS) > $@

#sed -e '1,/^\/\/v /d' |
#sed -e '/^\/\/v /d' |

$(ER_TESTS:%.sh=%.gc): %_recon.gc: %.out
	@echo creating $@
	@echo verilog > $@
	@echo include $(srcdir)/$*.ref >> $@
	@echo gschem >> $@
	@echo list >> $@
	@echo end >> $@

# gnucap-uf warns about pinlable collision.
eg34.sch.out: EXTRA_CMD=options nowarn
eg26.sch.out: EXTRA_CMD=options nowarn

nets.sch.out: .P
nets.sch.out: nets.sch
	$(GNUCAP_ENV) $(GNUCAP_GEDA) -a $(MODULE) $< |grep ^net | wc -l > $@

$(ER_TESTS:%.sh=%.log): %.log: %.out
# $(EG_TESTS:%.sch=%.log): %.log: %.sch.out
$(CUSTOM_TESTS:%.sh=%.log): %.log: %.out

bogus.sch.out: .P
bogus.sch.out: $(PWD)/bogus.sch
	$(GNUCAP_ENV) $(GNUCAP_GEDA) -a $(MODULE) $< > $@

%.reg: %.test %.ref
	diff $+

$(PWD)/%.sch: $(abs_srcdir)/%.sch
	$(LN_S) $< .

$(PWD)/%.sp: $(abs_srcdir)/%.sp
	$(LN_S) $< .

%.test: %.gc
	$(GNUCAP) -a ../src/.libs/lang_geda.so < $< | grep '^[a-zA-Z0-9]*=' > $@

res_net.out testbench.out: GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v
convert.out: $(PWD)/res_net.sch

auto_top.1.out: GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v

EXTRA_SED_ARG =
EXTRA_DIFF_ARG =
VLIB_FILES =

$(EG_TESTS:%.sch=%_recon.out): %_recon.out: %.out
# $(EG_TESTS:%.sch=%.log): %.log: %.out

$(ER_TESTS:%.sh=%.log): %.log: %.out

$(GC_TESTS:%.gc=%.out): %.out: %.gc
	echo gctest...
	$(GNUCAP_ENV) $(GNUCAP) -a $(MODULE) $(GC_INCLUDES) < $< | \
		sed -e '/^default plugins/d'  -e '/^lang_geda/d' | \
		sed -e '/^parameter /d' $(EXTRA_SED_ARG)| \
		grep -v ^@ > $@

testbench.out: EXTRA_SED_ARG=-e '/^parameter /d'
# $(GC_TESTS:%.gc=%.out): EXTRA_SED_ARG=-e '1,/^\/\/v /d'
$(GC_TESTS:%.gc=%.out): EXTRA_SED_ARG=-e '1,/^stashing as/d'
$(GC_TESTS:%.gc=%.out): EXTRA_SED_ARG+=-e '/^\/\/v /d'
spice.out: EXTRA_SED_ARG=-e '1,/^core-lib version/d' -e 's/499999.*$$/5/'
res_net.out: EXTRA_SED_ARG += -e 's/4999.*$$/5/'
verilog.out: EXTRA_SED_ARG=-e '1,/^stashing as/d'
verilog_place.out: EXTRA_SED_ARG=-e '1,/^stashing as/d'
notfound.out: EXTRA_SED_ARG=-e '1,/^core-lib version/d'
bogus.out: EXTRA_SED_ARG=
rc.2.out: EXTRA_SED_ARG += -e 's/675.*$$/675/'
rc.out: EXTRA_SED_ARG += -e '/vn2/s\#0\.11.*$$\#0.11\#'
rc.out: EXTRA_SED_ARG += -e '/vn2/s\#0\.67.*$$\#0.67\#'

$(ER_TESTS:%.sh=%.out): GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v
$(EG_TESTS:%.sch=%.out): GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v

rc.out rc.2.out: GC_INCLUDES+=-i $(abs_top_srcdir)/include/analog.v

eg2b.sch.out eg2b_recon.out: VLIB_FILES+= transistor.v diode.v
eg3.sch.out eg3_recon.out: VLIB_FILES += transistor.v 74.v
eg4_recon.out: VLIB_FILES += transistor.v connector.v
eg5_recon.out: VLIB_FILES = transistor.v connector.v analog.v
eg25_recon.out: VLIB_FILES = 74.v linear.v
eg25.sch.out: VLIB_FILES = 74.v linear.v
eg26_recon.out: VLIB_FILES = connector.v
eg42.sch.out: VLIB_FILES = transistor.v linear.v switcap.v diode.v analog.v
eg42_recon.out: VLIB_FILES = transistor.v linear.v switcap.v diode.v analog.v
eg43_recon.out: VLIB_FILES += asic.v analog.v

# vile hack. maybe should not wrap comments...?
$(ER_TESTS:%.sh=%.out): EXTRA_SED_ARGS=-e '1,/^stashing /d' -e 's%^//v %v %' -e '/^\/\/.*/d'

#$(EG_TESTS:%.sch=%.out)
$(ER_TESTS:%.sh=%.out): %.out: %.gc
	-@echo "$(GNUCAP) $<"; $(GNUCAP_ENV) $(GNUCAP) -a $(MODULE) $(GC_INCLUDES) \
		$(VLIB_FILES:%=-i $(abs_top_srcdir)/include/%) < $< | \
		sed -e '/^default plugins/d' \
		    -e '/^lang_geda/d' $(EXTRA_SED_ARGS) | \
		grep -v ^@ > $@

$(EG_TESTS:%.sch=%.sch.out): EXTRA_SED_ARGS= | sed -n '/^\/\/v /,$$p' | sed -e '/^parameter /d'

eg_tests: $(EG_TESTS:%.sch=%.log)
er_tests: $(ER_TESTS:%.sh=%.log)
gc_tests: $(GC_TESTS:%.gc=%.log)

OUTFILES = \
	$(CUSTOM_TESTS:%.sh=%.out) \
	$(EG_TESTS:%=%.out) \
	$(ER_TESTS:%.sh=%.out) \
	$(GC_TESTS:%.gc=%.out)

LOGFILES = $(OUTFILES:%.out:%.log)

CLEANFILES = $(TESTS:%.gc:%.out) \
	$(OUTFILES)

$(GC_TESTS:%.gc=%.log): .P
$(GC_TESTS:%.gc=%.out): .P

.PHONY:
	
#	$(OUTFILES) $(LOGFILES)
$(EG_TESTS:%=%.out) \
$(EG_TESTS:%.sch=%.out) \
	$(EG_TESTS:%.sch=%.log) : .P

$(CUSTOM_TESTS:%.sh=%.log) : .P


$(OUTFILES) $(LOGFILES): .P

.P:
	@:

.PHONY: %.reg clean-examples .P

EXTRA_DIST = \
	gc_log_compiler \
	sch_log_compiler \
	nets.sch nets.ref spice_foo.sp \
	dut.sch \
	auto_dut.1.sch \
	auto_top.1.sch \
	auto_dut.2.sch \
	auto_top.2.sch \
	auto_dut.3.sch \
	auto_top.3.sch \
	auto_dut.3a.sch \
	auto_dut.3b.sch \
	bogus.sch bogus.ref \
	gafrc \
	$(EG_TESTS:%.sch=%.ref) \
	$(ER_TESTS:%.sh=%.ref) \
	$(EG_TESTS) \
	$(ER_TESTS:%.sh=%.ref) \
	$(CUSTOM_TESTS:%.sh=%.ref) \
	$(GC_TESTS) \
	$(GC_TESTS:%.gc=%.ref) \
	$(SCHEMATICS)
