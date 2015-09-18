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
#include <vector>
#include <cstdio>
#include <memory>
#include <system_error>

#include <cerrno>

namespace detail {

  /**
   *  Object that holds the current state of the scanner. Also handles buffered reads.
   *
   */
  class scanner_state {
    public:
      scanner_state(const char *str, FILE *in=0, std::size_t buff_size=256);

      const char * filename(void) const;

      /*
          Get the current character from the input. Do not advance the read
          location. That is, getc can be called multiple consecutive times with
          the same return value.
       */
      int getc(void);

      /*
          Get the current character from the input and advance the read
          location. That is, calling advancec multiple consecutive times will
          not return the same value until the end of stream is reached and then
          EOF is returned.

          Do not adjust the putback buffer
       */
      int advancec(void);

      /*
          Forget any putback buffer, get the current character from the input,
          and advance the read location. That is, calling fadvancec multiple
          consecutive times will not return the same value until the end of
          stream is reached and then EOF is returned.
       */
      int fadvancec(void);

      /*
          Putback any bytes from the putback buffer to be read again. If the
          putback buffer is empty, this has no effect.
      */
      void putback(void);

      /*
          Forget the putback buffer if any exists
      */
      void forget(void);

    private:
      std::string fname;
      std::shared_ptr<FILE> stream;

      std::vector<unsigned char> buff;
      std::size_t begin_off;
      std::size_t cur_off;
      std::size_t end_off;

      bool refill(void);
  };

  inline scanner_state::scanner_state(const char *str, FILE *in,
    std::size_t buff_size) :buff(buff_size), begin_off(0), cur_off(0),
    end_off(0)
  {
    if(str)
      fname = str;

    if(!in) {
      errno = 0;
      in = fopen(str,"rb");
      if(!in) {
        throw std::system_error(errno,std::system_category());
      }
    }

    stream = std::shared_ptr<FILE>(in,&fclose);
  }

  inline const char * scanner_state::filename(void) const
  {
    return fname.c_str();
  }

  inline int scanner_state::getc(void)
  {
    if(cur_off == end_off && !refill())
      return EOF;

    return buff.at(cur_off);
  }

  inline int scanner_state::advancec(void)
  {
    int result = getc();

    if(result != EOF)
      ++cur_off;

    return result;
  }

  inline int scanner_state::fadvancec(void)
  {
    int result = getc();

    if(result != EOF)
      begin_off = ++cur_off;

    return result;
  }

  inline void scanner_state::putback(void)
  {
    cur_off = begin_off;
  }

  inline void scanner_state::forget(void)
  {
    begin_off = cur_off;
  }


  /**
      IMPORTANT! Buff MUST be bigger than twice the minimum putback size
      or this will likely infinite loop!
   */
  inline bool scanner_state::refill(void)
  {
//     std::cerr << "(Pre) begin_off (" << begin_off << "); cur_off ("
//       << cur_off << "); end_off (" << end_off << "); putback contains:\n [[";
//     for(std::size_t i=begin_off; i<cur_off; ++i) {
//       if(i>begin_off)
//         std::cerr << " ";
//       std::cerr << "'" << buff.at(i) << "'";
//     }
//     std::cerr << "]]\n";

    // move the putback buffer to the beginning of the read buffer
    // and reset the offset values
    if(begin_off != 0) {
      std::size_t putback_len = (cur_off - begin_off);
      std::move(buff.begin()+begin_off,buff.begin()+cur_off,buff.begin());
      begin_off = 0;
      cur_off = end_off = putback_len;

//       std::cerr << "(Move) begin_off (" << begin_off << "); cur_off ("
//         << cur_off << "); end_off (" << end_off << "); putback contains:\n [[";
//       for(std::size_t i=begin_off; i<cur_off; ++i) {
//         if(i>begin_off)
//           std::cerr << " ";
//         std::cerr << "'" << buff.at(i) << "'";
//       }
//       std::cerr << "]]\n";
    }

    // code adapted from flex non-posix fread
    errno=0;
    std::size_t len;
    std::size_t buf_len = buff.size()-cur_off;
    while ((len = std::fread(buff.data()+cur_off,1,buf_len,stream.get()))==0
      && std::ferror(stream.get()))
    {
      if( errno != EINTR) {
        throw std::system_error(errno,std::system_category());
      }
      errno=0;
      std::clearerr(stream.get());
    }

    end_off = cur_off + len;

//     std::cerr << "(Final) begin_off (" << begin_off << "); cur_off ("
//       << cur_off << "); end_off (" << end_off << "); putback contains:\n [[";
//     for(std::size_t i=begin_off; i<cur_off; ++i) {
//       if(i>begin_off)
//         std::cerr << " ";
//       std::cerr << "'" << buff.at(i) << "'";
//     }
//     std::cerr << "]] ";
//     for(std::size_t i=cur_off; i<end_off; ++i)
//       std::cerr << " '" << buff.at(i) << "'";
//     std::cerr << "\n";

    return cur_off != end_off;
  }
}


#endif
