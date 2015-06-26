/*
 Copyright (c) 2014, Mike Tegtmeyer
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \file This file is used to provide convenience functions across test cases
 */

#ifndef LIBDSV_TEST_TEST_DETAIL_H
#define LIBDSV_TEST_TEST_DETAIL_H


#ifndef TESTDATA_DIR
#error TESTDATA_DIR not defined
#endif

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#include <iostream>

namespace dsv {
namespace test {


namespace detail {


static const std::string testdatadir(QUOTEME(TESTDATA_DIR));


// 0x0A = LF
// 0x0D = CR
// 0x22 = "
// 0x27 = '
// 0x5C = backslash

const char rfc4180_charset_field[] = {
  ' ','!','#','$','%','&',0x27,'(',')','*','+','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J',
  'K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',0x5C,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u',
  'v','w','x','y','z','{','|','}','~'
};

const char rfc4180_quoted_charset_field[] = {
  ' ','!',0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J',
  'K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',0x5C,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u',
  'v','w','x','y','z','{','|','}','~',0x0D,0x0A
};



inline void parser_destroy(dsv_parser_t *p)
{
  dsv_parser_destroy(*p);
}

inline void operations_destroy(dsv_operations_t *p)
{
  dsv_operations_destroy(*p);
}

inline const std::string & format_str(dsv_log_code msg_code)
{
  switch(msg_code) {
    case dsv_syntax_error:
      static const std::string parse_error("syntax error: In file $4, line $1 column $3");
      return parse_error;

    case dsv_column_count_error:
      static const std::string column_count_error("column count error: In file $4, line $1."
        " Expected $2 fields, read $3 fields");
      return column_count_error;

    default:
      ;
  };

  static const std::string unknown_error("unknown error code");
  return unknown_error;
}

std::string msg_log(dsv_parser_t parser, dsv_log_level log_level)
{
  std::stringstream result;

  // Get the number of log messages for \c log_level
  size_t num_msgs = dsv_parse_log_count(parser,log_level);

  // iterate over all codes
  for(size_t i=0; i<num_msgs; ++i) {
    // Get the code and minimum storage requirement for the msg parameters
    dsv_log_code msg_code;

    errno = 0;
    ssize_t len = dsv_parse_log(parser,log_level,i,&msg_code,0,0);
    if(len < 0) {
      std::stringstream err;
      err << "dsv_parse_error message code get failed with errno " << errno;
      throw std::runtime_error(err.str());
    }

    // msg_code now holds the code associated with the msg
    // len now holds the storage needed for the the total number of parameters
    // associated with msg_code.

    const std::string &fmt_str = format_str(msg_code);

    // Allocate buf based on minimum needed to store the parameters and the
    // length needed to store the format string potentially trimmed of the
    // placeholders. Do not forget to add space for the null terminator
    size_t storage_len = (fmt_str.size()+len+1);
    std::unique_ptr<char> buff(new char[storage_len]);

    // copy the format string to buff and set the null-terminator
    *(std::copy(fmt_str.begin(),fmt_str.end(),buff.get())) = 0;

    errno = 0;
    
//     std::cerr << "attempting to copy in the formatted string\n";
    len = dsv_parse_log(parser,log_level,i,&msg_code,buff.get(),storage_len);
    if(len < 0) {
      std::stringstream err;
      err << "dsv_parse_error message formatter failed with errno " << errno;
      throw std::runtime_error(err.str());
    }

    result << buff.get();
  }

  return result.str();
}

bool msg_log_check(dsv_parser_t parser, const std::vector<dsv_log_code> &_code_vec)
{
  std::vector<dsv_log_code> code_vec = _code_vec;
  
  size_t num_msgs = dsv_parse_log_count(parser,dsv_log_all);
  
  if(num_msgs < code_vec.size()) {
    std::stringstream err;
    err << "ERROR: Parser logged less codes than required. Saw " << num_msgs
      << " required " << code_vec.size() << "\n";
    return false;
  }
  
  std::vector<dsv_log_code> logged_code_vec;
  for(std::size_t i=0; i<num_msgs; ++i) {
    // Get the code and minimum storage requirement for the msg parameters
    dsv_log_code msg_code;

    errno = 0;
    ssize_t len = dsv_parse_log(parser,dsv_log_all,i,&msg_code,0,0);
    if(len < 0) {
      std::stringstream err;
      err << "UNIT TEST FAILURE: dsv_parse_error message code get failed with errno "
        << errno;
      return false;
    }
    
    logged_code_vec.push_back(msg_code);
  }  

  for(std::size_t i=0; i<code_vec.size(); ++i) {
    std::vector<dsv_log_code>::iterator loc = 
      std::find(logged_code_vec.begin(),logged_code_vec.end(),code_vec[i]);
    
    if(loc == logged_code_vec.end()) {
      std::stringstream err;
      err << "ERROR: parser failed to log msg with code: " << code_vec[i];
      return false;
    }
    
    logged_code_vec.erase(loc);
  }

  // logged_code_vec may still contain logged messages
  if(!logged_code_vec.empty()) {
    std::stringstream err;
    err << "ERROR: parser logged unexpected msgs:";
    for(std::size_t i=0; i<logged_code_vec.size(); ++i)
      err << " " << logged_code_vec[i];
    return false;
  }
  
  return true;
}


struct field_context {
  std::vector<std::vector<std::string> > field_matrix;
  std::size_t invocation_count;
  
  field_context(void) :invocation_count(0) {}
};

static std::size_t invocation_count(const field_context &context)
{
  return context.invocation_count;
}

static std::size_t invocation_limit(const field_context &context)
{
  return context.field_matrix.size();
}

static bool required_invocations(const field_context &context)
{
  return invocation_count(context) == invocation_limit(context);
}

static int field_callback(const char *fields[], size_t size, void *context)
{
//   std::cerr << "field_callback !!!!!!!!!!!!!!!!!!\n";
  field_context &check_context = *static_cast<field_context*>(context);

  BOOST_REQUIRE_MESSAGE(check_context.field_matrix.size() > check_context.invocation_count,
    "field_callback called too many times. Called: " << check_context.invocation_count
    << " times before and limit was " << check_context.field_matrix.size());

  const std::vector<std::string> &row = 
    check_context.field_matrix[check_context.invocation_count];

  BOOST_REQUIRE_MESSAGE(size == row.size(),
    "field_callback called with an incorrect number of fields. Should be "
    << row.size() << " received " << size);

  for(std::size_t i=0; i<size; ++i) {
    BOOST_REQUIRE_MESSAGE(row[i] == fields[i],
      "field_callback saw incorrect field. Should be '"
      << row[i] << "' received '" << fields[i] << "'");
  }

  ++(check_context.invocation_count);

  return 1;
}

// context is an  pointer to an existing std::vector<std::vector<std::string> >
int fill_matrix(const char *fields[], size_t size, void *context)
{
  std::vector<std::vector<std::string> > &matrix = 
    *static_cast<std::vector<std::vector<std::string> >*>(context);
  
  std::vector<std::string> field_vec(fields,fields+size);
  
  matrix.push_back(field_vec);

  return 1;
}

std::string print_matrix(const std::vector<std::vector<std::string> > &matrix)
{
  std::stringstream out;
  for(std::size_t i=0; i<matrix.size(); ++i) {
    out << "-->";
    for(std::size_t j=0; j<matrix[i].size(); ++j)
      out << "  \"" << matrix[i][j] << "\"";
 
    out << "\n";
  }
  
  return out.str();
}


template<std::size_t Rows, std::size_t Columns>
field_context make_field_context(const char * (&matrix)[Rows][Columns])
{
  field_context result;
  result.field_matrix.resize(Rows);
  
  for(std::size_t i=0; i<Rows; ++i) {
    result.field_matrix[i].resize(Columns);
    for(std::size_t j=0; j<Columns; ++j) {
      result.field_matrix[i][j] = matrix[i][j];
    } 
  }
  
  return result;
}

}


}
}

#endif
