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

/** \file
 *  \brief Unit tests for basic parsing
 */




namespace dsv {
namespace test {


namespace b = boost;
namespace fs = boost::filesystem;



namespace detail {

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

inline void parser_destroy(dsv_parser_t *p)
{
  dsv_parser_destroy(*p);
}

inline void operations_destroy(dsv_operations_t *p)
{
  dsv_operations_destroy(*p);
}

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


}


BOOST_AUTO_TEST_SUITE( parse_suite )


/** \test
 *  Check for no messages log_count on no-parse condition
 */
BOOST_AUTO_TEST_CASE( no_parse_parse_log_count )
{
  dsv_parser_t parser = {};
  dsv_parser_create(&parser);

  size_t msg_len = dsv_parse_log_count(parser,dsv_log_all);

  BOOST_REQUIRE_MESSAGE(msg_len == 0,
    "dsv_parse_error returns a nonzero value on a newly created parser");
}


/** \test Check negative return from parse_log on no-parse condition with zero code and
 *  zero buff and zero length
 */
BOOST_AUTO_TEST_CASE( no_parse_parse_log_zero_code_zero_buff_zero_length )
{
  dsv_parser_t parser = {};
  dsv_parser_create(&parser);

  ssize_t ret_msg_len = dsv_parse_log(parser,dsv_log_all,0,0,0,0);
  BOOST_REQUIRE_MESSAGE(ret_msg_len < 0,
    "dsv_parse_error returns a nonnegative value '" << ret_msg_len <<
    "' when providing a zero code and buffer of 0 len on a parser without actually "
    "parsing");
  BOOST_REQUIRE_MESSAGE(errno == EINVAL,
    "dsv_parse_error deos not set errno to EINVAL when providing a zero code and buffer "
    "of 0 len on a parser without actually parsing");
}

/** \test Check negative return from parse_log on no-parse condition with nonzero buff
 *  and nonzero length
 */
BOOST_AUTO_TEST_CASE( no_parse_parse_log_nonzero_buff_nonzero_length )
{
  dsv_parser_t parser = {};
  dsv_parser_create(&parser);

  char buf[256];
  std::fill(buf,buf+255,'X');
  buf[255] = 0;

  dsv_log_code msg_code;
  ssize_t ret_msg_len = dsv_parse_log(parser,dsv_log_all,0,&msg_code,buf,256);
  BOOST_REQUIRE_MESSAGE(ret_msg_len < 0,
    "dsv_parse_error returns a nonnegative value '" << ret_msg_len <<
    "' when providing buffer of 0 len on a parser without actually parsing");

  bool found = false;
  for(size_t i=0; i<255 && !found; ++i)
    found = (buf[i] != 'X');
  found = found || buf[255] != 0;

  // reset the nul terminator if needed
  buf[255] = 0;
  BOOST_REQUIRE_MESSAGE(!found,
    "dsv_parse_error erroneously modifies the provided buffer containing all 'X's of "
    "nonzero length on a parser without actually parsing. Buffer: '" << buf << "'");
}


/** \test Creation of operations object
 */
BOOST_AUTO_TEST_CASE( operations_create_check )
{
  dsv_operations_t operations = {};

  int err = dsv_operations_create(&operations);

  BOOST_REQUIRE_MESSAGE(err == 0,"dsv_operations_create fails");

  dsv_operations_destroy(operations);
}


/** \test Getting and setting of operations members
 */
BOOST_AUTO_TEST_CASE( record_getting_and_setting )
{
  dsv_operations_t operations = {};

  int err = dsv_operations_create(&operations);
  BOOST_REQUIRE_MESSAGE(err == 0,"dsv_operations_create succeeds");

  record_callback_t fn = dsv_get_record_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == 0,
    "dsv_get_record_callback returns zero for an unset record callback");

  void *context = dsv_get_record_context(operations);
  BOOST_REQUIRE_MESSAGE(context == 0,
    "dsv_get_record_context returns zero for an unset record context");

  detail::record_context record_context; // may throw

  dsv_set_record_callback(detail::record_callback,&record_context,operations);

  fn = dsv_get_record_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == detail::record_callback,
    "dsv_get_record_callback returns the just-set record callback");

  context = dsv_get_record_context(operations);
  BOOST_REQUIRE_MESSAGE(context == &record_context,
    "dsv_get_record_context returns the just-set record context");

  dsv_operations_destroy(operations);
}

/** \test Create and destroy parser object
 */
BOOST_AUTO_TEST_CASE( parser_create )
{
  dsv_parser_t parser = {};

  int err = dsv_parser_create(&parser);

  BOOST_REQUIRE_MESSAGE(err == 0,"dsv_parser_create succeeds");

  dsv_parser_destroy(parser);
}

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







#if 0
/** \test Attempt to parse a nonexistent file
 */
BOOST_AUTO_TEST_CASE( parse_nonexistent_file )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  boost::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::record_context context;
  dsv_set_record_callback(detail::record_callback,&context,operations);

  fs::path filename(detail::testdatadir/"nonexistant_file.dsv");

  int result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == ENOENT,
    "dsv_parse attempted to open a nonexistent file and did not return ENOENT");
}


/** \test Parse a completely empty file
 */
BOOST_AUTO_TEST_CASE( parse_empty_file )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  boost::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::record_context context;
  dsv_set_record_callback(detail::record_callback,&context,operations);

  fs::path filename(detail::testdatadir/"empty.dsv");

  BOOST_TEST_MESSAGE("Testing file " << filename.string());

  int result = dsv_parse(filename.string().c_str(),parser,operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed on '" << filename << "' with message \""
      << detail::msg_log(parser,dsv_log_all) << "\"");
}

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

  detail::record_context context;
  dsv_set_record_callback(detail::record_callback,&context,operations);

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

  detail::record_context context;
  dsv_set_record_callback(detail::record_callback,&context,operations);

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

  detail::record_context context;
  dsv_set_record_callback(detail::record_callback,&context,operations);

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

  detail::record_context context;
  dsv_set_record_callback(detail::record_callback,&context,operations);

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
