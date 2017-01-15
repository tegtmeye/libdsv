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

#ifndef LIBDSV_TEST_API_DETAIL_H
#define LIBDSV_TEST_API_DETAIL_H


#include "test_detail.h"

#include "dsv_parser.h"

#include <string>
#include <vector>

namespace detail {


namespace fs=boost::filesystem;



inline void parser_destroy(dsv_parser_t *p)
{
  dsv_parser_destroy(*p);
}

inline void operations_destroy(dsv_operations_t *p)
{
  dsv_operations_destroy(*p);
}

const std::string & format_str(dsv_log_code msg_code);


std::string to_string(dsv_log_level level);

std::string to_string(dsv_log_code level);


struct log_msg {
  dsv_log_code code;
  dsv_log_level level;
  std::vector<std::string> param_vec;
};

struct logging_context {
  std::vector<log_msg> recd_logs;
};


/*
  The required log_msg list is compared to the received msg_log list. For each
  msg, if the codes are different, then return false. For each log_msg, the
  parameters are compared against each other. If the required params are empty
  (ie param_vec is empty()) then the received params are ignored. That is, any
  value of the params are accepted. If the required param_vec is non-empty, then
  the entire list must be present. If a single value of the required param_vec
  is empty, then any value of the received param is accepted, otherwise it must
  match exactly.

  let req_msg and rec_msg each be the ith value of required and received
    respectively
  if(req_msg.code != rec_msg.code) then false
  if(req_msg.param_vec.empty()) then accept
  if(req_msg.param_vec.size() != rec_msg.param_vec.size()) then UNIT TEST ERROR

  let req_param and rec_param each be the jth value of req_msg.param_vec and
    rec_msg.param_vec respectively
  if(req_param.empty()) accept this param
  if(req_param != rec_param) return false
*/
bool check_logs(const std::vector<log_msg> &required,
  const std::vector<log_msg> &received);


std::string output_logs(const std::vector<log_msg> &logs);

std::string compare_logs(const std::vector<log_msg> &required,
  const std::vector<log_msg> &received);

struct file_context {
  const std::vector<std::vector<field_storage_type > > valid_headers;
  std::vector<std::vector<field_storage_type > > parsed_headers;

  const std::vector<std::vector<field_storage_type > > valid_records;
  std::vector<std::vector<field_storage_type > > parsed_records;

  file_context(void) {}
  file_context(const std::vector<std::vector<field_storage_type > > &headers,
    const std::vector<std::vector<field_storage_type > > &records)
       :valid_headers(headers), valid_records(records) {}
};

int header_callback(const unsigned char *fields[],
  const size_t lengths[], size_t size, void *_context);

int record_callback(const unsigned char *fields[],
  const size_t lengths[], size_t size, void *_context);

int logger(dsv_log_code code, dsv_log_level level, const char *params[],
  size_t size, void *_context);

std::string output_fields(
  const std::vector<std::vector<field_storage_type> > &valid_matrix,
  const std::vector<std::vector<field_storage_type> > &parsed_matrix);

void check_file_compliance(dsv_parser_t parser,
  const std::vector<std::vector<field_storage_type> > &headers,
  const std::vector<std::vector<field_storage_type> > &records,
  const std::vector<log_msg> &log_msgs, const fs::path &filepath,
  const std::string &, int expected_result);

void check_compliance(dsv_parser_t parser,
  const std::vector<std::vector<field_storage_type> > &headers,
  const std::vector<std::vector<field_storage_type> > &records,
  const std::vector<log_msg> &log_msgs,
  const std::vector<field_storage_type> &contents,
  const std::string &label, int expected_result);

}


#endif
