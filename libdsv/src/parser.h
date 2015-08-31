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

#include <string>
#include <list>
#include <utility>

#include <iostream>

namespace detail {

class log_description {
  private:
    typedef std::list<std::string> param_list_type;

  public:
    typedef param_list_type::const_iterator const_param_iterator;

    template<typename StringIteratorT>
    log_description(dsv_log_code c, StringIteratorT first, StringIteratorT last);

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



class parser {
  private:
    typedef std::list<std::pair<dsv_log_level,log_description> > log_list_type;

  public:
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
    unsigned char delimiter(void) const;
    unsigned char delimiter(unsigned char d);

    const dsv_newline_behavior & newline_behavior(void) const;
    dsv_newline_behavior newline_behavior(dsv_newline_behavior behavior);

    ssize_t field_columns(void) const;
    ssize_t field_columns(ssize_t num_cols);

    bool escaped_binary_fields(void) const;
    bool escaped_binary_fields(bool flag);


    /* non-exposed behaviors */
    dsv_newline_behavior effective_newline(void) const;
    dsv_newline_behavior effective_newline(dsv_newline_behavior val);

    bool escaped_field(void) const;
    bool escaped_field(bool val);

    ssize_t effective_field_columns(void) const;
    ssize_t effective_field_columns(ssize_t num_cols);

    bool effective_field_columns_set(void) const;
    bool effective_field_columns_set(bool flag);

    void reset(void);

  private:
    log_callback_t _log_callback;
    void *_log_context;
    dsv_log_level _log_level;

    log_list_type log_list;

    unsigned char _delimiter;
    dsv_newline_behavior _newline_behavior;
    ssize_t _field_columns;
    bool _escaped_binary_fields;

    dsv_newline_behavior _effective_newline;
    bool _escaped_field;
    ssize_t _effective_field_columns;
    bool _effective_field_columns_set;

};

inline parser::parser(void) :_log_callback(0), _log_context(0),
  _log_level(dsv_log_none),
  _delimiter(','), _field_columns(0), _escaped_binary_fields(false),
  _escaped_field(false), _effective_field_columns(0),
  _effective_field_columns_set(false)
{
  newline_behavior(dsv_newline_permissive);
}

inline log_callback_t parser::log_callback(void) const
{
  return _log_callback;
}

inline log_callback_t parser::log_callback(log_callback_t fn)
{
  std::swap(fn,_log_callback);
  return fn;
}

inline void * parser::log_context(void) const
{
  return _log_context;
}

inline void * parser::log_context(void *context)
{
  std::swap(context,_log_context);
  return context;
}

inline dsv_log_level parser::log_level(void) const
{
  return _log_level;
}

inline dsv_log_level parser::log_level(dsv_log_level level)
{
  std::swap(level,_log_level);
  return level;
}

inline std::size_t parser::log_size(void) const
{
  return log_list.size();
}

inline parser::const_log_iterator parser::log_begin(void) const
{
  return log_list.begin();
}

inline parser::const_log_iterator parser::log_end(void) const
{
  return log_list.end();
}

inline void parser::append_log(dsv_log_level level, const log_description &desc)
{
  log_list.push_back(std::make_pair(level,desc));
}

inline unsigned char parser::delimiter(void) const
{
  return _delimiter;
}

inline unsigned char parser::delimiter(unsigned char d)
{
  std::swap(d,_delimiter);
  return d;
}

inline const dsv_newline_behavior &
parser::newline_behavior(void) const
{
  return _newline_behavior;
}

inline dsv_newline_behavior
parser::newline_behavior(dsv_newline_behavior behavior)
{
  std::swap(behavior,_newline_behavior);
  _effective_newline = _newline_behavior;

  return behavior;
}

inline ssize_t parser::field_columns(void) const
{
  return _field_columns;
}

inline ssize_t parser::field_columns(ssize_t cols)
{
  std::swap(cols,_field_columns);
  _effective_field_columns = _field_columns;
  _effective_field_columns_set = (cols > 0);

  return cols;
}

inline bool parser::escaped_binary_fields(void) const
{
  return _escaped_binary_fields;
}

inline bool parser::escaped_binary_fields(bool flag)
{
  std::swap(flag,_escaped_binary_fields);
  return flag;
}



inline dsv_newline_behavior parser::effective_newline(void) const
{
  return _effective_newline;
}

inline dsv_newline_behavior parser::effective_newline(dsv_newline_behavior val)
{
  std::swap(val,_effective_newline);
  return val;
}

inline bool parser::escaped_field(void) const
{
  return _escaped_field;
}

inline bool parser::escaped_field(bool val)
{
  std::swap(val,_escaped_field);
  return val;
}

inline ssize_t parser::effective_field_columns(void) const
{
  return _effective_field_columns;
}

inline ssize_t parser::effective_field_columns(ssize_t cols)
{
  std::swap(cols,_effective_field_columns);
  return cols;
}

inline bool parser::effective_field_columns_set(void) const
{
  return _effective_field_columns_set;
}

inline bool parser::effective_field_columns_set(bool flag)
{
  std::swap(flag,_effective_field_columns_set);
  return flag;
}


inline void parser::reset(void)
{
  log_list.clear();
  _effective_newline = _newline_behavior;
  _escaped_field = false;
  _effective_field_columns = _field_columns;
  _effective_field_columns_set = (_field_columns > 0);
}



}

#endif
