#include <boost/test/unit_test.hpp>

#include "test_detail.h"

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



namespace d=detail;

namespace dsv {
namespace test {


namespace fs=boost::filesystem;



BOOST_AUTO_TEST_SUITE( scanner_test_suite )


/**
    \test Check for proper handling of empty file by filename
 */
BOOST_AUTO_TEST_CASE( scanner_basic_create_filename_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = detail::gen_testfile(contents,
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

  fs::path filepath = detail::gen_testfile(contents,
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

  fs::path filepath = detail::gen_testfile(contents,
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

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_empty_eof_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF for empty file");

  scanner.accept();

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF after accepting empty token and empty "
    "file");

  fs::remove(filepath);
}

/**
    \test Check for proper handling of empty file by stream only using
    alternative calls
 */
BOOST_AUTO_TEST_CASE( scanner_empty_eof_fn_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_empty_eof_fn_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(!scanner.eof(),
    "eof: scanner returned EOF when the stream did not pull from the file yet");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF for empty file");

  BOOST_REQUIRE_MESSAGE(scanner.eof(),
    "getc: scanner did not return EOF after accepting empty token and empty "
    "file");

  scanner.accept();

  BOOST_REQUIRE_MESSAGE(scanner.eof(),
    "getc: scanner did not return EOF after accepting empty token and empty "
    "file");

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

  fs::path filepath = detail::gen_testfile(contents,
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
    \test Check for read of file with two chars with getc
 */
BOOST_AUTO_TEST_CASE( scanner_minimal_getc_test )
{
  std::vector<unsigned char> contents{
    'a','b'
  };

  fs::path filepath = detail::gen_testfile(contents,
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
    \test Check for read of file with single char with getc and putback
    handling
 */
BOOST_AUTO_TEST_CASE( scanner_single_getc_putback_test )
{
  std::vector<unsigned char> contents{
    'a'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_single_getc_putback_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  scanner.putback(1);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return a putback 'a' for single char file");

  scanner.putback(1);

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

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_minimal_refill_getc_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),1);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for multi char file");

  // buffer gets refilled here

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for multi char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for multi char "
      "file");

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

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_putback_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  int val;

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  scanner.putback(1);

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'd'");

  // putback entire buffer, did not trigger EOF
  scanner.putback(4);

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'd'");

  // trigger EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  // putback entire buffer, no longer EOF
  scanner.putback(4);

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'd'");

  // trigger EOF again
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  fs::remove(filepath);
}


/**
    \test Basic buffer checks
 */
BOOST_AUTO_TEST_CASE( scanner_basic_buffer_test )
{
  typedef d::scanner_state::const_iterator const_iterator;

  std::vector<unsigned char> contents{
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_basic_buffer_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get());

  std::pair<const_iterator,const_iterator> token = scanner.token();
  BOOST_REQUIRE(token.first == token.second);

  scanner.accept();

  token = scanner.token();
  BOOST_REQUIRE(token.first == token.second);
}

/**
    \test Read 4 characters and refill. Checks for proper buffer refill
    behavior when need to preserve contents of token and lookahead buffer
 */
BOOST_AUTO_TEST_CASE( scanner_putback_refill_test )
{
  typedef d::scanner_state::const_iterator const_iterator;

  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_putback_refill_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),8);

  int val;

  std::pair<const_iterator,const_iterator> token = scanner.token();
  BOOST_REQUIRE(token.first == token.second);


  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'd'");

  // set the putback and accept the empty token
  // read location now points to 'e'
  // token buffer points to a -> d
  scanner.set_lookahead(42);
  BOOST_REQUIRE(scanner.accept() == 42);

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'a');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'b');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'c');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'd');
  BOOST_REQUIRE(token.first == token.second);


  // start accumulating putback buffer.
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'e'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'f'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'g'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'h'");

  // set the putback buffer. The token buffer contains a-d and the lookahead
  // buffer now contains e-h
  scanner.set_lookahead(15);

  // empty the token buffer and set to the lookahead buffer: token contains
  // e-h and the putback buffer is empty. This leaves a hole in the front of
  // the read buffer
  BOOST_REQUIRE(scanner.accept() == 15);

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'e');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'f');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'g');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'h');
  BOOST_REQUIRE(token.first == token.second);


  // trigger refill
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'l'");

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'e');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'f');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'g');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'h');
  BOOST_REQUIRE(token.first == token.second);

  // putback the buffer
  scanner.putback(4);

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'l'");

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'e');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'f');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'g');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'h');
  BOOST_REQUIRE(token.first == token.second);

  // trigger EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  // token buffer contains e-h, lookahead contains i-l
  scanner.set_lookahead();

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  fs::remove(filepath);
}





// CHECK FOR MARK HANDLING


/**
    \test Check for putback and mark setting
 */
BOOST_AUTO_TEST_CASE( scanner_putmarkback_setting_test )
{
  typedef d::scanner_state::const_iterator const_iterator;

  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_putmarkback_refill_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  d::scanner_state scanner(0,in.get(),8);

  int val;

  std::pair<const_iterator,const_iterator> token = scanner.token();
  BOOST_REQUIRE(token.first == token.second);

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'd'");

  // set the putback and accept the empty token
  // read location now points to 'e'
  // token buffer points to a -> d
  scanner.set_lookahead(42);
  BOOST_REQUIRE(scanner.accept() == 42);

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'a');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'b');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'c');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'd');
  BOOST_REQUIRE(token.first == token.second);

  // start accumulating putback buffer.
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'e'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'f'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'g'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'h'");

  // set the putback buffer. Now the token buffer contains a-d and the
  // lookahead buffer contains e-h
  scanner.set_lookahead(15);


  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'a');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'b');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'c');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'd');
  BOOST_REQUIRE(token.first == token.second);


  // trigger refill
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'l'");

  // putback buffer. This should not invalidate any other buffers
  scanner.putback(4);

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'a');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'b');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'c');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'd');
  BOOST_REQUIRE(token.first == token.second);

  // re-read
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'i'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'j'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'k'");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'l'");

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'a');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'b');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'c');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'd');
  BOOST_REQUIRE(token.first == token.second);

  // Accept the token. Now the token buffer should read e-h and the lookahead
  // contains i-l
  BOOST_REQUIRE(scanner.accept() == 15);

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'e');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'f');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'g');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'h');
  BOOST_REQUIRE(token.first == token.second);

  // reset putback
  scanner.set_lookahead(0);

  // check to make sure at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  // Accept the token. Now the token buffer should read i-l and the lookahead
  // contains nothing
  BOOST_REQUIRE(scanner.accept() == 0);

  token = scanner.token();
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'i');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'j');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'k');
  BOOST_REQUIRE(token.first != token.second && *token.first++ == 'l');
  BOOST_REQUIRE(token.first == token.second);

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  // Accept the token. Now the token buffer contains nothing
  BOOST_REQUIRE(scanner.accept() == 0);

  BOOST_REQUIRE(token.first == token.second);

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  // should do nothing
  scanner.set_lookahead(15);

  BOOST_REQUIRE(token.first == token.second);

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  // Should do nothing but we explicitly set identifier
  BOOST_REQUIRE(scanner.accept() == 15);

  BOOST_REQUIRE(token.first == token.second);

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  fs::remove(filepath);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
