#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"



/** \file
 *  \brief Unit tests for the default settings for the different parsers
 */

namespace dsv {
namespace test {



BOOST_AUTO_TEST_SUITE( api_parser_defaults_suite )

/**
    \test Basic parser object checks. Checks low API getting and setting
    behavior only. Does not check for proper operation. For add checks of
    proper operation, place them into the appropriate api_XXX check file.

    These unit tests are scheduled before checks of the logging functionality.
    That is, do not place unit tests here that require logging functionality.

 */

/**
    \test Create and destroy uninitialized parser object
 */
BOOST_AUTO_TEST_CASE( parser_create_and_destroy )
{
  dsv_parser_t parser;
  int result = dsv_parser_create(&parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_create failed with exit code: " << result);

  dsv_parser_destroy(parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_destroy failed with exit code: " << result);
}






BOOST_AUTO_TEST_SUITE_END()

}
}
