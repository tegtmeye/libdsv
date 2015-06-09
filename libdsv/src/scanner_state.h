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

#ifndef LIBDSV_PARSER_STATE_H
#define LIBDSV_PARSER_STATE_H

#include "dsv_parser.h"

#include <string>
#include <cstdio>
#include <memory>
#include <system_error>


#include <errno.h>

namespace detail {

  /**
   *  Object that holds the current state of the scanner. Also handles buffered reads.
   *
   */
  class scanner_state {
    public:
      enum sstate {
        start = 0,
        field,
        carrage_return,
        linefeed,
        nl_permissive,
        field_single_quote,
        field_double_quote,
        parse_error
      };

      scanner_state(const char *str, FILE *in=0, std::size_t buff_size=256);

      const char * filename(void) const;

      bool getc(unsigned char &c) const;
      bool advance(void);

      sstate seek_state(void) const;
      sstate seek_state(sstate s);


    private:
      std::string fname;
      std::shared_ptr<FILE> stream;

      sstate state;
      std::size_t buff_max;
      std::shared_ptr<char> buff;
      char *cur;
      char *end;

      bool refill(void);
  };

  inline scanner_state::scanner_state(const char *str, FILE *in, std::size_t buff_size)
    :fname(str), state(start), buff_max(buff_size),
    buff(new char[buff_size]), cur(buff.get()), end(buff.get())
  {
    if(!in) {
      errno = 0;
      if(!(in = fopen(str,"rb")))
        throw std::system_error(errno,std::system_category());
    }

    stream = std::shared_ptr<FILE>(in,&fclose);
  }

  inline const char * scanner_state::filename(void) const
  {
    return fname.c_str();
  }

  inline bool scanner_state::getc(unsigned char &c) const
  {
    if(cur == end)
      return false;

    c = *cur;
    return true;
  }

  inline bool scanner_state::advance(void)
  {
    if(cur == end && !refill())
      return false;

    ++cur;
    return true;
  }

  inline bool scanner_state::refill(void)
  {
    std::size_t result;

    // code adapted from flex non-posix fread
    errno=0;
    while ( (result = std::fread(buff.get(), 1, buff_max, stream.get()))==0 && std::ferror(stream.get())) {
      if( errno != EINTR) {
        throw std::system_error(errno,std::system_category());
      }
      errno=0;
      std::clearerr(stream.get());
    }

    cur = buff.get();
    end = cur + result;

    return cur != end;
  }

  inline scanner_state::sstate scanner_state::seek_state(void) const
  {
    return state;
  }

  inline scanner_state::sstate scanner_state::seek_state(scanner_state::sstate s)
  {
    sstate tmp = state;
    state = s;
    return tmp;
  }
}


#endif
