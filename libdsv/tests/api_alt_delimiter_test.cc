#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"

#include <errno.h>

#include <string>
#include <sstream>
#include <memory>
#include <cstdio>

/** \file
 *  \brief Unit tests the scanner
 */




namespace dsv {
namespace test {


namespace fs=boost::filesystem;
namespace d=detail;


BOOST_AUTO_TEST_SUITE( alt_delimiter_test_suite )


/**
    \test Check for proper handling of empty file by filename
 */
BOOST_AUTO_TEST_CASE( check_single_byte_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

}


BOOST_AUTO_TEST_SUITE_END()

}
}
