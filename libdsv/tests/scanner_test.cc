#include <boost/test/unit_test.hpp>

#include <scanner_state.h>

#include <boost/filesystem.hpp>

#include <errno.h>

#include <string>
#include <sstream>
#include <memory>
#include <cstdio>

#include <iostream>

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

inline std::string ascii(int c)
{
  std::stringstream out;

  if(c >= 32 && c <= 126)
    out << char(c);
  else
    out << c;

  return out.str();
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

  BOOST_REQUIRE_MESSAGE(scanner.fgetc() == EOF,
    "fgetc: scanner did not return EOF for empty file");

  BOOST_REQUIRE_MESSAGE(scanner.fgetc() == EOF,
    "fgetc: scanner did not return repeated EOF for empty file");

  scanner.forget();

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF after force forget and empty file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with single char with getc
 */
BOOST_AUTO_TEST_CASE( scanner_single_getc_test )
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

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with single char with fgetc
 */
BOOST_AUTO_TEST_CASE( scanner_single_fgetc_test )
{
  std::vector<unsigned char> contents{
    'a'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_single_fgetc_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.fgetc() == 'a',
    "fgetc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.fgetc() == EOF,
    "follow-on fgetc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with two chars with getc
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_getc_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_getc_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for two char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for two char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for two char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with two chars with fgetc
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_fgetc_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_fgetc_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.fgetc() == 'a',
    "fgetc: scanner did not return 'a' for two char file");

  BOOST_REQUIRE_MESSAGE(scanner.fgetc() == 'b',
    "fgetc: scanner did not return 'b' for two char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on fgetc: scanner did not subsequently return EOF for two char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with single char with getc and putback
    handling
 */
BOOST_AUTO_TEST_CASE( scanner_single_getc_putback_test )
{
  std::vector<unsigned char> contents{
    'a'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_single_getc_putback_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  scanner.putback();

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return a putback 'a' for single char file");

  scanner.putback();

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return a putback 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}


/* CHECK FOR BUFFER REFILL HANDLING */

/**
    \test Check for read of file with two chars with getc. The initial
    buffer only has room for 1 char before refilling
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_refill_getc_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_minimal_refill_getc_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),1);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for multi char file");

  // must forget the putback buffer because otherwise the character in the
  // putback buffer will keep the single buffer full and nothing will be read.
  scanner.forget();

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for multi char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for multi char "
      "file");

  fs::remove(filepath);
}

/**
    \test Check for read of file with two chars with fgetc. The initial
    buffer only has room for 1 char before refilling
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_refill_fgetc_test )
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

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << ascii(val) << "', expected 'EOF'");

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

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "advancec: unexpected '" << ascii(val) << "', expected 'a'");

  scanner.putback();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << ascii(val) << "', expected 'd'");

  // putback entire buffer, did not trigger EOF
  scanner.putback();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << ascii(val) << "', expected 'd'");

  // trigger EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << ascii(val) << "', expected 'EOF'");

  // putback entire buffer, no longer EOF
  scanner.putback();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << ascii(val) << "', expected 'd'");

  // trigger EOF again
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << ascii(val) << "', expected 'EOF'");

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

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "fgetc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "fgetc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "fgetc: unexpected '" << ascii(val) << "', expected 'c'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "fgetc: unexpected '" << ascii(val) << "', expected 'd'");

  // start accumulating putback buffer. This leaves a hole in the front of
  // the read buffer
  scanner.forget();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "getc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "getc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "getc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "getc: unexpected '" << ascii(val) << "', expected 'h'");

  // trigger refill
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "getc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "getc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "getc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "getc: unexpected '" << ascii(val) << "', expected 'l'");


  // putback the entire buffer
  scanner.putback();

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "fgetc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "fgetc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "fgetc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "fgetc: unexpected '" << ascii(val) << "', expected 'h'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "fgetc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "fgetc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "fgetc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "fgetc: unexpected '" << ascii(val) << "', expected 'l'");

  // trigger EOF
  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << ascii(val) << "', expected 'EOF'");

  // Should do nothing
  scanner.putback();

  // check to make sure still at EOF
  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << ascii(val) << "', expected 'EOF'");


  fs::remove(filepath);
}




// CHECK FOR MARK HANDLING


/**
    \test Check for putback and mark setting
 */
BOOST_AUTO_TEST_CASE( scanner_putmarkback_setting_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_putmarkback_refill_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),8);

  int val;

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "fgetc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "fgetc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "fgetc: unexpected '" << ascii(val) << "', expected 'c'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "fgetc: unexpected '" << ascii(val) << "', expected 'd'");

  // start accumulating putback buffer. This leaves a hole in the front of
  // the read buffer
  scanner.forget();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "getc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "getc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "getc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "getc: unexpected '" << ascii(val) << "', expected 'h'");

  scanner.setmark();

  // trigger refill
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "getc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "getc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "getc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "getc: unexpected '" << ascii(val) << "', expected 'l'");

  // putback entire buffer. This should invalidate the mark buffer
  scanner.putback();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "getc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "getc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "getc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "getc: unexpected '" << ascii(val) << "', expected 'h'");

  // putback to mark, which now the front of the putback buffer
  scanner.putbackmark();

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "fgetc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "fgetc: unexpected '" << ascii(val) << "', expected 'f'");

  // Should just putback to 'f'
  scanner.putbackmark();

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "fgetc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "fgetc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "fgetc: unexpected '" << ascii(val) << "', expected 'h'");

  // Should putback to 'h' since previous are forgotten
  scanner.putback();

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "fgetc: unexpected '" << ascii(val) << "', expected 'h'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "fgetc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "fgetc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "fgetc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "fgetc: unexpected '" << ascii(val) << "', expected 'l'");


  // check to make sure still at EOF
  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << ascii(val) << "', expected 'EOF'");


  fs::remove(filepath);
}

/**
    \test
 */
BOOST_AUTO_TEST_CASE( scanner_putmarkback_refill_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = gen_testfile(contents,
    "scanner_putmarkback_refill_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),8);

  int val;

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "fgetc: unexpected '" << ascii(val) << "', expected 'a'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "fgetc: unexpected '" << ascii(val) << "', expected 'b'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "fgetc: unexpected '" << ascii(val) << "', expected 'c'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "fgetc: unexpected '" << ascii(val) << "', expected 'd'");

  // start accumulating putback buffer. This leaves a hole in the front of
  // the read buffer
  scanner.forget();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "getc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "getc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "getc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "getc: unexpected '" << ascii(val) << "', expected 'h'");

  scanner.setmark();

  // trigger refill
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "getc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "getc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "getc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "getc: unexpected '" << ascii(val) << "', expected 'l'");


  // putback the mark buffer
  scanner.putbackmark();

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "fgetc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "fgetc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "fgetc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "fgetc: unexpected '" << ascii(val) << "', expected 'l'");

  scanner.putback();

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "fgetc: unexpected '" << ascii(val) << "', expected 'e'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "fgetc: unexpected '" << ascii(val) << "', expected 'f'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "fgetc: unexpected '" << ascii(val) << "', expected 'g'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "fgetc: unexpected '" << ascii(val) << "', expected 'h'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "fgetc: unexpected '" << ascii(val) << "', expected 'i'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "fgetc: unexpected '" << ascii(val) << "', expected 'j'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "fgetc: unexpected '" << ascii(val) << "', expected 'k'");

  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "fgetc: unexpected '" << ascii(val) << "', expected 'l'");

  // trigger EOF
  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << ascii(val) << "', expected 'EOF'");

  // Should do nothing
  scanner.putback();

  // check to make sure still at EOF
  val=scanner.fgetc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << ascii(val) << "', expected 'EOF'");


  fs::remove(filepath);
}







BOOST_AUTO_TEST_SUITE_END()

}
}
