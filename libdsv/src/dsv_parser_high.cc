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

#if 0

#include "dsv_parser.h"
#include "parser.h"
#include "parse_operations.h"
#include "scanner_state.h"
#include "dsv_grammar.hh"

#include <cerrno>
#include <cstdlib>

#include <system_error>
#include <regex>
#include <sstream>
#include <memory>

#include <boost/system/error_code.hpp>

namespace bs = boost::system;

extern "C" {

int dsv_parser_create(dsv_parser_t *_parser)
{
  int err = 0;

  try {
    std::unique_ptr<detail::parser> parser(new detail::parser);

    parser->newline_behavior(dsv_newline_permissive);
    parser->field_columns(0);

    _parser->p = parser.release();
  }
  catch (std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}

int dsv_parser_create_RFC4180_strict(dsv_parser_t *_parser)
{
  int err = 0;

  try {
    std::unique_ptr<detail::parser> parser(new detail::parser);

    parser->newline_behavior(dsv_newline_RFC4180_strict);
    parser->field_columns(0);

    _parser->p = parser.release();
  }
  catch (std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}

int dsv_parser_create_RFC4180_permissive(dsv_parser_t *_parser)
{
  int err = 0;

  try {
    std::unique_ptr<detail::parser> parser(new detail::parser);

    parser->newline_behavior(dsv_newline_permissive);
    parser->field_columns(0);

    _parser->p = parser.release();
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

int dsv_parser_set_newline_behavior(dsv_parser_t _parser, dsv_newline_behavior behavior)
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

dsv_newline_behavior dsv_parser_get_newline_behavior(dsv_parser_t _parser)
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

int dsv_parser_set_field_delimiter(dsv_parser_t _parser, unsigned char delim)
{
  assert(_parser.p);

  int err = 0;

  try {
    const unsigned char *delim_arr[1] = {&delim};
    const size_t delimsize_arr[1] = {1};
    const int delimrepeat_arr[1] = {0};
    err = dsv_parser_set_field_wdelimiter_equiv(_parser,delim_arr,delimsize_arr,
      delimrepeat_arr,1,0,1);
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(...) {
    abort();
  }

  return err;
}

int dsv_parser_set_field_wdelimiter(dsv_parser_t _parser,
  const unsigned char *delim, size_t size)
{
  assert(_parser.p);

  int err = 0;

  try {
    const unsigned char *delim_arr[1] = {delim};
    const size_t delimsize_arr[1] = {size};
    const int delimrepeat_arr[1] = {0};
    err = dsv_parser_set_field_wdelimiter_equiv(_parser,delim_arr,delimsize_arr,
      delimrepeat_arr,1,0,1);
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(...) {
    abort();
  }

  return err;
}

// todo, cause the exclusive to carry over and have reset API
int dsv_parser_set_field_wdelimiter_equiv(dsv_parser_t _parser,
  const unsigned char *delim[], const size_t delimsize[],
  const int delim_repeat[], size_t size, int repeatflag, int exclusiveflag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int err = !size;

  try {
    if(!err) {
      // check validity before setting anything
      for(std::size_t i=0; i<size; ++i) {
        if(!delimsize[i])
          err = EINVAL;
      }
    }

    if(!err) {
      parser.set_equiv_delimiters(delim,delimsize,delim_repeat,size,
        repeatflag,exclusiveflag);
    }
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(...) {
    abort();
  }

  return err;
}

size_t dsv_parser_num_field_delimiters(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    result = parser.delimiters_descs().size();
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_delimiters_repeatflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    result = parser.delimiter_repeatflag();
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_delimiters_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    result = parser.delimiter_parse_exclusive();
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_field_delimiter(dsv_parser_t _parser, size_t n,
  unsigned char *buf, size_t bufsize, int *repeatflag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    if(n >= parser.delimiters_descs().size())
      result = 0;
    else {
      const detail::parser::byteseq_desc &delim =
        parser.delimiters_descs().at(n);

      if(bufsize == 0)
        result = delim.seq_bytes.size();
      else {
        while(result < delim.seq_bytes.size() && result < bufsize) {
          buf[result] = delim.seq_bytes[result];
          ++result;
        }
      }

      if(repeatflag)
        *repeatflag = delim.repeat;
    }
  }
  catch(...) {
    abort();
  }

  return result;
}

void dsv_parser_allow_escaped_binary_fields(dsv_parser_t _parser, int flag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  try {
    parser.escaped_binary_fields(flag);
  }
  catch(...) {
    abort();
  }
}

int dsv_parser_escaped_binary_fields_allowed(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    result = parser.escaped_binary_fields();
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
//    std::cerr << "got system error\n\n";
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

log_callback_t dsv_get_logger_callback(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  log_callback_t result = 0;

  try {
    result = parser.log_callback();
  }
  catch(...) {
    abort();
  }

  return result;
}

/**
 *  \brief Obtain the user-defined context currently set for header
 *
 *  \retval 0 No context is registered
 *  \retval nonzero The currently registered context
 */
void * dsv_get_logger_context(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  void *result = 0;

  try {
    result = parser.log_context();
  }
  catch(...) {
    abort();
  }

  return result;
}

dsv_log_level dsv_get_log_level(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  dsv_log_level result;

  try {
    result = parser.log_level();
  }
  catch(...) {
    abort();
  }

  return result;
}


/**
 *  \brief Associate the logging callback \c fn, a user-specified \c context,
 *  for logging \c level with \c parser.
 */
  void dsv_set_logger_callback(log_callback_t fn, void *context,
    dsv_log_level level, dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  try {
    parser.log_callback(fn);
    parser.log_context(context);
    parser.log_level(level);
  }
  catch(...) {
    abort();
  }
}














#if 0
todo

  /**
   *  \brief Enable or disable binary in double quoted fields for future parsing
   *  with \c parser
   *
   *  The default setting is 0 (false)
   *
   *  Under RFC-4180, the file shall only contain ASCII printable characters.
   *  When enabled, this turns off most character translation in double quoted
   *  fields. Double quotes are still recognized as well as double quote
   *  preceded by another double quote.
   *
   *  Enabling is useful if the fields contain non-printing but otherwise useful
   *  ASCII characters as well as allowing for non-ASCII encoding such as UTF-8.
   *  The library makes no attempt to translate any such characters.
   *
   *  The downside is that newline interpretation is also turned off. Therefore
   *  only newlines outside of the double quoted fields are counted for location
   *  tracking. This means that if syntax errors are encountered, it can make
   *  locating them difficult.
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] flag nonzero to enable, zero to disable
   */
  void dsv_parser_allow_escaped_binary_fields(dsv_parser_t parser, int flag);

  /**
   *  \brief Query the whether binary is recognized in double quoted fields for
   *  future parsing with \c parser
   *
   *  Under RFC-4180, the file shall only contain ASCII printable characters.
   *  When enabled, this turns off character translation in double quoted fields
   *  (double quotes are till recognized obviously--this includes double quote
   *  preceded by another double quote.)
   *
   *  Enabling is useful if the fields contain non-printing but otherwise useful
   *  ASCII characters as well as allowing for non-ASCII encoding such as UTF-8.
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] flag nonzero to enable, zero to disable
   */
  int dsv_parser_escaped_binary_fields_allowed(dsv_parser_t parser);




void dsv_parser_allow_escaped_binary_fields(dsv_parser_t _parser, int flag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  try {
    parser.escaped_binary_fields(flag);
  }
  catch(...) {
    abort();
  }
}

int dsv_parser_escaped_binary_fields_allowed(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    result = parser.escaped_binary_fields();
  }
  catch(...) {
    abort();
  }

  return result;
}












#endif

}

#endif