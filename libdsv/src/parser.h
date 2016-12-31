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

#ifndef LIBDSV_PARSER_H
#define LIBDSV_PARSER_H


#include "dsv_parser.h"
#include "equiv_bytesequence.h"

#include <list>
#include <vector>
#include <stack>
#include <memory>
#include <regex>


#include <iostream>

namespace detail {

// delete me
inline void print_packed(const std::vector<detail::byte_chunk> &packed_vec)
{
  for(std::size_t i=0; i<packed_vec.size(); ++i) {
    std::cerr << "Data @ " << i << ":\n"
      << "  Byte: " << char(packed_vec[i].byte)
      << "  Accept: " << int(packed_vec[i].accept)
      << "  Pass off: " << packed_vec[i].pass_skip
        << " (" << i+packed_vec[i].pass_skip << ")"
      << "  Fail off: " << packed_vec[i].fail_skip
        << " (";
    if(packed_vec[i].fail_skip==0)
      std::cerr << "reject";
    else
      std::cerr << i+packed_vec[i].fail_skip;
    std::cerr << ")\n\n";
  }
}


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

    typedef std::vector<
      std::pair<expression_type,char_sequence_type> > replacement_sequence_type;

    struct escaped_field_desc {
      expression_type open_expression;
      expression_type close_expression;
      replacement_sequence_type escaped_field_escape_replacements;
      bool open_exclusive;
      bool close_exclusive;
      bool replacement_exlusive;
    };

    typedef std::vector<escaped_field_desc> escaped_field_desc_seq_type;

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
    void exclusive_record_delimiter(const char_sequence_type &seq);
    const char_sequence_type & exclusive_record_delimiter(void) const;

    /* FIELD COMPLETION DELIMITER */
    void field_delimiters(const expression_type &exp);
    const expression_type & field_delimiters(void) const;
    void exclusive_field_delimiters(const char_sequence_type &seq);
    const char_sequence_type & exclusive_field_delimiters(void) const;

    /* FIELD ESCAPES */
    void field_escapes(const escaped_field_desc_seq_type &seq);
    const escaped_field_desc_seq_type & field_escapes(void) const;

    // if true, then a particular field escape pair should be selected as
    // exclusive if seen.
    void exclusive_field_escape(bool flag);
    bool exclusive_field_escape(void) const;

    // These set exclusivity within each escaped field pair
    void exclusive_open_field_escape(std::size_t pair_idx,
      const char_sequence_type &seq);
    const char_sequence_type &
      exclusive_open_field_escape(std::size_t pair_idx) const;

    void exclusive_close_field_escape(std::size_t pair_idx,
      const char_sequence_type &seq);
    const char_sequence_type &
      exclusive_close_field_escape(std::size_t pair_idx) const;

    void exclusive_escaped_field_escape(std::size_t pair_idx,
      const char_sequence_type &seq, std::size_t replace_idx);
    const char_sequence_type &
      exclusive_escaped_field_escape(std::size_t pair_idx) const;
    const char_sequence_type &
      exclusive_escaped_field_escape_replacement(std::size_t pair_idx) const;

    /* FIELD COLUMNS */
    bool restrict_field_columns(bool flag);
    bool restrict_field_columns(void) const;


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


    bool has_lookahead(void) const;
    std::pair<int,std::shared_ptr<char_sequence_type> >
      pop_lookahead(void) const;
    void push_lookahead(
      const std::pair<int,std::shared_ptr<char_sequence_type> > &val);



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
    // If \c num_cols is negative, then restrict_field_columns is true but a
    // full column count has not been calculated yet
    void effective_field_columns(int num_cols);
    int effective_field_columns(void) const;



    /* RESET EVERYTHING */
    void reset(void);





  private:
    struct escaped_field_exlusive_desc {
      char_sequence_type open_exlusive_sequence;
      char_sequence_type close_exlusive_sequence;
      char_sequence_type replacement_exlusive_sequence;
      std::size_t replacement_exlusive_index;
    };

    typedef std::vector<escaped_field_exlusive_desc>
      escaped_field_exclusive_desc_seq_type;

    log_callback_t _log_callback;
    void *_log_context;
    dsv_log_level _log_level;

    log_list_type log_list;

    expression_type _record_delimiters;
    char_sequence_type _exclusive_record_delimiter;

    expression_type _field_delimiters;
    char_sequence_type _exclusive_field_delimiter;

    escaped_field_desc_seq_type _field_escapes;
    escaped_field_exclusive_desc_seq_type _exclusive_field_escapes;
    int _selected_exclusive_field_escape;
    bool _exclusive_field_escape;

    int _effective_field_columns;
    bool _restrict_field_columns;
};

template<typename CharT>
inline parser<CharT>::parser(void) :_log_callback(0), _log_context(0),
  _log_level(dsv_log_none), _selected_exclusive_field_escape(-1),
  _exclusive_field_escape(false), _effective_field_columns(-1),
  _restrict_field_columns(false)
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
inline void parser<CharT>::reset(void)
{
// todo fix me
}

}

#endif
