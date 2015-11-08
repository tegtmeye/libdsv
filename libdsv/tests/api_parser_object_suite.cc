#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"


#include <errno.h>
#include <stdio.h>

#include <string>

/** \file
 *  \brief Unit tests for parser creation
 */

namespace dsv {
namespace test {



BOOST_AUTO_TEST_SUITE( api_parser_object_suite )

/** \test Basic parser object checks
 *
 *  These unit tests are scheduled before checks of the logging functionality.
 *  That is, do not place unit tests here that require logging functionality. For
 *  checks that require valid logging, see:
 *
 */

BOOST_AUTO_TEST_CASE( parser_create )
{
  dsv_parser_t parser;
  int result = dsv_parser_create(&parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_create failed with exit code " << result);
}

/** \test Destroy uninitialized parser object
 */
BOOST_AUTO_TEST_CASE( initialized_parser_destroy )
{
  dsv_parser_t parser;
  int result = dsv_parser_create(&parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_create failed with exit code " << result);

  dsv_parser_destroy(parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_destroy failed with exit code " << result);
}

/** \test Test newline getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_newline_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  dsv_newline_behavior behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_permissive,
    "dsv_parser_get_newline_behavior returned a value other than the default "
    "newline behavior (" << behavior << " != " << dsv_newline_permissive
    << ")");

  int err = dsv_parser_set_newline_behavior(parser,(dsv_newline_behavior)999);
  BOOST_REQUIRE_MESSAGE(err!=0,
    "dsv_parser_set_newline_behavior accepted a invalid value of "
    "dsv_newline_behavior");

  behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_permissive,
    "dsv_parser_get_newline_behavior returned a value other than the default "
    "newline behavior after attempting a invalid value of "
    "dsv_newline_behavior (" << behavior << " != " << dsv_newline_permissive
    << ")");

  err = dsv_parser_set_newline_behavior(parser,dsv_newline_lf_strict);
  BOOST_REQUIRE_MESSAGE(err==0,
    "dsv_parser_set_newline_behavior failed with error value " << err);

  behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_lf_strict,
    "dsv_parser_get_newline_behavior returned a value other than the set "
    "newline behavior (" << behavior << " != " << dsv_newline_lf_strict << ")");

  dsv_parser_destroy(parser);
}

/** \test Test field columns getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_field_columns_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  ssize_t field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == 0,
    "dsv_parser_get_field_columns returned a value other than the default 0 "
    " (" << field_cols << ")");

  dsv_parser_set_field_columns(parser,-1);
  field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == -1,
    "dsv_parser_get_field_columns returned a value other than the newly set "
    "value of -1 (" << field_cols << ")");

  dsv_parser_destroy(parser);
}

/** \test Test single delimiter getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_single_delimiter_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  std::vector<unsigned char> delimiter = {'a'};

  // add one for error checking
  std::vector<unsigned char> buf(7,'*');

  // CHECK dsv_parser_set_field_delimiter FUNCTION
  int err;
  BOOST_REQUIRE_MESSAGE((err = dsv_parser_set_field_delimiter(parser,'a')) == 0,
    "dsv_parser_set_field_delimiter failed with code: " << err);

  // CHECK dsv_parser_num_field_delimiters FUNCTION
  size_t size = dsv_parser_num_field_delimiters(parser);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_num_field_delimiters did not return 1 for a "
    "single byte delimiter but instead returned: " << size);

  // CHECK dsv_parser_get_field_delimiter FUNCTION
  size = dsv_parser_get_field_delimiter(parser,0,0,0,0);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a buffer length of 1 for a "
    "single byte delimiter but instead returned: " << size);


  // check for get with exact buffer size
  int repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),1,&repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a byte length of 1 when "
    "retrieving a single byte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == '*' && buf[2] == '*'
     && buf[3] == '*' && buf[4] == '*' && buf[5] == '*' && buf[6] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  BOOST_REQUIRE_MESSAGE(repeatflag == 0,
    "dsv_parser_get_field_delimiter did not return a nonrepeating flag but "
    "instead returned '" << repeatflag << "'");

  // reset buf
  buf.assign(7,'*');


  // check for get with larger buffer size
  repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),buf.size(),
    &repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a byte length of 1 when "
    "retrieving a single byte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == '*' && buf[2] == '*'
     && buf[3] == '*' && buf[4] == '*' && buf[5] == '*' && buf[6] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  BOOST_REQUIRE_MESSAGE(repeatflag == 0,
    "dsv_parser_get_field_delimiter did not return a nonrepeating flag but "
    "instead returned '" << repeatflag << "'");

  dsv_parser_destroy(parser);
}



/** \test Test single delimiter getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_multibyte_delimiter_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  std::vector<unsigned char> multibyte_delimiter = {'a','b','c','d','e','f'};

  // add one for error checking
  std::vector<unsigned char> buf(multibyte_delimiter.size()+4,'*');

  // CHECK dsv_parser_set_field_wdelimiter FUNCTION
  int err = dsv_parser_set_field_wdelimiter(parser,multibyte_delimiter.data(),
    multibyte_delimiter.size(),false);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
    << " for multibyte delimiter");

  // CHECK dsv_parser_num_field_delimiters FUNCTION
  size_t size = dsv_parser_num_field_delimiters(parser);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_num_field_delimiters did not return 1 for a "
    "single byte delimiter but instead returned: " << size);

  // CHECK dsv_parser_get_field_delimiter FUNCTION
  size = dsv_parser_get_field_delimiter(parser,0,0,0,0);
  BOOST_REQUIRE_MESSAGE(size == multibyte_delimiter.size(),
    "dsv_parser_get_field_delimiter did not return a buffer length of "
    << multibyte_delimiter.size() << " for a "
    "single byte delimiter but instead returned: " << size);

  // check for get with exact buffer size
  int repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),
    multibyte_delimiter.size(),&repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 6,
    "dsv_parser_get_field_delimiter did not return a byte length of 6 when "
    "retrieving the multibyte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c'
     && buf[3] == 'd' && buf[4] == 'e' && buf[5] == 'f' && buf[6] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  // reset buf
  buf.assign(multibyte_delimiter.size()+1,'*');


  // check for get with larger buffer size
  repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),buf.size(),
    &repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 6,
    "dsv_parser_get_field_delimiter did not return a byte length of 6 when "
    "retrieving the multibyte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c'
     && buf[3] == 'd' && buf[4] == 'e' && buf[5] == 'f' && buf[6] == '*'
     && buf[7] == '*' && buf[8] == '*' && buf[9] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "','" << buf[2]
     << "','" << buf[3] << "','" << buf[4] << "','" << buf[5] << "','" << buf[6]
     << "','" << buf[7] << "','" << buf[8] << "','" << buf[9] << "'");

  dsv_parser_destroy(parser);
}




#if 0
/** \test Check for default settings
 */
BOOST_AUTO_TEST_CASE( parser_default_object_settings )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_newline_behavior nl_behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(nl_behavior == dsv_newline_permissive,
    "Default parser newline behavior was not dsv_newline_permissive. Expected "
    << dsv_newline_permissive << " received " << nl_behavior);

  ssize_t field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == 0,
    "Default parser field columns was not '0' but rather '" << field_cols
    << "'");

  std::vector<unsigned char> buf(2,'*');
  std::size_t len = dsv_parser_get_field_delimiter(parser,buf.data(),
    buf.size());

  BOOST_REQUIRE_MESSAGE(buf[0] == ',' && buf[1] == '*',
    "Default parser delimiter was not ',' but instead the first " << len <<
    " bytes of '" << buf[0] << "','" << buf[1] << "'");
}
#endif


BOOST_AUTO_TEST_SUITE_END()

}
}
