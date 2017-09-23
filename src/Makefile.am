include $(top_srcdir)/Makefile.common

zhttpincludedir = $(includedir)/zhttp

zhttpinclude_HEADERS = zhttp.h \
					   zh-srv.h \
					   zh-cli.h \
					   zh-method.h \
					   zh-status.h \
					   zh-msg.h

noinst_HEADERS = zh-msg-priv.h

lib_LTLIBRARIES = libzhttp.la
libzhttp_la_SOURCES = zhttp.c \
					  zh-method.c \
					  zh-msg.c

zh-method.c : zh-method.gperf
	$(GPERF) $< > $@

EXTRA_DIST = zh-method.gperf
EXTRA_CLEAN = zh-method.c
