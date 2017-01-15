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

#ifndef LIBDSV_PARSER_H
#define LIBDSV_PARSER_H


#include "dsv_parser.h"

#include <list>
#include <vector>
#include <stack>
#include <memory>
#include <regex>


#include <iostream>

namespace detail {


class log_description {
  private:
    typedef std::list<std::string> param_list_type;

  public:
    typedef param_list_type::const_iterator const_param_iterator;

    template<typename StringIteratorT>
    log_description(dsv_log_code c, StringIteratorT first,
      StringIteratorT last);

    dsv_log_code code(void) const;
    std::size_t param_size(void) const;
    const_param_iterator param_begin(void) const;
    const_param_iterator param_end(void) const;

  private:
    dsv_log_code _code;
    param_list_type param_list;
};

template<typename StringIteratorT>
inline log_description::log_description(dsv_log_code c,
  StringIteratorT first, StringIteratorT last) :_code(c), param_list(first,last)
{
}

inline dsv_log_code log_description::code(void) const
{
  return _code;
}

inline std::size_t log_description::param_size(void) const
{
  return param_list.size();
}

inline log_description::const_param_iterator log_description::param_begin(void) const
{
  return param_list.begin();
}

inline log_description::const_param_iterator log_description::param_end(void) const
{
  return param_list.end();
}



template<typename CharT>
class parser {
  private:
    typedef std::list<std::pair<dsv_log_level,log_description> > log_list_type;

  public:
    typedef CharT char_type;
    typedef std::vector<char_type> char_sequence_type;
    typedef std::basic_string<char_type> expression_type;
    typedef std::basic_regex<char_type> regex_type;

    class escaped_replacement_desc;
    class escaped_field_desc;

    typedef std::vector<escaped_replacement_desc> escaped_replacement_desc_seq;
    typedef std::vector<escaped_field_desc> escaped_field_desc_seq;

    typedef log_list_type::const_iterator const_log_iterator;


    parser(void);

    log_callback_t log_callback(void) const;
    log_callback_t log_callback(log_callback_t fn);

    void * log_context(void) const;
    void * log_context(void *context);

    dsv_log_level log_level(void) const;
    dsv_log_level log_level(dsv_log_level level);

    std::size_t log_size(void) const;
    const_log_iterator log_begin(void) const;
    const_log_iterator log_end(void) const;

    void append_log(dsv_log_level level, const log_description &desc);

    /* exposed behaviors */

    /* RECORD COMPLETION DELIMITER */
    void record_delimiters(const expression_type &exp);
    const expression_type & record_delimiters(void) const;
    const regex_type & record_delimiters_regex(void) const;
    void exclusive_record_delimiter(const char_sequence_type &seq);
    const char_sequence_type & exclusive_record_delimiter(void) const;

    void exclusive_record_delimiter_flag(bool flag);
    bool exclusive_record_delimiter_flag(void) const;

    /* FIELD COMPLETION DELIMITER */
    void field_delimiters(const expression_type &exp);
    const expression_type & field_delimiters(void) const;
    const regex_type & field_delimiters_regex(void) const;
    void exclusive_field_delimiter(const char_sequence_type &seq);
    const char_sequence_type & exclusive_field_delimiter(void) const;

    void exclusive_field_delimiter_flag(bool flag);
    bool exclusive_field_delimiter_flag(void) const;

    /* FIELD ESCAPES */
    void field_escapes(const escaped_field_desc_seq &seq);
    const escaped_field_desc_seq & field_escapes(void) const;

    void exclusive_field_escape(bool flag);
    bool exclusive_field_escape(void) const;

    void escape_field_escapes(size_t pairi,
      const escaped_replacement_desc_seq &replacement_desc);

    const escaped_replacement_desc_seq &
      escape_field_escapes(size_t pairi) const;


    /* FIELD COLUMNS */
    std::size_t restrict_field_columns(void) const;
    void restrict_field_columns(std::size_t n);


    /*
        Regular expression to search for valid content in a non-field escape.

        Regex searches for field and record delimiters and open field escapes
    */
    const regex_type & free_field_regex(void) const;
    std::size_t field_subexp_idx(void) const;
    std::size_t record_subexp_idx(void) const;
    // return value corresponds to each of the field escape pairs. That is,
    // if the ith subexpression is returned and it is the jth element of the
    // return value, then the jth pair was opened.
    const std::vector<std::size_t> & open_field_escape_subexp_idx(void) const;

    /*
        Regular expression to search for valid content in a field escape
        corresponding to the \c pair_idx pair.
    */
    const regex_type & field_escape_regex(std::size_t pair_idx) const;
    std::size_t close_field_escape_subexp_idx(std::size_t pair_idx) const;
    const std::vector<std::size_t> &
      escaped_field_escape_replacement_subexp_idx(std::size_t pair_idx) const;



    /* STATE MAINTENANCE */

    /*
      These set exclusivity to a particular escaped field pair. If \c pair_idx
      is negative, then exclusive_field_escape is true but a field escape
      has not been encountered yet. Reset this back to -1 if
      exclusive_field_escape is false and no longer in a field escape. That is,
      the value should be sticky if exclusive_field_escape is true
    */
    void selected_exclusive_field_escape(int pair_idx);
    int selected_exclusive_field_escape(void) const;

    /* EFFECTIVE FIELD COLUMNS */
    void effective_field_columns(std::size_t num_cols);
    std::size_t effective_field_columns(void) const;



    /* RESET EVERYTHING */
    void reset(void);





  private:
    log_callback_t _log_callback;
    void *_log_context;
    dsv_log_level _log_level;

    log_list_type log_list;

    // RECORD DELIMITERS
    expression_type _record_delimiters;
    regex_type _record_delimiters_regex;
    char_sequence_type _exclusive_record_delimiter;
    bool _exclusive_record_delimiter_flag;

    // FIELD DELIMITERS
    expression_type _field_delimiters;
    regex_type _field_delimiters_regex;
    char_sequence_type _exclusive_field_delimiter;
    bool _exclusive_field_delimiter_flag;

    // FIELD ESCAPES
    escaped_field_desc_seq _field_escapes;
    bool _exclusive_field_escape;

    // FIELD COLUMN MANAGEMENT
    std::size_t _restrict_field_columns;
    std::size_t _effective_field_columns;
};

template<typename CharT>
class parser<CharT>::escaped_replacement_desc {
  public:
    escaped_replacement_desc(const expression_type &exp,
      const char_sequence_type &rep);

    const expression_type & expression(void) const;
    const regex_type & regex(void) const;
    const char_sequence_type & replacement(void) const;

  private:
    expression_type _expression;
    regex_type _regex;
    char_sequence_type _replacement;
};

template<typename CharT>
inline parser<CharT>::escaped_replacement_desc::escaped_replacement_desc(
  const expression_type &exp, const char_sequence_type &rep) :_expression(exp),
    _regex(exp), _replacement(rep)
{
}

template<typename CharT>
inline const typename parser<CharT>::expression_type &
parser<CharT>::escaped_replacement_desc::expression(void) const
{
  return _expression;
}

template<typename CharT>
inline const typename parser<CharT>::regex_type &
parser<CharT>::escaped_replacement_desc::regex(void) const
{
  return _regex;
}

template<typename CharT>
inline const typename parser<CharT>::char_sequence_type &
parser<CharT>::escaped_replacement_desc::replacement(void) const
{
  return _replacement;
}



template<typename CharT>
class parser<CharT>::escaped_field_desc {
  public:
    escaped_field_desc(const expression_type &open, bool open_excl,
      const expression_type &close, bool close_excl);

    const expression_type & open_expression(void) const;
    const regex_type & open_regex(void) const;
    const char_sequence_type & open_exclusive_seq(void) const;
    void open_exclusive_seq(const char_sequence_type &seq);
    bool open_exclusive(void) const;

    const expression_type & close_expression(void) const;
    const regex_type & close_regex(void) const;
    const char_sequence_type & close_exclusive_seq(void) const;
    void close_exclusive_seq(const char_sequence_type &seq);
    bool close_exclusive(void) const;

    const escaped_replacement_desc_seq & replacement_desc_seq(void) const;
    void replacement_desc_seq(const escaped_replacement_desc_seq &seq);

  private:
    expression_type _open_expression;
    regex_type _open_regex;
    char_sequence_type _open_exclusive_seq;
    bool _open_exclusive;

    expression_type _close_expression;
    regex_type _close_regex;
    char_sequence_type _close_exclusive_seq;
    bool _close_exclusive;

    escaped_replacement_desc_seq _replacement_desc_seq;
};

template<typename CharT>
inline parser<CharT>::escaped_field_desc::escaped_field_desc(
  const expression_type &open, bool open_excl,
  const expression_type &close, bool close_excl)
    :_open_expression(open), _open_regex(open), _open_exclusive(open_excl),
      _close_expression(close), _close_regex(close),
        _close_exclusive(close_excl)
{
}

template<typename CharT>
inline const typename parser<CharT>::expression_type &
parser<CharT>::escaped_field_desc::open_expression(void) const
{
  return _open_expression;
}

template<typename CharT>
inline const typename parser<CharT>::regex_type &
parser<CharT>::escaped_field_desc::open_regex(void) const
{
  return _open_regex;
}

template<typename CharT>
inline const typename parser<CharT>::char_sequence_type &
parser<CharT>::escaped_field_desc::open_exclusive_seq(void) const
{
  return _open_exclusive_seq;
}

template<typename CharT>
inline void parser<CharT>::escaped_field_desc::open_exclusive_seq(
  const char_sequence_type &seq)
{
  _open_exclusive_seq = seq;
}

template<typename CharT>
inline bool parser<CharT>::escaped_field_desc::open_exclusive(void) const
{
  return _open_exclusive;
}

template<typename CharT>
inline const typename parser<CharT>::expression_type &
parser<CharT>::escaped_field_desc::close_expression(void) const
{
  return _close_expression;
}

template<typename CharT>
inline const typename parser<CharT>::regex_type &
parser<CharT>::escaped_field_desc::close_regex(void) const
{
  return _close_regex;
}

template<typename CharT>
inline const typename parser<CharT>::char_sequence_type &
parser<CharT>::escaped_field_desc::close_exclusive_seq(void) const
{
  return _close_exclusive_seq;
}

template<typename CharT>
inline void parser<CharT>::escaped_field_desc::close_exclusive_seq(
  const char_sequence_type &seq)
{
  _close_exclusive_seq = seq;
}

template<typename CharT>
inline bool parser<CharT>::escaped_field_desc::close_exclusive(void) const
{
  return _close_exclusive;
}

template<typename CharT>
inline const typename parser<CharT>::escaped_replacement_desc_seq &
parser<CharT>::escaped_field_desc::replacement_desc_seq(void) const
{
  return _replacement_desc_seq;
}

template<typename CharT>
inline void parser<CharT>::escaped_field_desc::replacement_desc_seq(
  const escaped_replacement_desc_seq &seq)
{
  _replacement_desc_seq = seq;
}








template<typename CharT>
inline parser<CharT>::parser(void) :_log_callback(0), _log_context(0),
  _log_level(dsv_log_none), _exclusive_record_delimiter_flag(0),
  _exclusive_field_delimiter_flag(0), _exclusive_field_escape(0),
  _restrict_field_columns(0)
{
}

template<typename CharT>
inline log_callback_t parser<CharT>::log_callback(void) const
{
  return _log_callback;
}

template<typename CharT>
inline log_callback_t parser<CharT>::log_callback(log_callback_t fn)
{
  std::swap(fn,_log_callback);
  return fn;
}

template<typename CharT>
inline void * parser<CharT>::log_context(void) const
{
  return _log_context;
}

template<typename CharT>
inline void * parser<CharT>::log_context(void *context)
{
  std::swap(context,_log_context);
  return context;
}

template<typename CharT>
inline dsv_log_level parser<CharT>::log_level(void) const
{
  return _log_level;
}

template<typename CharT>
inline dsv_log_level parser<CharT>::log_level(dsv_log_level level)
{
  std::swap(level,_log_level);
  return level;
}

template<typename CharT>
inline std::size_t parser<CharT>::log_size(void) const
{
  return log_list.size();
}

template<typename CharT>
inline typename parser<CharT>::const_log_iterator
parser<CharT>::log_begin(void) const
{
  return log_list.begin();
}

template<typename CharT>
inline typename parser<CharT>::const_log_iterator
parser<CharT>::log_end(void) const
{
  return log_list.end();
}

template<typename CharT>
inline void
parser<CharT>::append_log(dsv_log_level level, const log_description &desc)
{
  log_list.push_back(std::make_pair(level,desc));
}

template<typename CharT>
inline void parser<CharT>::record_delimiters(const expression_type &exp)
{
  // construction may throw regex_error, don't set values unless we know it
  // succeeds
  regex_type tmp_regex(exp);

  _record_delimiters = exp;
  std::swap(_record_delimiters_regex,tmp_regex);

  // trigger recompile of derived
}

template<typename CharT>
inline const typename parser<CharT>::expression_type &
parser<CharT>::record_delimiters(void) const
{
  return _record_delimiters;
}

template<typename CharT>
inline const typename parser<CharT>::regex_type &
parser<CharT>::record_delimiters_regex(void) const
{
  return _record_delimiters_regex;
}

template<typename CharT>
inline void
parser<CharT>::exclusive_record_delimiter(const char_sequence_type &seq)
{
  _exclusive_record_delimiter = seq;
}

template<typename CharT>
inline const typename parser<CharT>::char_sequence_type &
parser<CharT>::exclusive_record_delimiter(void) const
{
  return _exclusive_record_delimiter;
}

template<typename CharT>
inline void parser<CharT>::exclusive_record_delimiter_flag(bool flag)
{
  _exclusive_record_delimiter_flag = flag;
}

template<typename CharT>
inline bool parser<CharT>::exclusive_record_delimiter_flag(void) const
{
  return _exclusive_record_delimiter_flag;
}

template<typename CharT>
inline void parser<CharT>::field_delimiters(const expression_type &exp)
{
  // construction may throw regex_error, don't set values unless we know it
  // succeeds
  regex_type tmp_regex(exp);

  _field_delimiters = exp;
  std::swap(_field_delimiters_regex,tmp_regex);

  // trigger recompile of derived
}

template<typename CharT>
inline const typename parser<CharT>::expression_type &
parser<CharT>::field_delimiters(void) const
{
  return _field_delimiters;
}

template<typename CharT>
inline const typename parser<CharT>::regex_type &
parser<CharT>::field_delimiters_regex(void) const
{
  return _field_delimiters_regex;
}

template<typename CharT>
inline void
parser<CharT>::exclusive_field_delimiter(const char_sequence_type &seq)
{
  _exclusive_field_delimiter = seq;
}

template<typename CharT>
inline const typename parser<CharT>::char_sequence_type &
parser<CharT>::exclusive_field_delimiter(void) const
{
  return _exclusive_field_delimiter;
}

template<typename CharT>
inline void parser<CharT>::exclusive_field_delimiter_flag(bool flag)
{
  _exclusive_field_delimiter_flag = flag;
}

template<typename CharT>
inline bool parser<CharT>::exclusive_field_delimiter_flag(void) const
{
  return _exclusive_field_delimiter_flag;
}




template<typename CharT>
inline void parser<CharT>::field_escapes(const escaped_field_desc_seq &seq)
{
  _field_escapes = seq;
}

template<typename CharT>
inline const typename parser<CharT>::escaped_field_desc_seq &
parser<CharT>::field_escapes(void) const
{
  return _field_escapes;
}

template<typename CharT>
inline void parser<CharT>::exclusive_field_escape(bool flag)
{
  _exclusive_field_escape = flag;
}

template<typename CharT>
inline bool parser<CharT>::exclusive_field_escape(void) const
{
  return _exclusive_field_escape;
}

template<typename CharT>
inline void parser<CharT>::escape_field_escapes(size_t pairi,
  const escaped_replacement_desc_seq &replacement_desc)
{
  _field_escapes[pairi].replacement_desc_seq(replacement_desc);
}

template<typename CharT>
inline const typename parser<CharT>::escaped_replacement_desc_seq &
parser<CharT>::escape_field_escapes(size_t pairi) const
{
  return _field_escapes[pairi].replacement_desc_seq();
}


template<typename CharT>
inline std::size_t parser<CharT>::restrict_field_columns(void) const
{
  return _restrict_field_columns;
}

template<typename CharT>
inline void parser<CharT>::restrict_field_columns(std::size_t n)
{
  _restrict_field_columns = n;
}



















template<typename CharT>
inline void parser<CharT>::reset(void)
{

  // fix me
}

}

#endif
