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


BOOST_AUTO_TEST_SUITE_END()

}
}
