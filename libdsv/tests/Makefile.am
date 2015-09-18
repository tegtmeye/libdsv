master_suite=$(top_srcdir)/tests/master_suite.cc

additional_test_cppflags= \
	-arch x86_64 -pedantic -Wno-long-long -ansi -Wall -std=c++11\
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
	scanner_test \
	api_parser_object_suite \
	api_operations_object_suite \
	api_RFC4180_parse_test \
	api_RFC4180_permissive_parse_test \
	api_column_count_test

scanner_test_SOURCES=$(master_suite) \
        scanner_test.cc
scanner_test_CPPFLAGS=$(additional_test_cppflags)
scanner_test_LDADD=$(additional_test_libs)
scanner_test_LDFLAGS=$(additional_test_ldflags)

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

api_RFC4180_parse_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_RFC4180_parse_test.cc
api_RFC4180_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_RFC4180_parse_test_LDADD=$(additional_test_libs)
api_RFC4180_parse_test_LDFLAGS=$(additional_test_ldflags)

api_RFC4180_permissive_parse_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_RFC4180_permissive_parse_test.cc
api_RFC4180_permissive_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_RFC4180_permissive_parse_test_LDADD=$(additional_test_libs)
api_RFC4180_permissive_parse_test_LDFLAGS=$(additional_test_ldflags)

api_column_count_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_column_count_test.cc
api_column_count_test_CPPFLAGS=$(additional_test_cppflags)
api_column_count_test_LDADD=$(additional_test_libs)
api_column_count_test_LDFLAGS=$(additional_test_ldflags)


TESTS=\
	scanner_test \
	api_parser_object_suite \
	api_operations_object_suite \
	api_RFC4180_parse_test \
	api_RFC4180_permissive_parse_test \
	api_column_count_test

CLEANFILES=\
	scanner_test.log \
	scanner_test.trs \
	api_parser_object_suite.log \
	api_parser_object_suite.trs \
	api_operations_object_suite.log \
	api_operations_object_suite.trs \
	api_RFC4180_parse_test.log \
	api_RFC4180_parse_test.trs \
	api_RFC4180_permissive_parse_test.log \
	api_RFC4180_permissive_parse_test.trs \
	api_column_count_test.log \
	api_column_count_test.trs
	api_test-suite.log

EXTRA_DIST=
