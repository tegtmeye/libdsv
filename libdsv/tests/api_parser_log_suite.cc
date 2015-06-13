#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"


#include <errno.h>
#include <stdio.h>

#include <string>

/** \file
 *  \brief Unit tests for parser logging
 *
 *  These unit tests are scheduled after basic parser creation and destruction tests
 *  are completed. That is, authors of tests here can expect that there will be no
 *  errors with basic parser object operations.
 *
 *  For logging unit tests that require basic parse operations to be valid, see:
 *
 */

namespace dsv {
namespace test {

BOOST_AUTO_TEST_SUITE( api_parser_log_suite )


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


BOOST_AUTO_TEST_SUITE_END()

}
}
