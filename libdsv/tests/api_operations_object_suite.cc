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

#include <dsv_parser.h>
#include "api_detail.h"


#include <errno.h>
#include <stdio.h>

#include <string>

/** \file
 *  \brief Unit tests for operations creation
 */


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

  detail::file_context fcontext; // may throw
  dsv_set_header_callback(detail::header_callback,&fcontext,operations);

  fn = dsv_get_header_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == detail::header_callback,
    "dsv_get_header_callback does not return the just-set header callback");

  context = dsv_get_header_context(operations);
  BOOST_REQUIRE_MESSAGE(context == &fcontext,
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

  detail::file_context fcontext; // may throw
  dsv_set_record_callback(detail::record_callback,&fcontext,operations);

  fn = dsv_get_record_callback(operations);
  BOOST_REQUIRE_MESSAGE(fn == detail::record_callback,
    "dsv_get_record_callback does not return the just-set record callback");

  context = dsv_get_record_context(operations);
  BOOST_REQUIRE_MESSAGE(context == &fcontext,
    "dsv_get_record_context does not return the just-set record context");

  dsv_operations_destroy(operations);
}



BOOST_AUTO_TEST_SUITE_END()
