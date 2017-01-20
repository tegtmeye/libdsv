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

#include "test_detail.h"

#include <basic_scanner.h>

#include <boost/filesystem.hpp>

#include <errno.h>

#include <iostream>
#include <vector>
#include <regex>

/** \file
 *  \brief Unit tests the scanner
 */



namespace d=detail;

namespace dsv {
namespace test {


namespace fs=boost::filesystem;

typedef d::basic_scanner<char> scanner_type;
typedef d::basic_scanner_iterator<char> scanner_iterator;


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

  scanner_type scanner(filepath.c_str(),0);

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

  scanner_type scanner(filepath.c_str(),in.get());

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

  scanner_type scanner(0,in.get());

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

  scanner_type scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF for empty file");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: did not return 0 for empty file");

  // should not crash
  scanner.cache_erase(0);

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

  scanner_type scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(!scanner.eof(),
    "eof: scanner returned EOF when the stream did not pull from the file yet");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "getc: scanner did not return EOF for empty file");

  BOOST_REQUIRE_MESSAGE(scanner.eof(),
    "getc: scanner did not return EOF after accepting empty token and empty "
    "file");

  // should not crash
  scanner.cache_erase(0);

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

  scanner_type scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE(scanner.eof() == false);

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

  scanner_type scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for two char file");

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for two char file");

  BOOST_REQUIRE(scanner.eof() == false);

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

  scanner_type scanner(0,in.get());

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");

  BOOST_REQUIRE_MESSAGE(scanner.at_cache(0) == 'a',
    "at_cache: scanner did not return 'a' for single char file");

  BOOST_REQUIRE(scanner.eof() == false);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");

  BOOST_REQUIRE_MESSAGE(scanner.at_cache(0) == 'a',
    "at_cache: scanner did not return 'a' for single char file");

  scanner.cache_erase(1);

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  BOOST_REQUIRE(scanner.eof() == true);

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

  scanner_type scanner(0,in.get(),1);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'a',
    "getc: scanner did not return 'a' for multi char file");

  // buffer gets refilled here

  BOOST_REQUIRE_MESSAGE(scanner.getc() == 'b',
    "getc: scanner did not return 'b' for multi char file");

  BOOST_REQUIRE(scanner.eof() == false);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for multi char "
      "file");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 2,
    "cache_size: scanner did not return a cache_size of 2");

  BOOST_REQUIRE_MESSAGE(scanner.at_cache(0) == 'a',
    "at_cache: scanner did not return 'a' for single char file");

  BOOST_REQUIRE_MESSAGE(scanner.at_cache(1) == 'b',
    "at_cache: scanner did not return 'a' for single char file");

  scanner.cache_erase(1);

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");

  BOOST_REQUIRE(scanner.eof() == true);

  BOOST_REQUIRE_MESSAGE(scanner.getc() == EOF,
    "follow-on getc: scanner did not subsequently return EOF for single char "
      "file");

  fs::remove(filepath);
}

 /* CACHE CHECKS */

/**
    \test Check to see if read characters are constantly putback as requested.
 */
BOOST_AUTO_TEST_CASE( scanner_constant_cache_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_constant_cache_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get());

  int val;

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'a'");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");
  scanner.cache_erase(1);
  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'b'");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");
  scanner.cache_erase(1);
  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'c'");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");
  scanner.cache_erase(1);
  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "getc: unexpected '" << detail::ascii(val) << "', expected 'd'");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 1,
    "cache_size: scanner did not return a cache_size of 1");
  scanner.cache_erase(1);
  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  fs::remove(filepath);
}




/**
    \test Check to see if read characters are putback as requested. Read buffer
    is large enough to hold the entire file.
 */
BOOST_AUTO_TEST_CASE( scanner_cache_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_cache_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get());

  int val;

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

  BOOST_REQUIRE(scanner.eof() == false);

  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");


  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 4,
    "cache_size: scanner did not return a cache_size of 4");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.at_cache(3);
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'd'");


  scanner.cache_erase(0);


  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 4,
    "cache_size: scanner did not return a cache_size of 4");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'a',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'a'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.at_cache(3);
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'd'");


  scanner.cache_erase(1);


  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 3,
    "cache_size: scanner did not return a cache_size of 3");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'b',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'b'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'c',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'c'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'd'");


  scanner.cache_erase(3);


  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  BOOST_REQUIRE(scanner.eof() == true);

  // trigger EOF again
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "getc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  fs::remove(filepath);
}



/**
    \test Basic buffer checks
 */
/**
    \test Read 4 characters and refill. Checks for proper buffer refill
    behavior when need to preserve cache contents (compaction)
 */
BOOST_AUTO_TEST_CASE( scanner_putback_refill_compaction_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_putback_refill_compaction_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  int val;

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


  scanner.cache_erase(4);
  // front of buffer is now empty

  // start accumulating cache.
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

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 4,
    "cache_size: scanner did not return a cache_size of 4");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'e'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'f'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'g'");

  val=scanner.at_cache(3);
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'h'");


  // trigger refill, should just compact
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


  // check to see if cache was preserved

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 8,
    "cache_size: scanner did not return a cache_size of 8");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'e'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'f'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'g'");

  val=scanner.at_cache(3);
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'h'");

  val=scanner.at_cache(4);
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'i'");

  val=scanner.at_cache(5);
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'j'");

  val=scanner.at_cache(6);
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'k'");

  val=scanner.at_cache(7);
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'l'");

  BOOST_REQUIRE(scanner.eof() == false);


  // trigger EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");


  scanner.cache_erase(8);

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  fs::remove(filepath);
}



/**
    \test Read 4 characters and refill. Checks for proper buffer refill
    behavior when need to preserve cache contents (expand)
 */
BOOST_AUTO_TEST_CASE( scanner_putback_refill_expand_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "scanner_putback_refill_expand_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  int val;

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


  scanner.cache_erase(3);
  // front of buffer is now has a single char

  // start accumulating cache.
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

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 5,
    "cache_size: scanner did not return a cache_size of 5");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'd'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'e'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'f'");

  val=scanner.at_cache(3);
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'g'");

  val=scanner.at_cache(4);
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'h'");


  // trigger refill, should expand
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


  // check to see if cache was preserved

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 9,
    "cache_size: scanner did not return a cache_size of 9");

  val=scanner.at_cache(0);
  BOOST_REQUIRE_MESSAGE(val == 'd',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'd'");

  val=scanner.at_cache(1);
  BOOST_REQUIRE_MESSAGE(val == 'e',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'e'");

  val=scanner.at_cache(2);
  BOOST_REQUIRE_MESSAGE(val == 'f',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'f'");

  val=scanner.at_cache(3);
  BOOST_REQUIRE_MESSAGE(val == 'g',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'g'");

  val=scanner.at_cache(4);
  BOOST_REQUIRE_MESSAGE(val == 'h',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'h'");

  val=scanner.at_cache(5);
  BOOST_REQUIRE_MESSAGE(val == 'i',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'i'");

  val=scanner.at_cache(6);
  BOOST_REQUIRE_MESSAGE(val == 'j',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'j'");

  val=scanner.at_cache(7);
  BOOST_REQUIRE_MESSAGE(val == 'k',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'k'");

  val=scanner.at_cache(8);
  BOOST_REQUIRE_MESSAGE(val == 'l',
    "at_cache: unexpected '" << detail::ascii(val) << "', expected 'l'");

  BOOST_REQUIRE(scanner.eof() == false);


  // trigger EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");


  scanner.cache_erase(9);

  BOOST_REQUIRE_MESSAGE(scanner.cache_size() == 0,
    "cache_size: scanner did not return a cache_size of 0");

  // check to make sure still at EOF
  val=scanner.getc();
  BOOST_REQUIRE_MESSAGE(val == EOF,
    "fgetc: unexpected '" << detail::ascii(val) << "', expected 'EOF'");

  fs::remove(filepath);
}




/*  CHECK FOR ITERATOR HANDLING */

/**
    \test Check for basic iterator object functionality
 */
BOOST_AUTO_TEST_CASE( basic_scanner_iterator_object_test )
{
  std::vector<unsigned char> contents{
  };

  fs::path filepath = detail::gen_testfile(contents,
    "basic_scanner_iterator_object_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  scanner_iterator iter1(scanner);
  scanner_iterator iter2(scanner);

  BOOST_REQUIRE(iter1 == iter2);
  BOOST_REQUIRE(iter1 == scanner_iterator());
  BOOST_REQUIRE(scanner_iterator() == scanner_iterator());

  scanner_iterator iter1_copy(iter1);
  BOOST_REQUIRE(iter1_copy == iter2);
  BOOST_REQUIRE(iter1_copy == scanner_iterator());

  scanner_iterator iter2_copy;
  iter2_copy = iter2;
  BOOST_REQUIRE(iter2 == iter2_copy);
  BOOST_REQUIRE(iter1 == iter2_copy);
  BOOST_REQUIRE(iter2_copy == scanner_iterator());

  fs::remove(filepath);
}

/**
    \test Check for basic iterator object functionality
 */
BOOST_AUTO_TEST_CASE( basic_scanner_iterator_object_test2 )
{
  std::vector<unsigned char> contents{
    'a'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "basic_scanner_iterator_object_test2");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  scanner_iterator iter1(scanner);

  BOOST_REQUIRE(scanner.cache_size() == 1);
  BOOST_REQUIRE(!scanner.eof());

  scanner_iterator iter2(scanner);
  BOOST_REQUIRE(!scanner.eof());

  BOOST_REQUIRE(iter1 == iter2);
  BOOST_REQUIRE(iter1 != scanner_iterator());
  BOOST_REQUIRE(scanner_iterator() == scanner_iterator());

  scanner_iterator iter1_copy(iter1);
  BOOST_REQUIRE(iter1_copy == iter2);
  BOOST_REQUIRE(iter1_copy != scanner_iterator());

  scanner_iterator iter2_copy;
  iter2_copy = iter2;
  BOOST_REQUIRE(iter2 == iter2_copy);
  BOOST_REQUIRE(iter1 == iter2_copy);
  BOOST_REQUIRE(iter2_copy != scanner_iterator());

  fs::remove(filepath);
}

/**
    \test Check for basic iterator object traversal functionality
 */
BOOST_AUTO_TEST_CASE( basic_scanner_iterator_traversal_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "basic_scanner_iterator_traversal_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  scanner_iterator iter(scanner);

  std::vector<unsigned char>::iterator first = contents.begin();
  while(iter != scanner_iterator() && first != contents.end()) {
    BOOST_REQUIRE_MESSAGE(*iter == *first,
      "iterator: unexpected '" << detail::ascii(*iter) << "', expected '"
        << *first << "'");
    ++iter;
    ++first;
  }

  // incremented iter and therefore hit EOF
  BOOST_REQUIRE(first == contents.end());
  BOOST_REQUIRE(scanner.eof());
  BOOST_REQUIRE(iter == scanner_iterator());

  std::vector<unsigned char>::reverse_iterator rfirst = contents.rbegin();
  while(rfirst != contents.rend()) {
    BOOST_REQUIRE_MESSAGE(*(--iter) == *rfirst,
      "iterator: unexpected '" << detail::ascii(*iter) << "', expected '"
        << *rfirst << "'");

    BOOST_REQUIRE(iter != scanner_iterator());
    ++rfirst;
  }

  first = contents.begin();
  while(iter != scanner_iterator() && first != contents.end()) {
    BOOST_REQUIRE_MESSAGE(*iter == *first,
      "iterator: unexpected '" << detail::ascii(*iter) << "', expected '"
        << *first << "'");
    ++iter;
    ++first;
  }

  BOOST_REQUIRE(first == contents.end());
  BOOST_REQUIRE(scanner.eof());
  BOOST_REQUIRE(iter == scanner_iterator());

  fs::remove(filepath);
}

/**
    \test Check for basic iterator distance check
 */
BOOST_AUTO_TEST_CASE( basic_scanner_iterator_distance_test )
{
  std::vector<unsigned char> contents{
    'a','b','c','d','e','f','g','h','i','j','k','l'
  };

  fs::path filepath = detail::gen_testfile(contents,
    "basic_scanner_iterator_distance_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  scanner_iterator cur(scanner);
  scanner_iterator first(scanner);

  for(std::size_t i=0; i<10; ++i) {
    BOOST_REQUIRE(cur != scanner_iterator());
    BOOST_REQUIRE(*cur == contents[i]);
    ++cur;
  }

  BOOST_REQUIRE(cur != scanner_iterator());
  BOOST_REQUIRE(std::distance(first,cur) == 10);

  fs::remove(filepath);
}



/**
    \test Check for basic iterator regular expression compatability
 */
BOOST_AUTO_TEST_CASE( basic_scanner_iterator_regex_test )
{
  // regex example from cppreference.com
  std::string s = "Some people, when confronted with a problem, think "
      "\"I know, I'll use regular expressions.\" "
      "Now they have two problems.";

  std::vector<unsigned char> contents(s.begin(),s.end());

  fs::path filepath = detail::gen_testfile(contents,
    "basic_scanner_iterator_regex_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  scanner_iterator iter(scanner);

  std::regex expression("REGULAR EXPRESSIONS",
    std::regex_constants::ECMAScript | std::regex_constants::icase);

  auto search_begin =
      std::regex_iterator<scanner_iterator>(iter,scanner_iterator(),
        expression);
  auto search_end = std::regex_iterator<scanner_iterator>();

  BOOST_REQUIRE(std::distance(search_begin, search_end) == 1);

  std::match_results<scanner_iterator> match = *search_begin;

  BOOST_REQUIRE(match.size() == 1);
  std::string out(match[0].first,match[0].second);
  BOOST_REQUIRE(out == "regular expressions");

  fs::remove(filepath);
}

/**
    \test Check for complex iterator regular expression compatability
 */
BOOST_AUTO_TEST_CASE( basic_scanner_iterator_complex_regex_test )
{
  // regex example from cppreference.com
  std::string s = "foofoobar";

  std::vector<unsigned char> contents(s.begin(),s.end());

  fs::path filepath = detail::gen_testfile(contents,
    "basic_scanner_iterator_complex_regex_test");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  scanner_type scanner(0,in.get(),8);

  scanner_iterator first(scanner);

  std::regex expression("(foo)|(bar)",
    std::regex_constants::ECMAScript);

  auto search_begin =
      std::regex_iterator<scanner_iterator>(scanner_iterator(scanner),
        scanner_iterator(),expression);
  auto search_end = std::regex_iterator<scanner_iterator>();

  // 3 matches, "foo", "foo", and "bar"
  BOOST_REQUIRE(std::distance(search_begin, search_end) == 3);

  // match first "foo"
  std::match_results<scanner_iterator> match = *search_begin;

  std::size_t groups = match.size();
  BOOST_REQUIRE_MESSAGE(groups == 3,
    "Unexpected number of groups " << groups << " expected 3");
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[0].first) == 0);
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[0].second) == 3);
  std::string out(match[0].first,match[0].second);
  BOOST_REQUIRE(out == "foo");

  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[1].first) == 0);
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[1].second) == 3);
  out = std::string(match[1].first,match[1].second);
  BOOST_REQUIRE(out == "foo");

  BOOST_REQUIRE(match[2].first == scanner_iterator());
  BOOST_REQUIRE(match[2].second == scanner_iterator());

  // match second "foo"
  match = *(++search_begin);

  groups = match.size();
  BOOST_REQUIRE_MESSAGE(groups == 3,
    "Unexpected number of groups " << groups << " expected 3");
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[0].first) == 3);
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[0].second) == 6);
  out = std::string(match[0].first,match[0].second);
  BOOST_REQUIRE(out == "foo");

  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[1].first) == 3);
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[1].second) == 6);
  out = std::string(match[1].first,match[1].second);
  BOOST_REQUIRE(out == "foo");

  BOOST_REQUIRE(match[2].first == scanner_iterator());
  BOOST_REQUIRE(match[2].second == scanner_iterator());


  // match "bar"
  match = *(++search_begin);

  groups = match.size();
  BOOST_REQUIRE_MESSAGE(groups == 3,
    "Unexpected number of groups " << groups << " expected 3");
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[0].first) == 6);
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[0].second) == 9);
  out = std::string(match[0].first,match[0].second);
  BOOST_REQUIRE(out == "bar");

  BOOST_REQUIRE(match[1].first == scanner_iterator());
  BOOST_REQUIRE(match[1].second == scanner_iterator());

  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[2].first) == 6);
  BOOST_REQUIRE(std::distance(scanner_iterator(first),match[2].second) == 9);
  out = std::string(match[2].first,match[2].second);
  BOOST_REQUIRE(out == "bar");

  fs::remove(filepath);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
