#
# Copyright (c) 2000, 2001, 2003, 2007, 2009 Thorsten Kukuk <kukuk@linux-nis.org>
#
AUTOMAKE_OPTIONS = 1.7 gnits

localedir = $(datadir)/locale

DEFS = @DEFS@ -DLOCALEDIR=\"$(localedir)\" -DYPBINDIR=\"$(libexecdir)\" \
	-DCONFDIR=\"$(sysconfdir)\" -DYPMAPDIR=\"@YPMAPDIR@\" \
	-DUSE_FQDN=@USE_FQDN@
AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir) -I$(top_builddir) -I$(srcdir)

CLEANFILES = *~ ${MANS}

EXTRA_DIST = ${MANS} ${XMLS}

man_MANS = yppush.8
XMLS = yppush.8.xml

sbin_PROGRAMS = yppush

yppush_SOURCES = yppush.c

yppush_LDADD =  @PIE_LDFLAGS@ @LIBDBM@ $(top_builddir)/lib/libyp.a \
	@NSL_LIBS@ @TIRPC_LIBS@
yppush_CFLAGS = @PIE_CFLAGS@ @NSL_CFLAGS@ @TIRPC_CFLAGS@

if ENABLE_REGENERATE_MAN
%.8: %.8.xml
	$(XMLLINT) --nonet --xinclude --postvalid --noout $<
	$(XSLTPROC) -o $(srcdir)/$@ --path $(srcdir) --xinclude --nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<
endif
