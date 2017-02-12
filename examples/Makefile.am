EXTRA_DIST = \
	README \
	comparator \
	freqdiv \
	opamp \
	osc \
	trigger

install-data-local:
	   $(INSTALL) -d $(DESTDIR)$(docdir)/examples
		for dir in $(EXTRA_DIST) ; do \
			$(INSTALL) -d $(DESTDIR)$(docdir)/examples/$$dir; \
			$(INSTALL_DATA) $(srcdir)/$$dir/* $(DESTDIR)$(docdir)/examples/$$dir; \
		done

CLEANFILES = */*.out */*~
