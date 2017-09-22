include $(top_srcdir)/Makefile.common

zhttpincludedir = $(includedir)/zhttp

zhttpinclude_HEADERS = zhttp.h \
					   zh-srv.h \
					   zh-cli.h \
					   zh-method.h
					   zh-msg.h

lib_LTLIBRARIES = libzhttp.la
libzhttp_la_SOURCES = zhttp.c \
					  zh-method.c

zh-method.c : zh-method.gperf
	$(GPERF) $< > $@

EXTRA_DIST = zh-method.gperf
EXTRA_CLEAN = zh-method.c
