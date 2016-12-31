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

additional_perf_test_libs= \
	$(BOOST_REGEX_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_FILESYSTEM_LIBS)

libdsv_test_ldflags=\
	$(top_builddir)/libdsv/src/libdsv.la

additional_test_ldflags=\
	$(BOOST_LDFLAGS) \
	$(BOOST_REGEX_LDFLAGS) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS)

additional_perf_test_ldflags=\
	$(BOOST_LDFLAGS) \
	$(BOOST_REGEX_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_FILESYSTEM_LDFLAGS)

check_PROGRAMS= \
	scanner_test \
	scanner_perf_test \
	equiv_bytesequence_test \
	read_bytes_test \
	api_parser_low_object_suite \
	api_operations_object_suite \
	api_parser_defaults_suite \
	api_parser_high_object_suite \
	api_RFC4180_parse_test \
	api_RFC4180_permissive_parse_test \
	api_column_count_test \
	api_alt_delimiter_test

scanner_test_SOURCES=$(master_suite) \
	test_detail.h \
	scanner_test.cc
scanner_test_CPPFLAGS=$(additional_test_cppflags)
scanner_test_LDADD=$(additional_test_libs)
scanner_test_LDFLAGS=

scanner_perf_test_SOURCES= \
	test_detail.h \
        scanner_perf_test.cc
scanner_perf_test_CPPFLAGS=$(additional_test_cppflags)
scanner_perf_test_LDADD=$(additional_perf_test_libs)
scanner_perf_test_LDFLAGS=

equiv_bytesequence_test_SOURCES=$(master_suite) \
	test_detail.h \
        equiv_bytesequence_test.cc
equiv_bytesequence_test_CPPFLAGS=$(additional_test_cppflags)
equiv_bytesequence_test_LDADD=$(additional_test_libs)
equiv_bytesequence_test_LDFLAGS=$(additional_test_ldflags)

read_bytes_test_SOURCES=$(master_suite) \
	test_detail.h \
        read_bytes_test.cc
read_bytes_test_CPPFLAGS=$(additional_test_cppflags)
read_bytes_test_LDADD=$(additional_test_libs)
read_bytes_test_LDFLAGS=

api_parser_low_object_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_parser_low_object_suite.cc
api_parser_low_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_low_object_suite_LDADD=$(additional_test_libs)
api_parser_low_object_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_operations_object_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_operations_object_suite.cc
api_operations_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_operations_object_suite_LDADD=$(additional_test_libs)
api_operations_object_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_parser_defaults_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_parser_defaults_suite.cc
api_parser_defaults_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_defaults_suite_LDADD=$(additional_test_libs)
api_parser_defaults_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_parser_high_object_suite_SOURCES=$(master_suite) \
	test_detail.h \
        api_parser_high_object_suite.cc
api_parser_high_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_high_object_suite_LDADD=$(additional_test_libs)
api_parser_high_object_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_RFC4180_parse_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_RFC4180_parse_test.cc
api_RFC4180_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_RFC4180_parse_test_LDADD=$(additional_test_libs)
api_RFC4180_parse_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_RFC4180_permissive_parse_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_RFC4180_permissive_parse_test.cc
api_RFC4180_permissive_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_RFC4180_permissive_parse_test_LDADD=$(additional_test_libs)
api_RFC4180_permissive_parse_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_column_count_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_column_count_test.cc
api_column_count_test_CPPFLAGS=$(additional_test_cppflags)
api_column_count_test_LDADD=$(additional_test_libs)
api_column_count_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_alt_delimiter_test_SOURCES=$(master_suite) \
	test_detail.h \
	api_alt_delimiter_test.cc
api_alt_delimiter_test_CPPFLAGS=$(additional_test_cppflags)
api_alt_delimiter_test_LDADD=$(additional_test_libs)
api_alt_delimiter_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

TESTS=\
	scanner_test \
	scanner_perf_test \
	equiv_bytesequence_test \
	read_bytes_test \
	api_parser_low_object_suite \
	api_parser_high_object_suite \
	api_operations_object_suite \
	api_parser_defaults_suite \
	api_RFC4180_parse_test \
	api_RFC4180_permissive_parse_test \
	api_column_count_test \
	api_alt_delimiter_test

CLEANFILES=\
	scanner_test.log \
	scanner_test.trs \
	scanner_perf_test.log \
	scanner_perf_test.trs \
	equiv_bytesequence_test.log \
	equiv_bytesequence_test.trs \
	read_bytes_test.log \
	read_bytes_test.trs \
	api_parser_low_object_suite.log \
	api_parser_low_object_suite.trs \
	api_parser_high_object_suite.log \
	api_parser_high_object_suite.trs \
	api_operations_object_suite.log \
	api_operations_object_suite.trs \
	api_parser_defaults_suite.log \
	api_parser_defaults_suite.trs \
	api_RFC4180_parse_test.log \
	api_RFC4180_parse_test.trs \
	api_RFC4180_permissive_parse_test.log \
	api_RFC4180_permissive_parse_test.trs \
	api_column_count_test.log \
	api_column_count_test.trs \
	api_alt_delimiter_test.log \
	api_alt_delimiter_test.trs \
	api_test-suite.log

EXTRA_DIST=
