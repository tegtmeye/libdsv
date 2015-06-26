#include <boost/test/unit_test.hpp>

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

namespace detail {



}


BOOST_AUTO_TEST_SUITE( RFC4180_parse_suite )






/** \test Attempt to parse a unnamed file with a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_unnamed_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  int result;
  if((result = dsv_parse(0,0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  /**
   *  It appears that the underlying fopen call can return error codes in platform
   *  dependent ways. For example, in POSIX it seems that fopen(0,...) would return 0
   *  and set errno to ENOENT but on MAC it return EFAULT which is equally justifiable
   *  so just make sure it doesn't return 0 here.
   */
  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse attempted to open a nonexistent file and did not return nonzero");
}


/** \test Attempt to parse a named nonexistent file using a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_named_nonexistent_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;

  int result;
  if((result = dsv_parse("nonexistant_file.dsv",0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == ENOENT,
    "dsv_parse attempted to open a nonexistent file and did not return ENOENT");
}

/** \test Attempt to parse an unnamed empty file opened with a stream
 */
BOOST_AUTO_TEST_CASE( parse_unnamed_empty_file_with_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::string filename(detail::testdatadir+"/empty.dsv");
  FILE *in = std::fopen(filename.c_str(),"rb");
  BOOST_REQUIRE_MESSAGE(in != 0,
    "Unit test failure: could not open unit test data file \"" << filename << "\"");

  std::shared_ptr<FILE> file_sentry(in,std::fclose);

  int result;
  if((result = dsv_parse(0,in,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid but empty file");
}

/** \test Attempt to parse an named empty file with a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_named_empty_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::string filename(detail::testdatadir+"/empty.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid but empty file");
}




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

  const char * field_matrix[][1] = {
    {detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with a single field consisting of the
 *    quoted rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_quoted_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][1] = {
    {detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_quoted_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
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

  const char * field_matrix[][1] = {
    {detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
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

  std::vector<std::vector<std::string> > header_matrix;
  dsv_set_header_callback(detail::fill_matrix,&header_matrix,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_lf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse incorrectly accepted a non-quoted linefeed when set to be RFC4180-strict: " 
      << filename << ". Received HEADER\n" << detail::print_matrix(header_matrix));
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

  const char * field_matrix[][2] = {
    {detail::rfc4180_charset_field,detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. A middle fields is empty
 */
BOOST_AUTO_TEST_CASE( parse_named_multi_empty_field_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][3] = {
    {detail::rfc4180_charset_field,"",detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multi_empty_field_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
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

  const char * field_matrix[][2] = {
    {detail::rfc4180_quoted_charset_field,detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_quoted_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set. One of the fields is empty
 */
BOOST_AUTO_TEST_CASE( parse_named_multi_empty_field_rfc4180_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][2] = {
    {detail::rfc4180_quoted_charset_field,detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_quoted_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
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

  const char * field_matrix[][2] = {
    {detail::rfc4180_charset_field,detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_rfc4180_charset.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
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

  const char * field_matrix[][2] = {
    {detail::rfc4180_quoted_charset_field,detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multifield_quoted_rfc4180_charset.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);
}


/** CHECK MULTILINE HANDLING **/

/** \test Attempt to parse an named file with multiple lines containing a single field 
 *    consisting of the rfc4180 character set. The file should be interpreted as exactly
 *    one header and one record each with only one field
 */
BOOST_AUTO_TEST_CASE( parse_named_multiline_field_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][1] = {
    {detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context = detail::make_field_context(field_matrix);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multiline_single_field_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(record_context),
      "dsv_parse failed to parse complete record list (one " 
      << record_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with multiple lines containing a single field 
 *    consisting of the rfc4180 character set. The file should be interpreted as exactly
 *    one header and one record each with one field. This test file has no final newline
 */
BOOST_AUTO_TEST_CASE( parse_named_multiline_field_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][1] = {
    {detail::rfc4180_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context = detail::make_field_context(field_matrix);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multiline_single_field_rfc4180_charset.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(record_context),
      "dsv_parse failed to parse complete record list (one " 
      << record_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with multiple lines containing a single field 
 *    consisting of the rfc4180 character set. The first and third record contains
 *    a single field while the second record record is empty. This should fail when
 *    specifying RFC4180 because all rows should have the same number of fields.
 */
BOOST_AUTO_TEST_CASE( parse_named_multi_empty_line_field_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::vector<std::vector<std::string> > header_matrix;
  dsv_set_header_callback(detail::fill_matrix,&header_matrix,operations);

  std::vector<std::vector<std::string> > record_matrix;
  dsv_set_record_callback(detail::fill_matrix,&record_matrix,operations);

  std::string filename(detail::testdatadir+"/multi_empty_line_single_field_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse improperly accepted nonuniform number of fields for file: " << filename
      << ". Received HEADER\n" << detail::print_matrix(header_matrix) << " RECORD:\n"
      << detail::print_matrix(record_matrix));
}

/** \test Attempt to parse an named file with multiple lines containing a single field 
 *    consisting of the quoted rfc4180 character set. The file should be interpreted as 
 *    exactly one header and one record
 */
BOOST_AUTO_TEST_CASE( parse_named_multiline_field_quoted_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][1] = {
    {detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context = detail::make_field_context(field_matrix);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multiline_single_field_quoted_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(record_context),
      "dsv_parse failed to parse complete record list (one " 
      << record_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with multiple lines containing a single field 
 *    consisting of the quoted rfc4180 character set. The file should be interpreted as 
 *    exactly one header and one record. This test file has no final newline
 */
BOOST_AUTO_TEST_CASE( parse_named_multiline_field_quoted_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * field_matrix[][1] = {
    {detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context = detail::make_field_context(field_matrix);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multiline_single_field_quoted_rfc4180_charset.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(header_context),
      "dsv_parse failed to parse complete header list (one " 
      << header_context.invocation_count << " of " << 1 << ") for file: " << filename);

  BOOST_REQUIRE_MESSAGE(detail::required_invocations(record_context),
      "dsv_parse failed to parse complete record list (one " 
      << record_context.invocation_count << " of " << 1 << ") for file: " << filename);
}

/** \test Attempt to parse an named file with multiple lines containing a single field 
 *    consisting of the quoted rfc4180 character set. The first and third record contains
 *    a single field while the second record record is empty. This should fail when
 *    specifying RFC4180 because all rows should have the same number of fields.
 */
BOOST_AUTO_TEST_CASE( parse_named_multi_empty_line_field_quoted_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  const char * header_field_matrix[][1] = {
    {detail::rfc4180_quoted_charset_field}
  };

  const char * record_field_matrix[][1] = {
    {detail::rfc4180_quoted_charset_field},
    {""},
    {detail::rfc4180_quoted_charset_field}
  };
  
  detail::field_context header_context = detail::make_field_context(header_field_matrix);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context = detail::make_field_context(record_field_matrix);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/multi_empty_line_single_field_quoted_rfc4180_charset_crlf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse improperly accepted nonuniform number of fields for file: " << filename);
}





























BOOST_AUTO_TEST_SUITE_END()

}
}
