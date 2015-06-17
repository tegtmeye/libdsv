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

inline const std::string & format_str(dsv_log_code msg_code)
{
  switch(msg_code) {
    case dsv_parse_error:
      static const std::string parse_error("parse error: unexpected '$1'");
      return parse_error;

    default:
      ;
  };

  static const std::string unknown_error("unknown error code");
  return unknown_error;
}

std::string msg_log(dsv_parser_t parser, dsv_log_level log_level)
{
  std::stringstream result;

  // Get the number of log messages for \c log_level
  size_t num_msgs = dsv_parse_log_count(parser,log_level);

  // iterate over all codes
  for(size_t i=0; i<num_msgs; ++i) {
    // Get the code and minimum storage requirement for the msg parameters
    dsv_log_code msg_code;

    errno = 0;
    ssize_t len = dsv_parse_log(parser,log_level,i,&msg_code,0,0);
    if(len < 0) {
      std::stringstream err;
      err << "dsv_parse_error message code get failed with errno " << errno;
      throw std::runtime_error(err.str());
    }

    // msg_code now holds the code associated with the msg
    // len now holds the storage needed for the the total number of parameters
    // associated with msg_code.

    const std::string &fmt_str = format_str(msg_code);

    // Allocate buf based on minimum needed to store the parameters and the
    // length needed to store the format string potentially trimmed of the
    // placeholders. Do not forget to add space for the null terminator
    size_t storage_len = (fmt_str.size()+len+1);
    std::unique_ptr<char> buff(new char[storage_len]);

    // copy the format string to buff and set the null-terminator
    *(std::copy(fmt_str.begin(),fmt_str.end(),buff.get())) = 0;

    errno = 0;
    len = dsv_parse_log(parser,log_level,i,&msg_code,buff.get(),storage_len);
    if(len < 0) {
      std::stringstream err;
      err << "dsv_parse_error message formatter failed with errno " << errno;
      throw std::runtime_error(err.str());
    }

    result << buff.get() << "\n";
  }

  return result.str();
}

#if 0
boost::shared_ptr<dsv_parser_t> make_parser(void)
{
  std::auto_ptr<dsv_parser_t> temp(new dsv_parser_t());

  dsv_parser_create(temp.get());

  return boost::shared_ptr<dsv_parser_t>(temp.release(),&parser_destroy);
}

boost::shared_ptr<dsv_operations_t> make_operations(void)
{
  std::auto_ptr<dsv_operations_t> temp(new dsv_operations_t());

  dsv_operations_create(temp.get());

  return boost::shared_ptr<dsv_operations_t>(temp.release(),&operations_destroy);
}
#endif

inline void parser_destroy(dsv_parser_t *p)
{
  dsv_parser_destroy(*p);
}

inline void operations_destroy(dsv_operations_t *p)
{
  dsv_operations_destroy(*p);
}

}


BOOST_AUTO_TEST_SUITE( parse_suite )




#if 0
/** \test Test newline getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_newline_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  dsv_newline_behavior behavior = dsv_parser_get_newline_handling(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_permissive,
    "dsv_parser_get_newline_handling returned a value other than the default newline "
    "behavior (" << behavior << " != " << dsv_newline_permissive << ")");

  int err = dsv_parser_set_newline_handling(parser,(dsv_newline_behavior)999);
  BOOST_REQUIRE_MESSAGE(err!=0,
    "dsv_parser_set_newline_handling accepted a invalid value of dsv_newline_behavior");

  behavior = dsv_parser_get_newline_handling(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_permissive,
    "dsv_parser_get_newline_handling returned a value other than the default newline "
    "behavior after attempting a invalid value of dsv_newline_behavior ("
    << behavior << " != " << dsv_newline_permissive << ")");

  err = dsv_parser_set_newline_handling(parser,dsv_newline_lf_strict);
  BOOST_REQUIRE_MESSAGE(err==0,
    "dsv_parser_set_newline_handling failed with error value " << err);

  behavior = dsv_parser_get_newline_handling(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_lf_strict,
    "dsv_parser_get_newline_handling returned a value other than the set newline "
    "behavior (" << behavior << " != " << dsv_newline_lf_strict << ")");

  dsv_parser_destroy(parser);
}
#endif


























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
  dsv_set_record_callback(detail::field_callback,&context,operations);

  int result = dsv_parse(0,0,parser,operations);

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
  dsv_set_record_callback(detail::field_callback,&context,operations);

  int result = dsv_parse("nonexistant_file.dsv",0,parser,operations);

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

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  std::string filename(detail::testdatadir+"/empty.dsv");
  FILE *in = std::fopen(filename.c_str(),"rb");
  BOOST_REQUIRE_MESSAGE(in != 0,
    "Unit test failure: could not open unit test data file \"" << filename << "\"");

  std::shared_ptr<FILE> file_sentry(in,std::fclose);

  int result = dsv_parse(0,in,parser,operations);

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

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  std::string filename(detail::testdatadir+"/empty.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

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

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

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

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  dsv_set_record_callback(detail::field_callback,&context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_lf.dsv");
  int result = dsv_parse(filename.c_str(),0,parser,operations);

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse incorrectly accepted a non-quoted linefeed when set to be RFC4180-strict: " 
      << filename);
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



BOOST_AUTO_TEST_SUITE_END()

}
}
