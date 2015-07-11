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

// 94 characters not including newline
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

// 100 characters not including newline
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


#if 0
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
    ssize_t len = dsv_parse_flog(parser,log_level,i,&msg_code,0,0);
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
    
    len = dsv_parse_flog(parser,log_level,i,&msg_code,buff.get(),storage_len);
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
    ssize_t len = dsv_parse_flog(parser,dsv_log_all,i,&msg_code,0,0);
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
#endif


std::string to_string(dsv_log_code code)
{
  switch(code) {
    case dsv_nonrectangular_records_info:
      return "dsv_nonrectangular_records_info";
    
    case dsv_syntax_error:
      return "dsv_syntax_error";
    
    case dsv_column_count_error:
      return "dsv_column_count_error";
    
  };
  
  return "unknown code";
}

struct log_msg {
  dsv_log_code code;
  std::vector<std::string> param_vec;
};

struct logging_context {
  std::vector<log_msg> recd_logs;
};


/*
  The required log_msg list is compared to the received msg_log list. For each msg, if the
  codes are different, then return false. For each log_msg, the parameters are compared 
  against each other. If the required params are empty (ie param_vec is empty()) then the 
  received params are ignored. That is, any value of the params are accepted. If the 
  required param_vec is non-empty, then the entire list must be present. If a single value 
  of the required param_vec is empty, then any value of the received param is accepted, 
  otherwise it must match exactly.
  
  let req_msg and rec_msg each be the ith value of required and received respectively
  if(req_msg.code != rec_msg.code) then false
  if(req_msg.param_vec.empty()) then accept
  if(req_msg.param_vec.size() != rec_msg.param_vec.size()) then UNIT TEST ERROR
  
  let req_param and rec_param each be the jth value of req_msg.param_vec and  
    rec_msg.param_vec respectively
  if(req_param.empty()) accept this param
  if(req_param != rec_param) return false
*/
bool check_logs(const std::vector<log_msg> &required, 
  const std::vector<log_msg> &received)
{
  if(required.size() != received.size())
    return false;
  
  for(std::size_t i=0; i<required.size(); ++i) {
    const log_msg &req_msg = required[i];
    const log_msg &rec_msg = received[i];
    
    if(req_msg.code != rec_msg.code)
      return false;
    
    if(req_msg.param_vec.empty())
      continue;
    
    if(req_msg.param_vec.size() != rec_msg.param_vec.size())
      BOOST_FAIL("UNIT TEST ERROR: msg parameter list must either be empty or match that"
        "of the given code: " << to_string(req_msg.code));
    
    for(std::size_t j=0; j<req_msg.param_vec.size(); ++j) {
      if(!req_msg.param_vec[j].empty() && req_msg.param_vec[j] != rec_msg.param_vec[j])
        return false;
    }
  }
  
  return true;
}


std::string output_logs(const std::vector<log_msg> &logs)
{
  std::stringstream out;
  out << "Log output:";

  if(logs.empty()) {
    out << "\n\t[[empty]]";
    return out.str();
  }

  for(std::size_t i=0; i<logs.size(); ++i) {
    out << "\n\t" << to_string(logs[i].code) << ":";
    for(std::size_t j=0; j<logs[i].param_vec.size(); ++j) {
        out << " '" << logs[i].param_vec[j] << "'";
    }
  }  

  return out.str();
}

std::string compare_logs(const std::vector<log_msg> &required, 
  const std::vector<log_msg> &received)
{
  std::stringstream out;
  
  out << "Required log messages:";

  for(std::size_t i=0; i<required.size(); ++i) {
    out << "\n\t" << to_string(required[i].code) << ":";
    if(required[i].param_vec.empty()) {
      out << "[params ignored]";
    }
    else {
      for(std::size_t j=0; j<required[i].param_vec.size(); ++j) {
        if(required[i].param_vec[j].empty())
          out << " [ignored]";
        else
          out << " " << required[i].param_vec[j];
      }
    }
  }  

  out << "\nReceived log messages:" << output_logs(received);

  return out.str();
}



struct field_context {
  std::string label;
  const std::vector<std::vector<std::string> > valid_matrix;
  std::vector<std::vector<std::string> > parsed_matrix;
  
  field_context(void) :label("unnamed"), valid_matrix() {}
  field_context(const std::string &str, 
    const std::vector<std::vector<std::string> > &matrix) :label(str),
       valid_matrix(matrix) {}
};


static int field_callback(const char *fields[], size_t size, void *_context)
{
  field_context &context = *static_cast<field_context*>(_context);

  std::vector<std::string> row;
  for(std::size_t i=0; i<size; ++i)
    row.push_back(fields[i]);
  
  context.parsed_matrix.push_back(row);

  return 1;
}

static int logger(dsv_log_code code, dsv_log_level level, const char *params[], 
  size_t size, void *_context)
{
  logging_context &context = *static_cast<logging_context*>(_context);

  log_msg msg{code};
  
  for(std::size_t i=0; i<size; ++i)
    msg.param_vec.push_back(params[i]);

  context.recd_logs.push_back(msg);

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

    for(;i<context.parsed_matrix.size() && j<context.parsed_matrix[i].size(); ++j) {
      out << " [[" << context.parsed_matrix[i][j] << "]]";
    }
  }    

  out << "\nextra parsed matrix:";
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
  const std::vector<log_msg> &log_msgs, const std::vector<std::string> contents, 
  const std::string &label, int expected_result)
{
  fs::path filepath = gen_testfile(contents,label);

  std::unique_ptr<std::FILE,int(*)(std::FILE *)> in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,parser);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);  

  detail::field_context header_context("header",headers);
  dsv_set_header_callback(detail::field_callback,&header_context,operations);

  detail::field_context record_context("records",records);
  dsv_set_record_callback(detail::field_callback,&record_context,operations);
  
  int result = dsv_parse(filepath.c_str(),in.get(),parser,operations);
  if(result != expected_result) {
    std::stringstream out;
    out << "dsv_parse returned with unexpected with code: " << result << ", expecting "
      << expected_result;
    
    if(result > 0)
      out << " (" << strerror(result) << ")";
    else
      out << " MSG LOG:\n" << compare_logs(log_msgs,log_context.recd_logs);
    
    out << "\nfor given file: ";
    if(!filepath.empty())
      out << filepath;
    else
      out << "[file stream]";
    
    out << "\nHeader " << output_fields(header_context) 
      << "\nRecord " << output_fields(record_context);
    
    BOOST_REQUIRE_MESSAGE(result == expected_result,out.str()); 
  }

  BOOST_REQUIRE_MESSAGE(
    header_context.valid_matrix.size() == header_context.parsed_matrix.size() &&
    header_context.valid_matrix == header_context.parsed_matrix,
      "Headers did not parse correctly. Correct: " << ": "
      << header_context.valid_matrix.size() << " Parsed: "
      << header_context.parsed_matrix.size() << "\n" << output_fields(header_context));

  BOOST_REQUIRE_MESSAGE(
    record_context.valid_matrix.size() == record_context.parsed_matrix.size() &&
    record_context.valid_matrix == record_context.parsed_matrix,
      "Records did not parse correctly. Correct: " << ": "
      << header_context.valid_matrix.size() << " Parsed: "
      << header_context.parsed_matrix.size() << "\n" << output_fields(record_context));

  BOOST_REQUIRE_MESSAGE(check_logs(log_msgs,log_context.recd_logs),
    "Did not receive the correct type and/or number of log messages:\n"
    << compare_logs(log_msgs,log_context.recd_logs));
  
  
  // if here, then delete the test file
  in.reset(0);
  fs::remove(filepath);
}

}


}
}

#endif
