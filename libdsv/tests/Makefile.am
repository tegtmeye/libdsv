#
#  Copyright (c) 2014-2017, Mike Tegtmeyer All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  1. Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#  3. Neither the name of the copyright holder nor the names of its
#  contributors may be used to endorse or promote products derived from
#  this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

master_suite=$(top_srcdir)/tests/master_suite.cc

additional_test_cppflags= \
	-arch x86_64 -std=c++11 -Wall -Wextra --save-temps \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libdsv/src \
	-DTESTDATA_DIR='$(srcdir)/data' \
	$(SYSTEM_BOOST_CPPFLAGS)

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
	api_parser_low_object_suite \
	api_operations_object_suite \
	api_defaults_parse_suite

scanner_test_SOURCES=$(master_suite) \
	test_detail.h \
	scanner_test.cc
scanner_test_CPPFLAGS=$(additional_test_cppflags)
scanner_test_LDADD=$(additional_test_libs)
scanner_test_LDFLAGS=

scanner_perf_test_SOURCES= \
	test_detail.h \
        scanner_perf_test.cc
scanner_perf_test_CPPFLAGS=$(additional_test_cppflags) -DNDEBUG=1
scanner_perf_test_LDADD=$(additional_perf_test_libs)
scanner_perf_test_LDFLAGS=

api_parser_low_object_suite_SOURCES=$(master_suite) \
	api_detail.h \
	api_detail.cc \
	test_detail.h \
        api_parser_low_object_suite.cc
api_parser_low_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_low_object_suite_LDADD=$(additional_test_libs)
api_parser_low_object_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_operations_object_suite_SOURCES=$(master_suite) \
	api_detail.h \
	api_detail.cc \
	test_detail.h \
        api_operations_object_suite.cc
api_operations_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_operations_object_suite_LDADD=$(additional_test_libs)
api_operations_object_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_parser_high_object_suite_SOURCES=$(master_suite) \
        api_detail.h \
        api_detail.cc \
        test_detail.h \
        api_parser_high_object_suite.cc
api_parser_high_object_suite_CPPFLAGS=$(additional_test_cppflags)
api_parser_high_object_suite_LDADD=$(additional_test_libs)
api_parser_high_object_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_defaults_parse_suite_SOURCES=$(master_suite) \
        api_detail.h \
        api_detail.cc \
        test_detail.h \
        api_defaults_parse_suite.cc
api_defaults_parse_suite_CPPFLAGS=$(additional_test_cppflags)
api_defaults_parse_suite_LDADD=$(additional_test_libs)
api_defaults_parse_suite_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_RFC4180_parse_test_SOURCES=$(master_suite) \
        api_detail.h \
        api_detail.cc \
        test_detail.h \
	api_RFC4180_parse_test.cc
api_RFC4180_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_RFC4180_parse_test_LDADD=$(additional_test_libs)
api_RFC4180_parse_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_RFC4180_permissive_parse_test_SOURCES=$(master_suite) \
        api_detail.h \
        api_detail.cc \
        test_detail.h \
	api_RFC4180_permissive_parse_test.cc
api_RFC4180_permissive_parse_test_CPPFLAGS=$(additional_test_cppflags)
api_RFC4180_permissive_parse_test_LDADD=$(additional_test_libs)
api_RFC4180_permissive_parse_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_column_count_test_SOURCES=$(master_suite) \
        api_detail.h \
        api_detail.cc \
        test_detail.h \
	api_column_count_test.cc
api_column_count_test_CPPFLAGS=$(additional_test_cppflags)
api_column_count_test_LDADD=$(additional_test_libs)
api_column_count_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

api_alt_delimiter_test_SOURCES=$(master_suite) \
        api_detail.h \
        api_detail.cc \
        test_detail.h \
	api_alt_delimiter_test.cc
api_alt_delimiter_test_CPPFLAGS=$(additional_test_cppflags)
api_alt_delimiter_test_LDADD=$(additional_test_libs)
api_alt_delimiter_test_LDFLAGS=\
	$(libdsv_test_ldflags)\
	$(additional_test_ldflags)

TESTS=\
	scanner_test \
	api_parser_low_object_suite \
	api_operations_object_suite \
	api_defaults_parse_suite

CLEANFILES=\
	scanner_test.log \
	scanner_test.trs \
	scanner_perf_test.log \
	scanner_perf_test.trs \
	api_parser_low_object_suite.log \
	api_parser_low_object_suite.trs \
	api_operations_object_suite.log \
	api_operations_object_suite.trs \
	api_parser_high_object_suite.log \
	api_parser_high_object_suite.trs \
	api_defaults_parse_suite.log \
	api_defaults_parse_suite.trs \
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
