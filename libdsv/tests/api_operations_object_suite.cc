#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"


#include <errno.h>
#include <stdio.h>

#include <string>

/** \file
 *  \brief Unit tests for operations creation
 */

namespace dsv {
namespace test {



BOOST_AUTO_TEST_SUITE( api_operations_object_test_suite )

/** \test Create operations object
 */
BOOST_AUTO_TEST_CASE( operations_create )
{
  dsv_operations_t operations;

  int result = dsv_operations_create(&operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_operations_create failed with exit code " << result);
}

/** \test Destroy initialized operations object
 */
BOOST_AUTO_TEST_CASE( initialized_operations_destroy )
{
  dsv_operations_t operations;

  int result = dsv_operations_create(&operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_operations_create failed with exit code " << result);

  dsv_operations_destroy(operations);
}

/** \test Create zeroed operations object
 */
BOOST_AUTO_TEST_CASE( zeroed_operations_create )
{
  dsv_operations_t operations = {};

  int result = dsv_operations_create(&operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_operations_create failed with exit code " << result);
}

/** \test Destroy initialized and zeroed operations object
 */
BOOST_AUTO_TEST_CASE( initialized_zeroed_operations_destroy )
{
  dsv_operations_t operations = {};

  int result = dsv_operations_create(&operations);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_operations_create failed with exit code " << result);

  dsv_operations_destroy(operations);
}



BOOST_AUTO_TEST_SUITE_END()

}
}
