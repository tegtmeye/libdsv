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



BOOST_AUTO_TEST_SUITE( api_operations_object_suite )

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

/** \test Getting and setting of header operations members
 */
BOOST_AUTO_TEST_CASE( header_getting_and_setting )
{
  dsv_operations_t operations = {};

  int err = dsv_operations_create(&operations);
  BOOST_REQUIRE_MESSAGE(err == 0,"dsv_operations_create succeeds");

  header_callback_t fn = dsv_get_header_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == 0,
    "dsv_get_header_callback returns nonzero for an unset header callback");

  void *context = dsv_get_header_context(operations);
  BOOST_REQUIRE_MESSAGE(context == 0,
    "dsv_get_header_context returns nonzero for an unset header context");

  detail::field_context header_context; // may throw

  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  fn = dsv_get_header_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == detail::field_callback,
    "dsv_get_header_callback does not return the just-set header callback");

  context = dsv_get_header_context(operations);
  BOOST_REQUIRE_MESSAGE(context == &header_context,
    "dsv_get_header_context does not return the just-set header context");

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
    "dsv_get_record_callback returns nonzero for an unset record callback");

  void *context = dsv_get_record_context(operations);
  BOOST_REQUIRE_MESSAGE(context == 0,
    "dsv_get_record_context returns nonzero for an unset record context");

  detail::field_context record_context; // may throw

  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  fn = dsv_get_record_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == detail::field_callback,
    "dsv_get_record_callback does not return the just-set record callback");

  context = dsv_get_record_context(operations);
  BOOST_REQUIRE_MESSAGE(context == &record_context,
    "dsv_get_record_context does not return the just-set record context");

  dsv_operations_destroy(operations);
}



BOOST_AUTO_TEST_SUITE_END()

}
}
