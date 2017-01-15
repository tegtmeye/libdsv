/*
  Copyright (c) 2014-2017, Mike Tegtmeyer All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>

#include <dsv_parser.h>
#include "test_detail.h"

#include <errno.h>
#include <stdio.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstdio>

/** \file
 *  \brief Unit tests for basic parsing
 */




namespace dsv {
namespace test {


namespace b = boost;
namespace fs = boost::filesystem;



namespace detail {

// note escaped '\'
const char *rfc4180_charset_field = " !#$%&'()*+-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

const char rfc4180_quoted_charset_field[] = {
  ' ','!',0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K',
  'L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_','`',
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
  'w','x','y','z','{','|','}','~',0x0D,0x0A
};


}


BOOST_AUTO_TEST_SUITE( alt_parse_suite )







#if 0
/** CHECK BASIC RECOGNITION OF THE CHARACTERSET **/

/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_crlf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}

/** \test Attempt to parse an named file with a single field consisting of the
 *    quoted rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_quoted_rfc4180_charset_crlf )
{
std::cerr << "START SINGLE CHARSET\n";
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_quoted_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_quoted_rfc4180_charset_crlf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}







/** CHECK BASIC NEWLINE HANDLING **/


/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set. This file is not line terminated. If this test seems to
 *    be failing for unknown reasons, make sure that the test data file is not appended
 *    with an auto-generated newline as many text editors tend to do (as well as git if
 *    it isn't set up correctly to handle this case).
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}

// todo, update this when logging gets worked out
/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set terminated with just a linefeed. When the parser is
 *    set to be RFC4180 strict, this is an error.
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_rfc4180_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::vector<std::string> context;
  dsv_set_header_callback(detail::fill_vector_with_fields,&context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_lf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  std::stringstream out;
  for(std::size_t i=0; i<context.size(); ++i)
    out << context[i] << " ";
  out << "\n";

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse incorrectly accepted a non-quoted linefeed when set to be RFC4180-strict: "
      << filename << ". Received '" << out.str() << "'");
}

// todo, update this when logging gets worked out
/** \test Attempt to parse an named file with a single field consisting of the
 *    the POSIX character set terminated with just a linefeed.
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_POSIX_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set LF strict
  dsv_parser_set_newline_handling(parser,dsv_newline_lf_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_lf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}

// todo, update this when logging gets worked out
/** \test Attempt to parse an named file with a single field consisting of the
 *    the POSIX character set terminated with a carriage return and line feed. When the
 *    parser is set to be LF strict, this is an error.
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_POSIX_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set LF strict
  dsv_parser_set_newline_handling(parser,dsv_newline_lf_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::vector<std::string> context;
  dsv_set_header_callback(detail::fill_vector_with_fields,&context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_crlf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  std::stringstream out;
  for(std::size_t i=0; i<context.size(); ++i)
    out << context[i] << " ";
  out << "\n";

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse incorrectly accepted a CRLF when set to be LF-strict: "
      << filename << ". Received '" << out.str() << "'");
}






/** CHECK BASIC DELIMITER HANDLING **/


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_named_multifield_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_rfc4180_charset_crlf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_named_multifield_rfc4180_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_quoted_charset_field);
  header_context.field_matrix[0].push_back(detail::rfc4180_quoted_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_quoted_rfc4180_charset_crlf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}


/** CHECK NEWLINE HANDLING FOR MULTIFIELDS **/

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. This file is not newline terminated
 */
BOOST_AUTO_TEST_CASE( parse_named_multifield_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  header_context.field_matrix[0].push_back(detail::rfc4180_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_rfc4180_charset.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set. This file is not newline terminated.
 */
BOOST_AUTO_TEST_CASE( parse_named_multifield_rfc4180_quoted_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context header_context;
  header_context.field_matrix.resize(1);
  header_context.field_matrix[0].push_back(detail::rfc4180_quoted_charset_field);
  header_context.field_matrix[0].push_back(detail::rfc4180_quoted_charset_field);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  record_context.invocation_limit = 0;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_quoted_rfc4180_charset.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);
}
















#if 0
/** \test Parse a file with no records but has a single LF using permissive newline
 *  handling
 */
BOOST_AUTO_TEST_CASE( permissively_parse_recordless_file_w_LF )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  assert(dsv_parser_set_newline_handling(parser,dsv_newline_permissive) == 0);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  boost::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  fs::path filename(detail::testdatadir/"empty_lf.dsv");

  int result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed on '" << filename << "' with message \""
      << detail::msg_log(parser,dsv_log_all) << "\"");
}

/** \test Parse a file with no records but has a single CRLF using permissive newline
 *  handling
 */
BOOST_AUTO_TEST_CASE( permissively_parse_recordless_file_w_CRLF )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  assert(dsv_parser_set_newline_handling(parser,dsv_newline_permissive) == 0);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  boost::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  fs::path filename(detail::testdatadir/"empty_crlf.dsv");

  int result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed on '" << filename << "' with message \""
      << detail::msg_log(parser,dsv_log_all) << "\"");
}

/**
 *  \test Parse a file with no records using strict LF newline handling
 */
BOOST_AUTO_TEST_CASE( strictly_parse_recordless_LF_handling )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  assert(dsv_parser_set_newline_handling(parser,dsv_newline_lf_strict) == 0);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  boost::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  fs::path filename(detail::testdatadir/"empty_lf.dsv");

  int result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed on '" << filename << "' with message \""
      << detail::msg_log(parser,dsv_log_all) << "\"");

  filename = fs::path(detail::testdatadir/"empty_crlf.dsv");

  result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == -1,
    "dsv_parse succeeded or failed for incorrect reasons on '" << filename
      << "' with message \"" << detail::msg_log(parser,dsv_log_all) << "\"");
}

/**
 *  \test Parse a file with no records using strict CRLF newline handling
 */
BOOST_AUTO_TEST_CASE( strictly_parse_recordless_CRLF_handling )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  assert(dsv_parser_set_newline_handling(parser,dsv_newline_crlf_strict) == 0);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  boost::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  fs::path filename(detail::testdatadir/"empty_crlf.dsv");

  int result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed on '" << filename << "' with message \""
      << detail::msg_log(parser,dsv_log_all) << "\"");

  filename = fs::path(detail::testdatadir/"empty_lf.dsv");

  result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == -1,
    "dsv_parse succeeded or failed for incorrect reasons on '" << filename
      << "' with message \"" << detail::msg_log(parser,dsv_log_all) << "\"");
}

#endif
#endif


BOOST_AUTO_TEST_SUITE_END()

}
}
