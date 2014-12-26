ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES= libdsv.la

BUILT_SOURCES=dsv_grammar.hh
AM_YFLAGS= -d -p dsv_parser_
AM_LFLAGS= -Pdsv_parser_ -odsv_rules.cc --header-file=dsv_rules.h


libdsv_la_SOURCES= \
	dsv_rules.cc \
	dsv_grammar.yy \
	parser_state.h \
	parser_state_detail.h \
	dsv_parser.h \
	dsv_parser.cc

include_HEADERS= \
	dsv_parser.h

dsv_rules.cc: dsv_rules.ll
	$(LEX) $(LFLAGS) $(AM_LFLAGS) $^


libdsv_la_CPPFLAGS=-pedantic -ansi -Wall \
	$(BOOST_CPPFLAGS)

libdsv_la_LDFLAGS= -version-info 0:0:0

libdsv_la_LIBADD=


EXTRA_DIST=\
	COPYING \
	ChangeLog \
	INSTALL \
	README \
	dsv_rules.ll \
	dsv_rules.h \
	dsv_rules.cc \
	dsv_grammar.hh \
	dsv_grammar.cc

CLEANFILES=\
	dsv_rules.h \
	dsv_rules.cc \
	dsv_grammar.hh \
	dsv_grammar.cc

DISTCLEANFILES= \
	dsv_rules.h \
	dsv_rules.cc \
	dsv_grammar.hh \
	dsv_grammar.cc
