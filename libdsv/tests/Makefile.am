master_suite=$(top_srcdir)/tests/master_suite.cc

additional_test_cppflags= \
	-pedantic -Wno-long-long -ansi -Wall \
        -I$(top_srcdir) \
        -I$(top_srcdir)/libdsv/src \
	-DTESTDATA_DIR='$(srcdir)/data' \
        $(BOOST_CPPFLAGS)

additional_test_libs= \
	$(BOOST_REGEX_LIBS) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_FILESYSTEM_LIBS)

additional_test_ldflags=\
        $(top_builddir)/libdsv/src/libdsv.la \
        $(BOOST_LDFLAGS) \
        $(BOOST_REGEX_LDFLAGS) \
        $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS)

check_PROGRAMS= \
	api_parser_object_suite \
	api_operations_object_suite \
	api_parser_log_suite \
	api_parse_test

api_parser_object_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_parser_object_suite.cc
api_parser_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_object_suite_LDADD=$(additional_test_libs)
api_parser_object_suite_LDFLAGS=$(additional_test_ldflags)

api_operations_object_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_operations_object_suite.cc
api_operations_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_operations_object_suite_LDADD=$(additional_test_libs)
api_operations_object_suite_LDFLAGS=$(additional_test_ldflags)

api_parser_log_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_parser_log_suite.cc
api_parser_log_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_log_suite_LDADD=$(additional_test_libs)
api_parser_log_suite_LDFLAGS=$(additional_test_ldflags)

api_parse_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_parse_test.cc
api_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_parse_test_LDADD=$(additional_test_libs)
api_parse_test_LDFLAGS=$(additional_test_ldflags)

TESTS=\
	api_parser_object_suite \
	api_operations_object_suite \
	api_parser_log_suite \
	api_parse_test

CLEANFILES=\
	api_parser_object_suite.log \
	api_parser_object_suite.trs \
	api_operations_object_suite.log \
	api_operations_object_suite.trs \
	api_parser_log_suite.log \
	api_parser_log_suite.trs \
	api_parse_test.log \
	api_parse_test.trs \
	api_test-suite.log

EXTRA_DIST=\
	data/empty.dsv \
	data/empty_lf.dsv \
	data/empty_crlf.dsv



