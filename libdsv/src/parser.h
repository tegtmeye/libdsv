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
    ~parser(void) {std::cerr << "PARSER DESTROYED!\n";}

    log_callback_t log_callback(void) const;
    log_callback_t log_callback(log_callback_t fn);

    void * log_context(void) const;
    void * log_context(void *context);


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

    ssize_t effective_field_columns(void) const;
    ssize_t effective_field_columns(ssize_t num_cols);
    
    bool effective_field_columns_set(void) const;
    bool effective_field_columns_set(bool flag);

    bool effective_escaped_binary(void) const;
    bool effective_escaped_binary(bool flag);

    void reset(void);

  private:
    log_callback_t log_fn;
    void *lcontext;
  
    log_list_type log_list;

    dsv_newline_behavior newline_flag;
    ssize_t _field_columns;
    unsigned char field_delimiter;
    bool escaped_binary;

    dsv_newline_behavior effective_newline_flag;
    
    ssize_t _effective_field_columns;
    bool _effective_field_columns_set;
    bool _effective_escaped_binary;
    
};

inline parser::parser(void) :log_fn(0), lcontext(0), _field_columns(0), 
  field_delimiter(','), escaped_binary(false), _effective_field_columns(0),
  _effective_field_columns_set(false), _effective_escaped_binary(false)
{
  newline_behavior(dsv_newline_permissive);
}

inline log_callback_t parser::log_callback(void) const
{
  return log_fn;
}

inline log_callback_t parser::log_callback(log_callback_t fn)
{
  log_callback_t tmp = log_fn;
  log_fn = fn;
  return tmp;
}

inline void * parser::log_context(void) const
{
  return lcontext;
}

inline void * parser::log_context(void *context)
{
  void * tmp = lcontext;
  lcontext = context;
  return tmp;
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
  return field_delimiter;
}

inline unsigned char parser::delimiter(unsigned char d)
{
  unsigned char tmp = field_delimiter;
  field_delimiter = d;
  return tmp;
}

inline const dsv_newline_behavior &
parser::newline_behavior(void) const
{
  return newline_flag;
}

inline dsv_newline_behavior
parser::newline_behavior(dsv_newline_behavior behavior)
{
  dsv_newline_behavior tmp = newline_flag;
  newline_flag = behavior;
  effective_newline_flag = behavior;
  
  return tmp;
}

inline ssize_t parser::field_columns(void) const
{
  return _field_columns;
}

inline ssize_t parser::field_columns(ssize_t cols)
{
  ssize_t tmp = _field_columns;
  _field_columns = cols;
  _effective_field_columns = cols;
  
  _effective_field_columns_set = (cols > 0);
  
  return tmp;
}

inline bool parser::escaped_binary_fields(void) const
{
  return escaped_binary;
}

inline bool parser::escaped_binary_fields(bool flag)
{
  bool tmp = escaped_binary;
  escaped_binary = flag;
  return tmp;
}



inline dsv_newline_behavior parser::effective_newline(void) const
{
  return effective_newline_flag;
}

inline dsv_newline_behavior parser::effective_newline(dsv_newline_behavior val)
{
  dsv_newline_behavior tmp = effective_newline_flag;
  effective_newline_flag = val;
  return tmp;
}

inline ssize_t parser::effective_field_columns(void) const
{
  return _effective_field_columns;
}

inline ssize_t parser::effective_field_columns(ssize_t cols)
{
  ssize_t tmp = _effective_field_columns;
  _effective_field_columns = cols;
  return tmp;
}

inline bool parser::effective_field_columns_set(void) const
{
  return _effective_field_columns_set;
}

inline bool parser::effective_field_columns_set(bool flag)
{
  bool tmp = _effective_field_columns_set;
  _effective_field_columns_set = flag;
  return tmp;
}

inline bool parser::effective_escaped_binary(void) const
{
  return _effective_escaped_binary;
}

inline bool parser::effective_escaped_binary(bool flag)
{
  bool tmp = _effective_escaped_binary;
  _effective_escaped_binary = flag;
  return tmp;
}


inline void parser::reset(void)
{
  log_list.clear();
  effective_newline_flag = newline_flag;
  _effective_field_columns = _field_columns;
  _effective_field_columns_set = (_field_columns > 0);
}



}

#endif
