#include <boost/test/unit_test.hpp>

#include <scanner_state.h>

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
namespace d=detail;


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



BOOST_AUTO_TEST_SUITE( scanner_test_suite )


/**
    \test Check for proper handling of empty file by filename
 */
BOOST_AUTO_TEST_CASE( scanner_basic_create_filename_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_basic_create_filename_test");

  d::scanner_state scanner(filepath.c_str(),0);

  fs::remove(filepath);
}

/**
    \test Check for proper handling of empty file by stream with filename
 */
BOOST_AUTO_TEST_CASE( scanner_basic_create_filename_stream_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_basic_create_filename_stream_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(filepath.c_str(),in.get());

  fs::remove(filepath);
}

/**
    \test Check for proper handling of empty file by stream only
 */
BOOST_AUTO_TEST_CASE( scanner_basic_create_stream_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_basic_create_stream_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  fs::remove(filepath);
}

/**
    \test Check for proper handling of empty file by stream only
 */
BOOST_AUTO_TEST_CASE( scanner_empty_eof_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_empty_eof_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF for empty file");

  BOOST_REQUIRE_MESSAGE(scanner.advancec() == EOF,
    "advancec: scanner did not return EOF for empty file");

  BOOST_REQUIRE_MESSAGE(scanner.fadvancec() == EOF,
    "fadvancec: scanner did not return EOF for empty file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with single char with getc/advance
 */
BOOST_AUTO_TEST_CASE( scanner_single_getc_advancec_test )
{
  std::vector<unsigned char> contents{
    'a'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_single_getc_advancec_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.advancec() == 'a',
    "advancec: scanner did not return EOF for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with single char with fadvance
 */
BOOST_AUTO_TEST_CASE( scanner_single_getc_fadvancec_test )
{
  std::vector<unsigned char> contents{
    'a'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_single_getc_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.fadvancec() == 'a',
    "fadvancec: scanner did not return EOF for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with two chars with getc/advancec
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_getc_advancec_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_getc_advancec_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.advancec() == 'a',
    "advancec: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.advancec() == 'b',
    "advancec: scanner did not return 'b' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with two chars with getc/advancec
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_getc_fadvancec_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_getc_fadvancec_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.fadvancec() == 'a',
    "fadvancec: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.fadvancec() == 'b',
    "fadvancec: scanner did not return 'b' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}


/* CHECK FOR BUFFER REFILL HANDLING */

/**
    \test Check for read of file with two chars with getc/advancec. The initial
    buffer only has room for 1 char before refilling
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_refill_getc_advancec_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_refill_getc_advancec_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),1);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.advancec() == 'a',
    "advancec: scanner did not return 'a' for single char file");

  scanner.forget(); // putback buffer should be empty now

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.advancec() == 'b',
    "advancec: scanner did not return 'b' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with two chars with getc/fadvancec. The initial
    buffer only has room for 1 char before refilling
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_refill_getc_fadvancec_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_refill_getc_fadvancec_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),1);

  int val;

  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == 'a',
    "getc: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'a',
    "fadvancec: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == 'b',
    "getc: unexpected '" << val << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'b',
    "advancec: unexpected '" << val << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == EOF,
    "getc: unexpected '" << val << "', expected 'EOF'");

  fs::remove(filepath);
}


 /* PUTBACK CHECKS */

/**
    \test Check to see if read characters are putback as requested. Read buffer
    is large enough to hold the entire file.
 */
BOOST_AUTO_TEST_CASE( scanner_putback_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_putback_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  int val;

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'a',
    "advancec: unexpected '" << val << "', expected 'a'");

  scanner.putback();

  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == 'a',
    "getc: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'a',
    "advancec: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'b',
    "advancec: unexpected '" << val << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'c',
    "advancec: unexpected '" << val << "', expected 'c'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'd',
    "advancec: unexpected '" << val << "', expected 'd'");

  // putback entire buffer, did not trigger EOF
  scanner.putback();

  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == 'a',
    "getc: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'a',
    "advancec: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'b',
    "advancec: unexpected '" << val << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'c',
    "advancec: unexpected '" << val << "', expected 'c'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'd',
    "advancec: unexpected '" << val << "', expected 'd'");

  // trigger EOF
  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == EOF,
    "getc: unexpected '" << val << "', expected 'EOF'");

  // putback entire buffer, no longer EOF
  scanner.putback();

  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == 'a',
    "getc: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'a',
    "advancec: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'b',
    "advancec: unexpected '" << val << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'c',
    "advancec: unexpected '" << val << "', expected 'c'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'd',
    "advancec: unexpected '" << val << "', expected 'd'");

  // trigger EOF again
  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == EOF,
    "getc: unexpected '" << val << "', expected 'EOF'");

  fs::remove(filepath);
}



/**
    \test Read 4 characters into the putback buffer and refill. putback the
    initial 4 characters and continue reading
 */
BOOST_AUTO_TEST_CASE( scanner_putback_refill_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_putback_refill_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),8);

  int val;

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'a',
    "fadvancec: unexpected '" << val << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'b',
    "fadvancec: unexpected '" << val << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'c',
    "fadvancec: unexpected '" << val << "', expected 'c'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'd',
    "fadvancec: unexpected '" << val << "', expected 'd'");

  // start accumulating putback buffer. This leaves a hole in the front of
  // the read buffer
  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'e',
    "advancec: unexpected '" << val << "', expected 'e'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'f',
    "advancec: unexpected '" << val << "', expected 'f'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'g',
    "advancec: unexpected '" << val << "', expected 'g'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'h',
    "advancec: unexpected '" << val << "', expected 'h'");


  // trigger refill
  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'i',
    "advancec: unexpected '" << val << "', expected 'i'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'j',
    "advancec: unexpected '" << val << "', expected 'j'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'k',
    "advancec: unexpected '" << val << "', expected 'k'");

  BOOST_REQUIRE_MESSAGE((val=scanner.advancec()) == 'l',
    "advancec: unexpected '" << val << "', expected 'l'");


  // putback the entire buffer
  scanner.putback();

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'e',
    "fadvancec: unexpected '" << val << "', expected 'e'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'f',
    "fadvancec: unexpected '" << val << "', expected 'f'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'g',
    "fadvancec: unexpected '" << val << "', expected 'g'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'h',
    "fadvancec: unexpected '" << val << "', expected 'h'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'i',
    "fadvancec: unexpected '" << val << "', expected 'i'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'j',
    "fadvancec: unexpected '" << val << "', expected 'j'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'k',
    "fadvancec: unexpected '" << val << "', expected 'k'");

  BOOST_REQUIRE_MESSAGE((val=scanner.fadvancec()) == 'l',
    "fadvancec: unexpected '" << val << "', expected 'l'");

  // trigger EOF
  BOOST_REQUIRE_MESSAGE((val=scanner.getc()) == EOF,
    "getc: unexpected '" << val << "', expected 'EOF'");

  fs::remove(filepath);
}









BOOST_AUTO_TEST_SUITE_END()

}
}
