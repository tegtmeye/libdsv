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

#include "dsv_parser.h"
#include "parser.h"
#include "parse_operations.h"
#include "scanner_state.h"
#include "dsv_grammar.hh"

#include <cerrno>
#include <stdlib.h>

#include <system_error>
#include <regex>
#include <sstream>

#include <boost/system/error_code.hpp>

namespace bs = boost::system;

extern "C" {

int dsv_parser_create(dsv_parser_t *parser)
{
  int err = 0;

  try {
     parser->p = new detail::parser;
  }
  catch (std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}

void dsv_parser_destroy(dsv_parser_t parser)
{
  try {
     delete static_cast<detail::parser*>(parser.p);
  }
  catch(...) {
    abort();
  }
}

int dsv_parser_set_newline_handling(dsv_parser_t _parser, dsv_newline_behavior behavior)
{
  assert(_parser.p);

  if(!(behavior >= dsv_newline_permissive && behavior <= dsv_newline_crlf_strict))
    return EINVAL;

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = 0;

  try {
    parser.newline_behavior(behavior);
  }
  catch(...) {
    abort();
  }

  return result;
}

dsv_newline_behavior dsv_parser_get_newline_handling(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  dsv_newline_behavior result;

  try {
    result = parser.newline_behavior();
  }
  catch(...) {
    abort();
  }

  return result;
}


void dsv_parser_set_field_columns(dsv_parser_t _parser, ssize_t num_cols)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  try {
    parser.field_columns(num_cols);
  }
  catch(...) {
    abort();
  }
}

ssize_t dsv_parser_get_field_columns(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  ssize_t result;

  try {
    result = parser.field_columns();
  }
  catch(...) {
    abort();
  }

  return result;
}


int dsv_operations_create(dsv_operations_t *_operations)
{
  int err = 0;

  try {
    detail::parse_operations *operations = new detail::parse_operations;
    operations->record_callback = 0;
    operations->record_context = 0;

    _operations->p = operations;

  }
  catch (std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}

void dsv_operations_destroy(dsv_operations_t operations)
{
  try {
    delete static_cast<detail::parse_operations*>(operations.p);
  }
  catch(...) {
    abort();
  }
}


header_callback_t dsv_get_header_callback(dsv_operations_t _operations)
{
  assert(_operations.p);

  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  header_callback_t result = 0;

  try {
    result = operations.header_callback;

  }
  catch(...) {
    abort();
  }

  return result;
}

void * dsv_get_header_context(dsv_operations_t _operations)
{
  assert(_operations.p);

  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  void * result = 0;

  try {
    result = operations.header_context;

  }
  catch(...) {
    abort();
  }

  return result;
}



void dsv_set_header_callback(header_callback_t fn, void *context,
  dsv_operations_t _operations)
{
  assert(_operations.p);

  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  try {
    operations.header_callback = fn;
    operations.header_context = context;
  }
  catch(...) {
    abort();
  }
}











record_callback_t dsv_get_record_callback(dsv_operations_t _operations)
{
  assert(_operations.p);

  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  record_callback_t result = 0;

  try {
    result = operations.record_callback;

  }
  catch(...) {
    abort();
  }

  return result;
}

void * dsv_get_record_context(dsv_operations_t _operations)
{
  assert(_operations.p);

  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  void * result = 0;

  try {
    result = operations.record_context;

  }
  catch(...) {
    abort();
  }

  return result;
}



void dsv_set_record_callback(record_callback_t fn, void *context,
  dsv_operations_t _operations)
{
  assert(_operations.p);

  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  try {
    operations.record_callback = fn;
    operations.record_context = context;
  }
  catch(...) {
    abort();
  }
}

int dsv_parse(const char *location_str, FILE *stream, dsv_parser_t _parser,
              dsv_operations_t _operations)
{
  dsv_parse_log_count(_parser,dsv_log_all);

  assert(_parser.p && _operations.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);
  detail::parse_operations &operations = *static_cast<detail::parse_operations*>(_operations.p);

  int err = 0;

  try {
    //parser_debug = 1;

    detail::scanner_state scanner(location_str,stream);
    std::unique_ptr<detail::scanner_state> base_ctx;

    parser.reset();
    int err = parser_parse(scanner,parser,operations,base_ctx);
    if(err != 0) {
      if(err == 2)
        throw std::system_error(ENOMEM,std::system_category());
      throw std::system_error(-1,std::generic_category(),"Parse failed");
    }
  }
  catch(std::system_error &ex) {
    // system errors due to failed parser or memory error from parse
    if(ex.code().category() == std::system_category())
      err = ex.code().value();
    else if(ex.code().category() == std::generic_category()) {
      if(ex.code().value() == -1)
        err = -1;
      else
        abort();
    }
    else
      abort();
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(...) {
    abort();
  }

  return err;
}

size_t dsv_parse_log_count(dsv_parser_t _parser, dsv_log_level log_level)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    detail::parser::const_log_iterator cur = parser.log_begin();
    while(cur != parser.log_end()) {
      if(cur->first & log_level)
        ++result;
      ++cur;
    }
  }
  catch(...) {
    abort();
  }

  return result;
}

ssize_t dsv_parse_log(dsv_parser_t _parser, dsv_log_level log_level, size_t num,
  dsv_log_code *code, char *buf, size_t len)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  ssize_t result = 0;

  try {
    size_t i = 0;
    detail::parser::const_log_iterator cur = parser.log_begin();
    while(cur != parser.log_end()) {
      if(cur->first & log_level && i++ == num)
        break;

      ++cur;
    }

    // num is greater than number of filtered messages
    if(cur == parser.log_end()) {
      errno = EINVAL;
      return -1;
    }

    // cur now hold the selected message
    const detail::log_description &desc = cur->second;

    *code = desc.code();

    if(!buf) {
      detail::log_description::const_param_iterator param = desc.param_begin();
      while(param != desc.param_end()) {
        result += param->size();
        ++param;
      }
    }
    else {
      // for each parameter, copy it into the corresponding placeholder deducting the
      // length of the parameter from len. If len is smaller than the length of the
      // parameter, just copy len characters. Stop on the last parameter or if len is
      // zero
      std::string tmp_str(buf);
      detail::log_description::const_param_iterator param = desc.param_begin();
      for(std::size_t i=0; param != desc.param_end(); ++i, ++param) {
        // create the placeholder string
        std::stringstream n;
        n << "\\$" << i;

        std::regex exp(n.str());
        tmp_str = std::regex_replace(tmp_str,exp,*param);
      }

      buf = std::copy_n(tmp_str.begin(),std::min(tmp_str.size(),len-1),buf);

      // add null character to the end
      *buf = 0;
    }
  }
  catch(std::bad_alloc &) {
    errno = ENOMEM;
    result = -1;
  }
  catch(...) {
    abort();
  }

  return result;
}

}
