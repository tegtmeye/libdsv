#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>

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
// namespace d=detail;


inline fs::path gen_testfile(const std::vector<unsigned char> &contents,
  const std::string &label)
{
  std::time_t now = std::time(0);

  fs::path tmpdir = fs::temp_directory_path();
  fs::path filename = std::string("libdsv_") + std::to_string(now) + "_"
    + label + ".dsv";
  fs::path filepath = tmpdir/filename;


  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    out(std::fopen(filepath.c_str(),"wb"),&std::fclose);

  assert(std::fwrite(contents.data(),sizeof(unsigned char),
    contents.size(),out.get()) == contents.size());

  return filepath;
}



BOOST_AUTO_TEST_SUITE( alt_delimiter_test_suite )


/**
    \test Check for proper handling of empty file by filename
 */
BOOST_AUTO_TEST_CASE( check_single_byte_test )
{
}


BOOST_AUTO_TEST_SUITE_END()

}
}
