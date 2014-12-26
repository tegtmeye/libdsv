#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>

//#include <libatchdf.h>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <errno.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>

/** \file
 *  \brief Unit tests for creation
 */

namespace dsv {
namespace test {

namespace b = boost;
namespace fs = boost::filesystem;

namespace detail {


}


BOOST_AUTO_TEST_SUITE( create_suite )

/** \test Attempt to create
 */
BOOST_AUTO_TEST_CASE( parser_create )
{
//   b::shared_ptr<const char> filename(tempnam(0,"atch"),free);
//
//   atchdf_t doc_handle;
//
//   int result = atchdf_file_open(filename.get(),0,&doc_handle);
//
//   BOOST_CHECK_EQUAL(result,EINVAL);
//
//   BOOST_REQUIRE_MESSAGE(!fs::exists(filename.get()),
//     "atchdf_file_open created the file '"
//       << filename.get() << "' when it shouldn't have");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
