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
#include "basic_scanner.h"
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

typedef detail::parser<char> parser_type;
typedef detail::basic_scanner<char> scanner_type;

int dsv_parser_create(dsv_parser_t *_parser)
{
  int err = 0;

  try {
    _parser->p = new parser_type;
  }
  catch (std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}

void dsv_parser_destroy(dsv_parser_t _parser)
{
  try {
     delete static_cast<parser_type*>(_parser.p);
  }
  catch(...) {
    abort();
  }
}

// todo, cause the exclusive to carry over and have reset API
int dsv_parser_set_record_delimiters(dsv_parser_t _parser,
  const char *utf8_regex, size_t regex_size, int exclusiveflag)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int err = 0;

  try {
    if(regex_size)
      parser.record_delimiters(
        parser_type::expression_type(utf8_regex,regex_size));
    else
      parser.record_delimiters(parser_type::expression_type());

    // must be after as expressions may throw regex_error
    parser.exclusive_record_delimiter_flag(exclusiveflag);
  }
  catch(std::length_error &) {
    // thrown for basic_string<>(buff,-1)
    err = ENOMEM;
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(std::regex_error &) {
    err = EINVAL;
  }
  catch(...) {
    abort();
  }

  return err;
}

int dsv_parser_get_record_delimiter_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int result;

  try {
    result = parser.exclusive_record_delimiter_flag();
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_record_delimiters(dsv_parser_t _parser, char *buff,
  size_t buffsize)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = 0;

  try {
    std::size_t size = parser.record_delimiters().size();
    if(buffsize == 0)
      result = size;
    else {
      result = std::min(size,buffsize);

      copy_n(parser.record_delimiters().begin(),result,buff);
    }
  }
  catch(...) {
    abort();
  }

  return result;
}

// todo, cause the exclusive to carry over and have reset API
int dsv_parser_set_field_delimiters(dsv_parser_t _parser,
  const char *utf8_regex, size_t regex_size, int exclusiveflag)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int err = 0;

  try {
    if(regex_size)
      parser.field_delimiters(
        parser_type::expression_type(utf8_regex,regex_size));
    else
      parser.field_delimiters(parser_type::expression_type());

    // must be after as expressions may throw regex_error
    parser.exclusive_field_delimiter_flag(exclusiveflag);
  }
  catch(std::length_error &) {
    // thrown for basic_string<>(buff,-1)
    err = ENOMEM;
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(std::regex_error &) {
    err = EINVAL;
  }
  catch(...) {
    abort();
  }

  return err;
}

int dsv_parser_get_field_delimiter_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int result;

  try {
    result = parser.exclusive_field_delimiter_flag();
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_field_delimiters(dsv_parser_t _parser, char *buff,
  size_t buffsize)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = 0;

  try {
    std::size_t size = parser.field_delimiters().size();
    if(buffsize == 0)
      result = size;
    else {
      result = std::min(size,buffsize);

      copy_n(parser.field_delimiters().begin(),result,buff);
    }
  }
  catch(...) {
    abort();
  }

  return result;
}


int dsv_parser_set_field_escape_pair(dsv_parser_t _parser,
  const char *open_utf8_regex[], const size_t open_regex_size[],
  const int open_exclusiveflag[],
  const char *close_utf8_regex[], const size_t close_regex_size[],
  const int close_exclusiveflag[],
  size_t pair_size, int pair_exclusiveflag)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  if(pair_size == 0) {
    parser.field_escapes(parser_type::escaped_field_desc_seq());
    return 0;
  }

  if(!(open_utf8_regex && open_regex_size && open_exclusiveflag
    && close_utf8_regex && close_regex_size && close_exclusiveflag))
  {
    return EINVAL;
  }

  int err = 0;

  try {
    parser_type::escaped_field_desc_seq escaped_field_seq;

    for(std::size_t i=0; i<pair_size; ++i) {
      if(!(open_utf8_regex[i] && open_regex_size[i]
        && close_utf8_regex[i] && close_regex_size[i]))
      {
        err = EINVAL;
        break;
      }

      escaped_field_seq.emplace_back(parser_type::escaped_field_desc(
        parser_type::expression_type(open_utf8_regex[i],open_regex_size[i]),
          open_exclusiveflag[i],
        parser_type::expression_type(close_utf8_regex[i],close_regex_size[i]),
          close_exclusiveflag[i])
        );
    }
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(std::regex_error &) {
    err = EINVAL;
  }
  catch(...) {
    abort();
  }

  return err;
}

size_t dsv_parser_num_field_escape_pairs(dsv_parser_t _parser)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result;

  try {
    result = parser.field_escapes().size();
  }
  catch (...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_escape_pair_open_exclusiveflag(dsv_parser_t _parser,
  size_t i)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int result;

  try {
    typedef parser_type::escaped_field_desc escaped_field_desc;

    if(i >= parser.field_escapes().size())
      result = -1;
    else {
      const escaped_field_desc &desc = parser.field_escapes().at(i);
      result = desc.open_exclusive();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_escape_pair_close_exclusiveflag(dsv_parser_t _parser,
  size_t i)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int result;

  try {
    typedef parser_type::escaped_field_desc escaped_field_desc;

    if(i >= parser.field_escapes().size())
      result = -1;
    else {
      const escaped_field_desc &desc = parser.field_escapes().at(i);
      result = desc.close_exclusive();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_field_escape_pair_open_expression(dsv_parser_t _parser,
    size_t pairi, char *buff, size_t buffsize)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = 0;


  try {
    if(pairi >= parser.field_escapes().size())
      result = 0;
    else {
      const parser_type::escaped_field_desc &desc = parser.field_escapes().at(pairi);

      std::size_t expr_size = desc.open_expression().size();

      if(!buffsize)
        result = expr_size;
      else {
        result = std::min(buffsize,expr_size);

        std::copy_n(desc.open_expression().begin(),result,buff);
      }
    }
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_field_escape_pair_close_expression(dsv_parser_t _parser,
    size_t pairi, char *buff, size_t buffsize)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = 0;


  try {
    if(pairi >= parser.field_escapes().size())
      result = 0;
    else {
      const parser_type::escaped_field_desc &desc = parser.field_escapes().at(pairi);

      std::size_t expr_size = desc.close_expression().size();

      if(!buffsize)
        result = expr_size;
      else {
        result = std::min(buffsize,expr_size);

        std::copy_n(desc.close_expression().begin(),result,buff);
      }
    }
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_escape_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  int result;

  try {
    result = parser.exclusive_field_escape();
  }
  catch (...) {
    abort();
  }

  return result;
}

void dsv_parser_set_field_escape_exclusiveflag(dsv_parser_t _parser, int flag)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  try {
    parser.exclusive_field_escape(flag);
  }
  catch (...) {
    abort();
  }
}


int dsv_parser_set_escape_field_escapes(dsv_parser_t _parser,
  size_t pairi, const char *utf8_regex[], const size_t regex_size[],
  const char *replacement[], size_t replacement_size[], size_t nescapes)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  if(nescapes && !(utf8_regex && regex_size && replacement && replacement_size))
  {
    return EINVAL;
  }

  int err = 0;

  try {
    if(pairi >= parser.field_escapes().size())
      err = -1;
    else {
      parser_type::escaped_replacement_desc_seq replacement_seq;

      for(std::size_t i=0; i<nescapes; ++i) {
        if(!(utf8_regex[i] && regex_size[i] &&
          (!replacement_size[i] || (replacement_size[i] && replacement[i]))))
        {
          err = EINVAL;
          break;
        }

        replacement_seq.emplace_back(
          parser_type::escaped_replacement_desc(
            parser_type::expression_type(utf8_regex[i],regex_size[i]),
            parser_type::char_sequence_type(replacement[i],
              replacement[i]+replacement_size[i]))
          );
      }

      parser.escape_field_escapes(pairi,replacement_seq);
    }
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(std::regex_error &) {
    err = EINVAL;
  }
  catch (...) {
    abort();
  }

  return err;
}





size_t dsv_parser_num_escape_field_escapes(dsv_parser_t _parser, size_t pairi)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = SIZE_MAX;

  try {
    if(pairi < parser.field_escapes().size()) {
      result = parser.field_escapes().at(pairi).replacement_desc_seq().size();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_escaped_field_escape_expression(dsv_parser_t _parser,
    size_t pairi, size_t idx,  unsigned char *buff, size_t buffsize)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = SIZE_MAX;

  try {
    if(pairi < parser.field_escapes().size()) {
      const parser_type::escaped_field_desc &escaped_field =
        parser.field_escapes().at(pairi);

      if(idx < escaped_field.replacement_desc_seq().size()) {
        const parser_type::escaped_replacement_desc &escape_desc =
          escaped_field.replacement_desc_seq().at(idx);

        std::size_t exp_size = escape_desc.expression().size();

        if(buffsize == 0)
          result = exp_size;
        else if(buff) {
          result = std::min(exp_size,buffsize);
          std::copy_n(escape_desc.expression().begin(),result,buff);
        }
      }
    }
  }
  catch(...) {
    abort();
  }

  return result;
}



size_t dsv_parser_get_escaped_field_escape_replacement(dsv_parser_t _parser,
    size_t pairi, size_t idx,  unsigned char *buff, size_t buffsize)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result = SIZE_MAX;

  try {
    if(pairi < parser.field_escapes().size()) {
      const parser_type::escaped_field_desc &escaped_field =
        parser.field_escapes().at(pairi);

      if(idx < escaped_field.replacement_desc_seq().size()) {
        const parser_type::escaped_replacement_desc &escape_desc =
          escaped_field.replacement_desc_seq().at(idx);

        std::size_t exp_size = escape_desc.replacement().size();

        if(buffsize == 0)
          result = exp_size;
        else if(buff) {
          result = std::min(exp_size,buffsize);
          std::copy_n(escape_desc.replacement().begin(),result,buff);
        }
      }
    }
  }
  catch(...) {
    abort();
  }

  return result;
}

void dsv_parser_set_field_columns(dsv_parser_t _parser, size_t num_cols)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  try {
    parser.restrict_field_columns(num_cols);
  }
  catch(...) {
    abort();
  }
}

size_t dsv_parser_get_field_columns(dsv_parser_t _parser)
{
  assert(_parser.p);

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  size_t result;

  try {
    result = parser.restrict_field_columns();
  }
  catch(...) {
    abort();
  }

  return result;
}


















// OPERATIONS
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

  parser_type &parser = *static_cast<parser_type*>(_parser.p);
  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

  int err = 0;

  try {
    //parser_debug = 1;

    scanner_type scanner(location_str,stream);
    std::unique_ptr<scanner_type> base_ctx;

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

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

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

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

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

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

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

  parser_type &parser = *static_cast<parser_type*>(_parser.p);

  try {
    parser.log_callback(fn);
    parser.log_context(context);
    parser.log_level(level);
  }
  catch(...) {
    abort();
  }
}


}
