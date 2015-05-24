lib_LTLIBRARIES= libdsv.la

BUILT_SOURCES=dsv_grammar.hh
AM_YFLAGS= -d -p parser_


libdsv_la_SOURCES= \
	dsv_grammar.yy \
	scanner_state.h \
	file_parser.h \
	file_parser.cc \
	parse_operations.h \
	parser.h \
	dsv_parser.cc

libdsv_la_CPPFLAGS=-pedantic -ansi -Wall -I$(top_srcdir) \
	$(BOOST_CPPFLAGS)

libdsv_la_LDFLAGS= -version-info 0:0:0

libdsv_la_LIBADD=

EXTRA_DIST=\
	dsv_grammar.hh \
	dsv_grammar.cc

CLEANFILES=\
	dsv_grammar.hh \
	dsv_grammar.cc

DISTCLEANFILES= \
	dsv_grammar.hh \
	dsv_grammar.cc
