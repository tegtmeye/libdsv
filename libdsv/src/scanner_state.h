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

#include <iostream>

namespace detail {

  /**
   *  Object that holds the current state of the scanner.
   *  Also handles buffered reads. That is, we could have just used C
   *  functionality but the buffering scheme is implementation dependent so we
   *  buffer on our own here.
   *
   */
  class scanner_state {
    public:
      scanner_state(const char *str, FILE *in=0, std::size_t min_buff_size=256);

      const char * filename(void) const;

      /*
          get character and advance the read location

          Get the current character from the input and advance the read
          location but do not advance the putback buffer. That is, calling
          getc multiple consecutive times will not return the same value
          until the end of stream is reached and then EOF is returned.
       */
      int getc(void);

      /*
          forget any mark and putback buffers, get character, and advance the
          read location

          Forget any putback buffer, get the current character from the input,
          advance the read location, set the mark location to the read location.
          That is, calling fgetc multiple consecutive times will not return the
          same value until the end of stream is reached and then EOF is returned
          regardless if the the (f)getc was interlaced with a call to \c putback
          or \c putbackmark
       */
      int fgetc(void);

      /*
          Set the putback marker to the current read position. That is, in a
          call sequence of:
            setmark();
            a = getc();
            putback();
            b = getc();

            'a' and 'b' will have the same value
       */
      void setmark(void);

      /*
          Putback any bytes from the putback buffer up to the mark location to
          be read again then set the current read location to the mark location.
      */
      void putbackmark(void);


      /*
          Putback any bytes from the putback buffer to be read again. If the
          putback buffer is empty, this has no effect. This also sets the
          putbackmark to the current read location.
      */
      void putback(void);

      /*
          Forget the putback buffer and mark if any exists. That is, set the
          putback location and mark location to the read location.
      */
      void forget(void);

      /*
          Return the number of bytes in the putback buffer
      */
      std::size_t putback_len(void) const;

      /*
          Return nonzero if the input has reached EOF. That is, any additional
          calls to [f]getc() will return EOF.
      */
      bool eof(void) const;

    private:
      std::string fname;
      std::shared_ptr<FILE> stream;

      std::size_t min_buf_sz;
      std::vector<unsigned char> buff;
      std::size_t begin_off;
      std::size_t mark_off;
      std::size_t cur_off;
      std::size_t end_off;

      bool refill(void);
  };

  inline scanner_state::scanner_state(const char *str, FILE *in,
    std::size_t min_buff_size) :min_buf_sz(min_buff_size),
    buff(min_buff_size,0), begin_off(0), mark_off(0), cur_off(0), end_off(0)
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

    return buff[cur_off++];
  }

  inline int scanner_state::fgetc(void)
  {
    forget();

    return getc();
  }

  inline void scanner_state::setmark(void)
  {
    mark_off = cur_off;
  }

  inline void scanner_state::putbackmark(void)
  {
    cur_off = mark_off;
  }

  inline void scanner_state::putback(void)
  {
    cur_off = begin_off;
    mark_off = begin_off;
  }

  inline void scanner_state::forget(void)
  {
    begin_off = cur_off;
    mark_off = cur_off;
  }

  inline std::size_t scanner_state::putback_len(void) const
  {
    return (cur_off - begin_off);
  }

  inline bool scanner_state::eof(void) const
  {
    return std::feof(stream.get());
  }

  /**
      IMPORTANT! Buff MUST be bigger than twice the minimum putback size
      or this will likely infinite loop!
   */
  inline bool scanner_state::refill(void)
  {
// std::cerr << "REFILL\n";
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

    //todo investigate how much we really refill with a nonempty putback buffer
    if(begin_off != 0) {
      std::size_t putback_len = (cur_off - begin_off);
      // reset mark_off to distance from beginning
      mark_off = (mark_off - begin_off);
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

    // ensure we are not left with a buffer smaller than the min buffer
    if(buf_len < min_buf_sz) {
      buff.resize(cur_off+min_buf_sz,0);
      buf_len = min_buf_sz;
    }

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
