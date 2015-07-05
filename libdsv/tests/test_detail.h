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
#include <sstream>
#include <ctime>
#include <cstring>

#include <boost/filesystem.hpp>

namespace dsv {
namespace test {


namespace detail {


namespace fs=boost::filesystem;

static const std::string testdatadir(QUOTEME(TESTDATA_DIR));


// 0x0A = LF
// 0x0D = CR
// 0x22 = "
// 0x27 = '
// 0x5C = backslash

static const char rfc4180_charset[] = {
  ' ','!','#','$','%','&',0x27,'(',')','*','+','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J',
  'K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',0x5C,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u',
  'v','w','x','y','z','{','|','}','~',0
};

static const char rfc4180_quoted_charset[] = {
  ' ','!',0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/','0','1','2','3','4',
  '5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J',
  'K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',0x5C,']','^','_',
  '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u',
  'v','w','x','y','z','{','|','}','~',0x0D,0x0A,0
};

static const char rfc4180_raw_quoted_charset[] = {
  0x22,' ','!',0x22,0x22,'#','$','%','&',0x27,'(',')','*','+',',','-','.','/','0','1','2',
  '3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H',
  'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',0x5C,']',
  '^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
  't','u','v','w','x','y','z','{','|','}','~',0x0D,0x0A,0x22,0
};

static const char crlf[] = {0x0D,0x0A,0};

static const char lf[] = {0x0A,0};

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
  std::string label;
  const std::vector<std::vector<std::string> > valid_matrix;
  std::vector<std::vector<std::string> > parsed_matrix;
  std::size_t invocation_count;
  
  field_context(void) :label("unnamed"), valid_matrix(), invocation_count(0) {}
  field_context(const std::string &str, const std::vector<std::vector<std::string> > &matrix) 
    :label(str), valid_matrix(matrix), invocation_count(0) {}
};


static int field_callback(const char *fields[], size_t size, void *_context)
{
  field_context &context = *static_cast<field_context*>(_context);

// if(size == 0)
//   std::cerr << "GOT EMPTY FIELDS\n";
  
// std::cerr << context.label << " field_callback " << context.invocation_count << " got: ";
// for(std::size_t i=0; i<size; ++i)
//   std::cerr << fields[i] << " ";
// std::cerr << "\n";

  std::vector<std::string> row;
  for(std::size_t i=0; i<size; ++i)
    row.push_back(fields[i]);
  
  context.parsed_matrix.push_back(row);
  
  (context.invocation_count)++;

  return 1;
}

inline fs::path gen_testfile(const std::vector<std::string> contents,
  const std::string &label)
{
  std::time_t now = std::time(0);

  fs::path tmpdir = fs::temp_directory_path();
  fs::path filename = std::string("libdsv_") + std::to_string(now) + "_" + label + ".dsv";
  fs::path filepath = tmpdir/filename;


  std::unique_ptr<std::FILE,int(*)(std::FILE *)> out(std::fopen(filepath.c_str(),"wb"),&std::fclose);
//  std::pair<file_sentry_type,std::string> result(file_sentry_type(out,&std::fclose),filename);
//   std::pair<file_sentry_type,std::string> result(file_sentry_type(out,&myfclose),filename);

  for(std::size_t i=0; i<contents.size(); ++i) {
    assert(std::fwrite(contents[i].data(),sizeof(std::string::value_type),
      contents[i].size(),out.get()) == contents[i].size());
  }
  
  return filepath;
}

std::string output_fields(const detail::field_context &context)
{
  std::stringstream out;
  
  out << "valid matrix:";
  
  std::size_t i;
  for(i=0; i<context.valid_matrix.size(); ++i) {
    out << "\n\t";
    
    std::size_t j;
    for(j=0; j<context.valid_matrix[i].size(); ++j) {
      out << "-->" << context.valid_matrix[i][j];
      if(i<context.parsed_matrix.size() && j<context.parsed_matrix[i].size()) {
        if(context.valid_matrix[i][j] != context.parsed_matrix[i][j])
          out << "[" << context.parsed_matrix[i][j] << "]";
      }
      else {
        out << "[missing]";
      }
      
      out << "<-- ";
    }

//     std::cerr << "i is: " << i << " size is: " << context.parsed_matrix.size() << "\n";
    for(;i<context.parsed_matrix.size() && j<context.parsed_matrix[i].size(); ++j) {
//       std::cerr << "j is: " << j << " size is: " << context.parsed_matrix[i].size() << "\n";
      out << " [[" << context.parsed_matrix[i][j] << "]]";
    }
  }    

  for(;i<context.parsed_matrix.size(); ++i) {
    out << "\n\t";
    for(std::size_t j=0; j<context.parsed_matrix[i].size(); ++j) {
      out << " [[" << context.parsed_matrix[i][j] << "]]";
    }
  }
  
  return out.str();
}

void check_compliance(const std::vector<std::vector<std::string> > &headers,
  const std::vector<std::vector<std::string> > &records, 
  const std::vector<std::string> contents, const std::string &label)
{
  fs::path filepath = gen_testfile(contents,label);

  std::unique_ptr<std::FILE,int(*)(std::FILE *)> in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);  

  detail::field_context header_context("header",headers);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context("records",records);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);
  
  int result;
  if((result = dsv_parse(filepath.c_str(),in.get(),parser,operations))) {
    std::stringstream reason;
    reason << "dsv_parse failed with code: " << result << " ";
    
    if(result > 0)
      reason << "(" << strerror(result) << ")";
    else
      reason << "MSG LOG: '" << detail::msg_log(parser,dsv_log_all) << "'";
    
    reason << " for given file: '";
    if(!filepath.empty())
      reason << filepath;
    else
      reason << "[file stream]";
    
    reason << "'\nHeader " << output_fields(header_context) 
      << "\nRecord " << output_fields(record_context);
    
    BOOST_REQUIRE_MESSAGE(result == 0,reason.str());
    
  }

  BOOST_REQUIRE_MESSAGE(
    header_context.invocation_count == header_context.parsed_matrix.size() &&
    header_context.valid_matrix == header_context.parsed_matrix,
      "Headers did not parse correctly. Correct: " << ": "
      << header_context.parsed_matrix.size() << " Parsed: "
      << header_context.invocation_count << "\n" << output_fields(header_context));

  BOOST_REQUIRE_MESSAGE(
    record_context.invocation_count == record_context.parsed_matrix.size() &&
    record_context.valid_matrix == record_context.parsed_matrix,
      "Records did not parse correctly. Correct: " << ": "
      << header_context.parsed_matrix.size() << " Parsed: "
      << header_context.invocation_count << "\n" << output_fields(record_context));

  // if here, then delete the test file
  in.reset(0);
  fs::remove(filepath);
}

}


}
}

#endif
