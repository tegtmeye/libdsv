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

    std::size_t log_size(void) const;
    const_log_iterator log_begin(void) const;
    const_log_iterator log_end(void) const;

    void append_log(dsv_log_level level,const log_description &desc);

    unsigned char delimiter(void) const;
    unsigned char delimiter(unsigned char d);

    const dsv_newline_behavior & newline_behavior(void) const;
    dsv_newline_behavior newline_behavior(dsv_newline_behavior behavior);

    dsv_newline_behavior effective_newline(void) const;
    dsv_newline_behavior effective_newline(dsv_newline_behavior val);

    bool reject_nonprinting(void) const;
    bool reject_nonprinting(bool flag);

  private:
    log_list_type log_list;

    dsv_newline_behavior newline_flag;
    dsv_newline_behavior effective_newline_flag;
    
    unsigned char field_delimiter;
    bool reject_nonprinting_flag;
};

inline parser::parser(void) :newline_flag(dsv_newline_permissive),
  effective_newline_flag(dsv_newline_permissive), field_delimiter(','),
  reject_nonprinting_flag(true)
{
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

inline void parser::append_log(dsv_log_level level,const log_description &desc)
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

inline dsv_newline_behavior parser::effective_newline(void) const
{
  return effective_newline_flag;
}

inline dsv_newline_behavior parser::effective_newline(dsv_newline_behavior val)
{
  if(effective_newline_flag != dsv_newline_permissive)
    throw std::runtime_error("SET EFFECTIVE NEWLINE WHEN IT SHOULDN'T");
  
  dsv_newline_behavior tmp = effective_newline_flag;
  effective_newline_flag = val;
  return tmp;
}

inline bool parser::reject_nonprinting(void) const
{
  return reject_nonprinting_flag;
}

inline bool parser::reject_nonprinting(bool flag)
{
  bool tmp = reject_nonprinting_flag;
  reject_nonprinting_flag = flag;
  return tmp;
}
}

#endif
