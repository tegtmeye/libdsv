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
    \file This file is used to provide convenience functions across test cases.
    Put functions that rely on having a built and linked dsv_library in here
 */

#include "api_detail.h"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <cstdio>


namespace detail {

const std::string & format_str(dsv_log_code msg_code)
{
  switch(msg_code) {
    case dsv_syntax_error:
      static const std::string parse_error("syntax error: In file $4, line $1 "
        "column $3");
      return parse_error;

    case dsv_inconsistant_column_count:
      static const std::string column_count_message("inconsistant column "
        "count: In file $4, line $1. Expected $2 fields, read $3 fields");

      return column_count_message;

    default:
      ;
  };

  static const std::string unknown_error("unknown error code");
  return unknown_error;
}

std::string to_string(dsv_log_level level)
{
  if(level == dsv_log_none)
    return "dsv_log_none";

  std::stringstream out;

  out << "[";

  if(level & dsv_log_error)
    out << " dsv_log_error";

  if(level & dsv_log_warning)
    out << " dsv_log_warning";

  if(level & dsv_log_info)
    out << " dsv_log_info";

  if(level & dsv_log_debug)
    out << " dsv_log_debug";

  out << "]";

  return out.str();
}

std::string to_string(dsv_log_code level)
{
  if(level == dsv_syntax_error)
    return "dsv_syntax_error";

  if(level == dsv_inconsistant_column_count)
    return "dsv_inconsistant_column_count";

  if(level == dsv_unexpected_binary)
    return "dsv_unexpected_binary";

  return "Unknown dsv_log_code";
}

bool check_logs(const std::vector<log_msg> &required,
  const std::vector<log_msg> &received)
{
  if(required.size() != received.size())
    return false;

  for(std::size_t i=0; i<required.size(); ++i) {
    const log_msg &req_msg = required[i];
    const log_msg &rec_msg = received[i];

    if(req_msg.code != rec_msg.code && req_msg.level != rec_msg.level)
      return false;

    if(req_msg.param_vec.empty())
      continue;

    if(req_msg.param_vec.size() != rec_msg.param_vec.size())
      BOOST_FAIL("UNIT TEST ERROR: msg parameter list must either be empty or "
        "match that of the given code: " << to_string(req_msg.code));

    for(std::size_t j=0; j<req_msg.param_vec.size(); ++j) {
      if(!req_msg.param_vec[j].empty()
        && req_msg.param_vec[j] != rec_msg.param_vec[j])
      {
        return false;
      }
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
    out << "\n\t" << to_string(logs[i].code) << to_string(logs[i].level)
      << ":";
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
    out << "\n\t" << to_string(required[i].code) << to_string(required[i].level)
      << ":";
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

int header_callback(const unsigned char *fields[],
  const size_t lengths[], size_t size, void *_context)
{
  file_context &context = *static_cast<file_context*>(_context);

  BOOST_REQUIRE_MESSAGE(context.parsed_records.empty(),
    "Header callback called after " << context.parsed_records.size()
      << " records seen");

  std::vector<field_storage_type> row;
  for(std::size_t i=0; i<size; ++i)
    row.push_back(field_storage_type(fields[i],fields[i]+lengths[i]));

  context.parsed_headers.push_back(row);

  return 1;
}

int record_callback(const unsigned char *fields[],
  const size_t lengths[], size_t size, void *_context)
{
  file_context &context = *static_cast<file_context*>(_context);

  std::vector<field_storage_type> row;
  for(std::size_t i=0; i<size; ++i) {
    row.push_back(field_storage_type(fields[i],fields[i]+lengths[i]));
  }

  context.parsed_records.push_back(row);

  return 1;
}

int logger(dsv_log_code code, dsv_log_level level, const char *params[],
  size_t size, void *_context)
{
  logging_context &context = *static_cast<logging_context*>(_context);

  log_msg msg{code,level,{}};

  for(std::size_t i=0; i<size; ++i) {
    msg.param_vec.push_back(params[i]);
  }

  context.recd_logs.push_back(msg);

  return 1;
}

std::string output_fields(
  const std::vector<std::vector<field_storage_type> > &valid_matrix,
  const std::vector<std::vector<field_storage_type> > &parsed_matrix)
{
  std::stringstream out;

  out << "Matrix. Key := valid='--><--' parsed='[[]]' extra='(())'";

  std::size_t i;
  for(i=0; i<valid_matrix.size(); ++i) {
    out << "\n\t";

    std::size_t j;
    for(j=0; j<valid_matrix[i].size(); ++j) {
      out << "-->" << to_string(valid_matrix[i][j]) << "<--";
      if(i<parsed_matrix.size() && j<parsed_matrix[i].size()) {
        if(valid_matrix[i][j] != parsed_matrix[i][j])
          out << " [[" << to_string(parsed_matrix[i][j]) << "]]";
        else
          out << " [[parsed identical]]";
      }
      else {
        out << "[[missing]]";
      }
    }

    for(;i<parsed_matrix.size() && j<parsed_matrix[i].size(); ++j) {
      out << " ((" << to_string(parsed_matrix[i][j]) << "))";
    }
  }

  out << "\nextra parsed matrix:";
  for(;i<parsed_matrix.size(); ++i) {
    out << "\n\t";
    for(std::size_t j=0; j<parsed_matrix[i].size(); ++j) {
      out << " ((" << to_string(parsed_matrix[i][j]) << "))";
    }
  }

  return out.str();
}

void check_file_compliance(dsv_parser_t parser,
  const std::vector<std::vector<field_storage_type> > &headers,
  const std::vector<std::vector<field_storage_type> > &records,
  const std::vector<log_msg> &log_msgs, const fs::path &filepath,
  const std::string &, int expected_result)
{
  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  file_context context(headers,records);
  dsv_set_header_callback(header_callback,&context,operations);
  dsv_set_record_callback(record_callback,&context,operations);

  int result = dsv_parse(filepath.c_str(),in.get(),parser,operations);
  if(result != expected_result) {
    std::stringstream out;
    out << "dsv_parse returned with unexpected code: " << result
      << ", expecting " << expected_result;

    if(result > 0)
      out << " (" << strerror(result) << ")";
    else
      out << " MSG LOG:\n" << compare_logs(log_msgs,log_context.recd_logs);

    out << "\nfor given file: ";
    if(!filepath.empty())
      out << filepath;
    else
      out << "[file stream]";

    out << "\nHeader "
      << output_fields(context.valid_headers,context.parsed_headers)
      << "\nRecord "
      << output_fields(context.valid_records,context.parsed_records);

    BOOST_REQUIRE_MESSAGE(result == expected_result,out.str());
  }

  BOOST_REQUIRE_MESSAGE(
    context.valid_headers.size() == context.parsed_headers.size() &&
    context.valid_headers == context.parsed_headers,
      "Headers did not parse correctly. Correct: " << ": "
      << context.valid_headers.size() << " Parsed: "
      << context.parsed_headers.size() << "\n"
      << output_fields(context.valid_headers,context.parsed_headers));

  BOOST_REQUIRE_MESSAGE(
    context.valid_records.size() == context.parsed_records.size() &&
    context.valid_records == context.parsed_records,
      "Records did not parse correctly. Correct: " << ": "
      << context.valid_records.size() << " Parsed: "
      << context.parsed_records.size() << "\n"
      << output_fields(context.valid_records,context.parsed_records));

  BOOST_REQUIRE_MESSAGE(check_logs(log_msgs,log_context.recd_logs),
    "Did not receive the correct type and/or number of log messages:\n"
    << compare_logs(log_msgs,log_context.recd_logs));
}

void check_compliance(dsv_parser_t parser,
  const std::vector<std::vector<field_storage_type> > &headers,
  const std::vector<std::vector<field_storage_type> > &records,
  const std::vector<log_msg> &log_msgs,
  const std::vector<field_storage_type> &contents,
  const std::string &label, int expected_result)
{
  fs::path filepath = gen_testfile(contents,label);

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  file_context context(headers,records);
  dsv_set_header_callback(header_callback,&context,operations);
  dsv_set_record_callback(record_callback,&context,operations);

  int result = dsv_parse(filepath.c_str(),in.get(),parser,operations);
  if(result != expected_result) {
    std::stringstream out;
    out << "dsv_parse returned with unexpected code: " << result
      << ", expecting " << expected_result;

    if(result > 0)
      out << " (" << strerror(result) << ")";
    else
      out << " MSG LOG:\n" << compare_logs(log_msgs,log_context.recd_logs);

    out << "\nfor given file: ";
    if(!filepath.empty())
      out << filepath;
    else
      out << "[file stream]";

    out << "\nHeader "
      << output_fields(context.valid_headers,context.parsed_headers)
      << "\nRecord "
      << output_fields(context.valid_records,context.parsed_records);

    BOOST_REQUIRE_MESSAGE(result == expected_result,out.str());
  }

  BOOST_REQUIRE_MESSAGE(
    context.valid_headers.size() == context.parsed_headers.size() &&
    context.valid_headers == context.parsed_headers,
      "Headers did not parse correctly. Correct: " << ": "
      << context.valid_headers.size() << " Parsed: "
      << context.parsed_headers.size() << "\n"
      << output_fields(context.valid_headers,context.parsed_headers));

  BOOST_REQUIRE_MESSAGE(
    context.valid_records.size() == context.parsed_records.size() &&
    context.valid_records == context.parsed_records,
      "Records did not parse correctly. Correct: " << ": "
      << context.valid_records.size() << " Parsed: "
      << context.parsed_records.size() << "\n"
      << output_fields(context.valid_records,context.parsed_records));

  BOOST_REQUIRE_MESSAGE(check_logs(log_msgs,log_context.recd_logs),
    "Did not receive the correct type and/or number of log messages:\n"
    << compare_logs(log_msgs,log_context.recd_logs));

  // if here, then delete the test file
  in.reset(0);
  fs::remove(filepath);
}


}
