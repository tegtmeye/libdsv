#include <boost/test/unit_test.hpp>

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


namespace fs=boost::filesystem;
namespace d=detail;



BOOST_AUTO_TEST_SUITE( RFC4180_permissive_parse_suite )


BOOST_AUTO_TEST_CASE( empty )
{
}

BOOST_AUTO_TEST_SUITE_END()

}
}
