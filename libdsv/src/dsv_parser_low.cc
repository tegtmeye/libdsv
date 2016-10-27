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
    _parser->p = new detail::parser;
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

// todo, cause the exclusive to carry over and have reset API
int dsv_parser_set_equiv_record_delimiters(dsv_parser_t _parser,
  const unsigned char *equiv_byteseq[], const size_t byteseq_size[],
  const int byteseq_repeat[], size_t size, int repeatflag, int exclusiveflag)
{
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  if(!size)
    return EINVAL;

  int err = 0;

  try {
    parser.record_delimiters(equiv_bytesequence_type(equiv_byteseq,
      byteseq_size,byteseq_repeat,size,repeatflag,exclusiveflag));
  }
  catch(std::bad_alloc &) {
    // todo, bug
    // apparantly calling reserve with an argument of
    // numeric_limits<size_t>::max() (which is arguably > vector.max_size())
    // produces a bad_alloc instead of length_error for at least the LLVM
    // implementation of vector. The allocate method of vector seems to throw
    // length_error but since what we actually get is a bad_alloc, something
    // else is going on. For now, just return EINVAL as a correctly implemented
    // version of libdsv should only produce an EINVAL for incorrect arguments
    // as of 6/16.
    // err = ENOMEM;
    err = EINVAL;
  }
  catch(std::length_error &) {
    err = EINVAL;
  }
  catch(...) {
    abort();
  }

  return err;
}

size_t dsv_parser_num_equiv_record_delimiters(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    result = parser.record_delimiters().byteseq_desc_vec().size();
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_equiv_record_delimiters_repeatflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    result = parser.record_delimiters().repeatflag();
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_equiv_record_delimiters_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    result = parser.record_delimiters().exclusiveflag();
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_equiv_record_delimiter(dsv_parser_t _parser, size_t n,
  unsigned char *buf, size_t bufsize, int *repeatflag)
{
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;
  typedef equiv_bytesequence_type::byteseq_desc byteseq_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  const equiv_bytesequence_type &bytesequence = parser.record_delimiters();

  try {
    if(n < bytesequence.byteseq_desc_vec().size()) {
      const byteseq_desc &delim = bytesequence.byteseq_desc_vec().at(n);

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
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}




void dsv_parser_set_field_columns(dsv_parser_t _parser, size_t num_cols)
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

size_t dsv_parser_get_field_columns(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result;

  try {
    result = parser.field_columns();
  }
  catch(...) {
    abort();
  }

  return result;
}












// todo, cause the exclusive to carry over and have reset API
int dsv_parser_set_equiv_field_delimiters(dsv_parser_t _parser,
  const unsigned char *equiv_byteseq[], const size_t byteseq_size[],
  const int byteseq_repeat[], size_t size, int repeatflag, int exclusiveflag)
{
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  if(!size)
    return EINVAL;

  int err = 0;

  try {
    parser.field_delimiters(equiv_bytesequence_type(equiv_byteseq,
      byteseq_size,byteseq_repeat,size,repeatflag,exclusiveflag));
  }
  catch(std::bad_alloc &) {
    // todo, bug
    // apparantly calling reserve with an argument of
    // numeric_limits<size_t>::max() (which is arguably > vector.max_size())
    // produces a bad_alloc instead of length_error for at least the LLVM
    // implementation of vector. The allocate method of vector seems to throw
    // length_error but since what we actually get is a bad_alloc, something
    // else is going on. For now, just return EINVAL as a correctly implemented
    // version of libdsv should only produce an EINVAL for incorrect arguments
    // as of 6/16.
    // err = ENOMEM;
    err = EINVAL;
  }
  catch(std::length_error &) {
    err = EINVAL;
  }
  catch(...) {
    abort();
  }

  return err;
}

size_t dsv_parser_num_equiv_field_delimiters(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    result = parser.field_delimiters().byteseq_desc_vec().size();
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_equiv_field_delimiters_repeatflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    result = parser.field_delimiters().repeatflag();
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_equiv_field_delimiters_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    result = parser.field_delimiters().exclusiveflag();
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_equiv_field_delimiter(dsv_parser_t _parser, size_t n,
  unsigned char *buf, size_t bufsize, int *repeatflag)
{
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;
  typedef equiv_bytesequence_type::byteseq_desc byteseq_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  const equiv_bytesequence_type &bytesequence = parser.field_delimiters();

  try {
    if(n < bytesequence.byteseq_desc_vec().size()) {
      const byteseq_desc &delim = bytesequence.byteseq_desc_vec().at(n);

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
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_append_field_escape_pair(dsv_parser_t _parser,
  const unsigned char *open_escape_seq[],
  const size_t open_escape_seq_size[], const int open_escape_repeat[],
  size_t open_size, int open_repeatflag, int open_exclusiveflag,
  const unsigned char *close_escape_seq[],
  const size_t close_escape_seq_size[], const int close_escape_repeat[],
  size_t close_size, int close_repeatflag, int close_exclusiveflag)
{
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  if(open_size == 0 || close_size == 0)
    return EINVAL;

  for(std::size_t i = 0; i < open_size; ++i) {
    if(open_escape_seq_size[i] == 0)
      return EINVAL;
  }

  for(std::size_t i = 0; i < close_size; ++i) {
    if(close_escape_seq_size[i] == 0)
      return EINVAL;
  }

  int err = 0;

  try {
    detail::parser::escaped_field_desc_seq_type escaped_field_seq;
    escaped_field_seq.reserve(parser.field_escapes().size()+1);
    escaped_field_seq.insert(escaped_field_seq.end(),
      parser.field_escapes().begin(),parser.field_escapes().end());

    escaped_field_seq.emplace_back(
      detail::parser::escaped_field_desc {
        equiv_bytesequence_type(open_escape_seq,open_escape_seq_size,
          open_escape_repeat,open_size,open_repeatflag,open_exclusiveflag),
        equiv_bytesequence_type(close_escape_seq,close_escape_seq_size,
          close_escape_repeat,close_size,close_repeatflag,close_exclusiveflag)
        }
      );

    parser.field_escapes(escaped_field_seq);
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch(...) {
    abort();
  }

  return err;
}

size_t dsv_parser_num_field_escape_pairs(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result;

  try {
    result = parser.field_escapes().size();
  }
  catch (...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_escape_pair_open_repeatflag(dsv_parser_t _parser,
  size_t i)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    if(i >= parser.field_escapes().size())
      result = -1;
    else {
      const escaped_field_desc &desc = parser.field_escapes()[i];
      result = desc.open_equiv_bytesequence.repeatflag();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_escape_pair_close_repeatflag(dsv_parser_t _parser,
  size_t i)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    if(i >= parser.field_escapes().size())
      result = -1;
    else {
      const escaped_field_desc &desc = parser.field_escapes()[i];
      result = desc.close_equiv_bytesequence.repeatflag();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

int dsv_parser_get_field_escape_pair_open_exclusiveflag(dsv_parser_t _parser,
  size_t i)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    if(i >= parser.field_escapes().size())
      result = -1;
    else {
      const escaped_field_desc &desc = parser.field_escapes()[i];
      result = desc.open_equiv_bytesequence.exclusiveflag();
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
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    if(i >= parser.field_escapes().size())
      result = -1;
    else {
      const escaped_field_desc &desc = parser.field_escapes()[i];
      result = desc.close_equiv_bytesequence.exclusiveflag();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

size_t dsv_parser_num_field_escape_pair_open_sequences(dsv_parser_t _parser,
  size_t pairi)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    if(pairi < parser.field_escapes().size()) {
      const escaped_field_desc &desc = parser.field_escapes()[pairi];
      result = desc.open_equiv_bytesequence.byteseq_desc_vec().size();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

size_t dsv_parser_num_field_escape_pair_close_sequences(dsv_parser_t _parser,
  size_t pairi)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;

  try {
    if(pairi < parser.field_escapes().size()) {
      const escaped_field_desc &desc = parser.field_escapes()[pairi];
      result = desc.close_equiv_bytesequence.byteseq_desc_vec().size();
    }
  }
  catch (...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_field_escape_pair_open_sequence(dsv_parser_t _parser,
  size_t pairi, size_t n, unsigned char *buf, size_t bufsize, int *repeatflag)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;
  typedef equiv_bytesequence_type::byteseq_desc byteseq_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;


  try {
    if(pairi >= parser.field_escapes().size())
      result = 0;
    else {
      const escaped_field_desc &desc = parser.field_escapes()[pairi];
      const equiv_bytesequence_type &bytesequence =
        desc.open_equiv_bytesequence;

      if(n < bytesequence.byteseq_desc_vec().size()) {
        const byteseq_desc &delim = bytesequence.byteseq_desc_vec().at(n);

        if(bufsize == 0)
          result = delim.base_seq_bytes.size();
        else {
          while(result < delim.base_seq_bytes.size() && result < bufsize) {
            buf[result] = delim.base_seq_bytes[result];
            ++result;
          }
        }

        if(repeatflag)
          *repeatflag = delim.repeat;
      }
    }
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}

size_t dsv_parser_get_field_escape_pair_close_sequence(dsv_parser_t _parser,
  size_t pairi, size_t n, unsigned char *buf, size_t bufsize, int *repeatflag)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;
  typedef equiv_bytesequence_type::byteseq_desc byteseq_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;


  try {
    if(pairi >= parser.field_escapes().size())
      result = 0;
    else {
      const escaped_field_desc &desc = parser.field_escapes()[pairi];
      const equiv_bytesequence_type &bytesequence =
        desc.close_equiv_bytesequence;

      if(n < bytesequence.byteseq_desc_vec().size()) {
        const byteseq_desc &delim = bytesequence.byteseq_desc_vec().at(n);

        if(bufsize == 0)
          result = delim.base_seq_bytes.size();
        else {
          while(result < delim.base_seq_bytes.size() && result < bufsize) {
            buf[result] = delim.base_seq_bytes[result];
            ++result;
          }
        }

        if(repeatflag)
          *repeatflag = delim.repeat;
      }
    }
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}

void dsv_parser_clear_field_escape_pairs(dsv_parser_t _parser)
{
  typedef detail::parser::escaped_field_desc_seq_type escaped_field_desc_seq;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  parser.field_escapes(escaped_field_desc_seq());

}

int dsv_parser_get_field_escape_exclusiveflag(dsv_parser_t _parser)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result;

  try {
    result = parser.field_escapes_exclusives();
  }
  catch (...) {
    abort();
  }

  return result;
}

void dsv_parser_set_field_escape_exclusiveflag(dsv_parser_t _parser, int flag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  try {
    parser.field_escapes_exclusives(flag);
  }
  catch (...) {
    abort();
  }
}


int dsv_parser_append_equiv_escaped_field_escapes(dsv_parser_t _parser,
  size_t field_escape_pair, const unsigned char *equiv_byteseq[],
  const size_t byteseq_size[], const int byteseq_repeat[],
  size_t equiv_size, int repeatflag, int exclusiveflag,
  const unsigned char replace_seq[], size_t replace_seq_size)
{
  typedef detail::parser::bytesequence_type bytesequence_type;
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  if(equiv_byteseq == 0 || byteseq_size == 0 || byteseq_repeat == 0
    || equiv_size == 0)
  {
    return EINVAL;
  }

  if(parser.field_escapes().size() <= field_escape_pair)
    return EINVAL;

  for(std::size_t i = 0; i < equiv_size; ++i) {
    if(!(equiv_byteseq[i] && byteseq_size[i]))
      return EINVAL;
  }


  int err = 0;

  try {
    detail::parser::replacement_pair_seq_type replacement_pair_seq;
    const detail::parser::escaped_field_desc &desc =
      parser.field_escapes().at(field_escape_pair);

    replacement_pair_seq.reserve(desc.escaped_field_escapes.size()+1);
    replacement_pair_seq.insert(replacement_pair_seq.end(),
      desc.escaped_field_escapes.begin(),desc.escaped_field_escapes.end());

    replacement_pair_seq.emplace_back(
      detail::parser::replacement_pair_type {
        equiv_bytesequence_type(equiv_byteseq,byteseq_size,byteseq_repeat,
          equiv_size,repeatflag,exclusiveflag),
        bytesequence_type(replace_seq,replace_seq+replace_seq_size)
      }
    );

    parser.set_escaped_field_escapes(field_escape_pair,replacement_pair_seq);
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}



int dsv_parser_clear_equiv_escaped_field_escapes(dsv_parser_t _parser,
  size_t field_escape_pair)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  if(parser.field_escapes().size() <= field_escape_pair)
    return EINVAL;

  int err = 0;

  try {
    detail::parser::replacement_pair_seq_type replacement_pair_seq;

    parser.set_escaped_field_escapes(field_escape_pair,replacement_pair_seq);
  }
  catch(std::bad_alloc &) {
    err = ENOMEM;
  }
  catch (...) {
    abort();
  }

  return err;
}


size_t dsv_parser_num_equiv_escaped_field_escapes(dsv_parser_t _parser,
  size_t field_escape_pair)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t value = static_cast<size_t>(-1);

  try {
    if(field_escape_pair < parser.field_escapes().size()) {
      const detail::parser::escaped_field_desc &desc =
        parser.field_escapes().at(field_escape_pair);

      value = desc.escaped_field_escapes.size();
    }
  }
  catch (...) {
    abort();
  }

  return value;
}

size_t dsv_parser_num_equiv_escaped_field_escapes_sequences(
  dsv_parser_t _parser, size_t field_escape_pair, size_t equiv_escape_idx)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t value = static_cast<size_t>(-1);

  try {
    if(field_escape_pair < parser.field_escapes().size()) {
      const detail::parser::escaped_field_desc &desc =
        parser.field_escapes().at(field_escape_pair);

      if(equiv_escape_idx < desc.escaped_field_escapes.size()) {
        const detail::parser::escaped_field_desc &desc =
          parser.field_escapes().at(field_escape_pair);

        value = desc.escaped_field_escapes.size();
      }
    }
  }
  catch (...) {
    abort();
  }

  return value;
}

size_t dsv_parser_get_equiv_escaped_field_escapes_replacement(
  dsv_parser_t _parser, size_t pairi, size_t idx, unsigned char *buf,
  size_t bufsize)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;
  typedef detail::parser::bytesequence_type bytesequence_type;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;


  try {
    if(pairi < parser.field_escapes().size()) {
      const escaped_field_desc &desc = parser.field_escapes().at(pairi);

      if(idx < desc.escaped_field_escapes.size()) {
        const bytesequence_type &bytesequence =
          desc.escaped_field_escapes.at(idx).second;

        if(bufsize == 0)
          result = bytesequence.size();
        else {
          while(result < bytesequence.size() && result < bufsize) {
            buf[result] = bytesequence[result];
            ++result;
          }
        }
      }
    }
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}



size_t dsv_parser_get_equiv_escaped_field_escapes_sequence(
  dsv_parser_t _parser, size_t pairi, size_t idx, size_t n,
  unsigned char *buf, size_t bufsize, int *repeatflag)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;
  typedef detail::parser::equiv_bytesequence_type equiv_bytesequence_type;
  typedef equiv_bytesequence_type::byteseq_desc byteseq_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  size_t result = 0;


  try {
    if(pairi < parser.field_escapes().size()) {
      const escaped_field_desc &desc = parser.field_escapes().at(pairi);

      if(idx < desc.escaped_field_escapes.size()) {
        const equiv_bytesequence_type &bytesequence =
          desc.escaped_field_escapes.at(idx).first;

        if(n < bytesequence.byteseq_desc_vec().size()) {
          const byteseq_desc &seq = bytesequence.byteseq_desc_vec().at(n);

          if(bufsize == 0)
            result = seq.base_seq_bytes.size();
          else {
            while(result < seq.base_seq_bytes.size() && result < bufsize) {
              buf[result] = seq.base_seq_bytes[result];
              ++result;
            }
          }

          if(repeatflag)
            *repeatflag = seq.repeat;
        }
      }
    }
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_get_escaped_field_escapes_repeatflag(dsv_parser_t _parser,
  size_t pairi)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    if(pairi < parser.field_escapes().size()) {
      const escaped_field_desc &desc = parser.field_escapes().at(pairi);

      result = desc.escaped_repeatflag;
    }
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_set_escaped_field_escapes_repeatflag(dsv_parser_t _parser,
    size_t pairi, int flag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    if(pairi < parser.field_escapes().size())
      parser.set_escaped_field_escapes_repeat(pairi,flag);
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}




int dsv_parser_get_escaped_field_escapes_exclusiveflag(dsv_parser_t _parser,
  size_t pairi)
{
  typedef detail::parser::escaped_field_desc escaped_field_desc;

  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    if(pairi < parser.field_escapes().size()) {
      const escaped_field_desc &desc = parser.field_escapes().at(pairi);

      result = desc.escaped_exclusiveflag;
    }
  }
  catch(std::range_error &) {
    abort(); // should never get here.
  }
  catch(...) {
    abort();
  }

  return result;
}

int dsv_parser_set_escaped_field_escapes_exclusiveflag(dsv_parser_t _parser,
    size_t pairi, int flag)
{
  assert(_parser.p);

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);

  int result = -1;

  try {
    if(pairi < parser.field_escapes().size())
      parser.set_escaped_field_escapes_exclusives(pairi,flag);
  }
  catch(std::range_error &) {
    abort(); // should never get here.
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

  detail::parser &parser = *static_cast<detail::parser*>(_parser.p);
  detail::parse_operations &operations =
    *static_cast<detail::parse_operations*>(_operations.p);

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


}
