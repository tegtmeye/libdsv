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

/**
    \file This file is used to provide convenience functions across test cases
    only put functions that do not rely on having a built dsv library in here
 */

#ifndef LIBDSV_TEST_TEST_DETAIL_H
#define LIBDSV_TEST_TEST_DETAIL_H


#ifndef TESTDATA_DIR
#error TESTDATA_DIR not defined
#endif

#ifndef RANDOM_DATA_FILENAME
#define RANDOM_DATA_FILENAME random_data_file.dat
#endif

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#include <iostream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <vector>


#include <boost/filesystem.hpp>

namespace detail {


namespace fs=boost::filesystem;


typedef std::vector<char> field_storage_type;


static const std::string testdatadir(QUOTEME(TESTDATA_DIR));
static const std::string random_data_file(QUOTEME(RANDOM_DATA_FILENAME));

static const std::string missing_random_data_file_msg =
  "Missing random data file '" QUOTEME(RANDOM_DATA_FILENAME) "'. You can "
  "generate a new one by compiling and running '" QUOTEME(RANDOM_DATA_FILENAME)
  ".cc' in this directory.";

// 0x0A = LF
// 0x0D = CR
// 0x22 = "
// 0x27 = '
// 0x5C = backslash

// 94 characters not including newline
static const field_storage_type rfc4180_charset = {
  ' ','!','#','$','%','&',0x27,'(',')','*','+','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G',
  'H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '[',0x5C,']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m',
  'n','o','p','q','r','s','t','u','v','w','x','y','z','{','|','}','~'
};

static const field_storage_type rfc4180_quoted_charset = {
  ' ','!',0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/','0','1','2',
  '3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E',
  'F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X',
  'Y','Z','[',0x5C,']','^','_','`','a','b','c','d','e','f','g','h','i','j','k',
  'l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',
  0x0D,0x0A
};

// same as above but embedded linefeed instead of carriage-return line-feed
static const field_storage_type rfc4180_lf_quoted_charset = {
  ' ','!',0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/','0','1','2',
  '3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E',
  'F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X',
  'Y','Z','[',0x5C,']','^','_','`','a','b','c','d','e','f','g','h','i','j','k',
  'l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',
  0x0A
};

// 100 characters not including newline
static const field_storage_type rfc4180_raw_quoted_charset = {
  0x22,' ','!',0x22,0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B',
  'C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
  'V','W','X','Y','Z','[',0x5C,']','^','_','`','a','b','c','d','e','f','g','h',
  'i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{',
  '|','}','~',0x0D,0x0A,0x22
};

// same as above but embedded linefeed instead of carriage-return line-feed
static const field_storage_type rfc4180_lf_raw_quoted_charset = {
  0x22,' ','!',0x22,0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/',
  '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B',
  'C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
  'V','W','X','Y','Z','[',0x5C,']','^','_','`','a','b','c','d','e','f','g','h',
  'i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{',
  '|','}','~',0x0A,0x22
};

// same as rfc4180 charset but without whitespace chars, 93 characters
static const field_storage_type sub_rfc4180_charset = {
  '!','#','$','%','&',0x27,'(',')','*','+','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G',
  'H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '[',0x5C,']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m',
  'n','o','p','q','r','s','t','u','v','w','x','y','z','{','|','}','~'
};

static const field_storage_type crlf = {0x0D,0x0A};

static const field_storage_type lf = {0x0A};

static const field_storage_type comma = {0x2C};

static const field_storage_type empty;

static const unsigned char space = ' ';
static const unsigned char tab = '\t';
static const unsigned char underscore = '_';



template<typename CharT>
std::basic_string<CharT> to_string(const std::vector<CharT> &buf)
{
  std::basic_stringstream<CharT> out;

  for(std::size_t i=0; i<buf.size(); ++i) {
    if(buf[i] > 32 && buf[i] < 126)
      out << buf[i];
    else
      out << "0x" << std::hex << std::showbase << std::setw(2)
        << std::setfill('0') << int(buf[i]);
  }

  return out.str();
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


inline fs::path gen_testfile(const std::vector<field_storage_type> &contents,
  const std::string &label)
{
  std::time_t now = std::time(0);

  fs::path tmpdir = fs::temp_directory_path();
  fs::path filename = std::string("libdsv_") + std::to_string(now) + "_"
    + label + ".dsv";
  fs::path filepath = tmpdir/filename;


  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    out(std::fopen(filepath.c_str(),"wb"),&std::fclose);

  for(std::size_t i=0; i<contents.size(); ++i) {
    assert(std::fwrite(contents[i].data(),sizeof(unsigned char),
      contents[i].size(),out.get()) == contents[i].size());
  }

  return filepath;
}


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


template<typename T>
std::basic_string<T> print_byte(T byte)
{
  std::basic_stringstream<T> out;

  if(byte < 32 || byte > 126)
    out << int(byte);
  else
    out << char(byte);

  return out.str();
}


}


// In namespace std due to overload
namespace std {

template<typename CharT>
std::basic_ostream<CharT> & operator<<(std::basic_ostream<CharT> &out,
  const std::vector<CharT> &seq)
{
  typename std::vector<CharT>::const_iterator cur = seq.begin();

  while(cur != seq.end()) {
    if(*cur < 32 || *cur > 126)
      out << int(*cur);
    else
      out << char(*cur);

    ++cur;
  }

  return out;
}

}
#endif
