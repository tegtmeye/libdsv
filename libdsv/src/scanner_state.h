/*
 Copyright (c) 2014-2016, Michael B. Tegtmeyer. All rights reserved.

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

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIBDSV_PARSER_STATE_H
#define LIBDSV_PARSER_STATE_H

#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <system_error>

#include <cerrno>

#include <iostream>

namespace detail {

  /**
      Object that holds the current state of the scanner.
      Also handles buffered reads. That is, we could have just used C
      functionality but the buffering scheme is implementation dependent so we
      buffer on our own here.

      - - - - - - - - - - - - - - - - - - - - - - - - - - -
      ^           ^                 ^               ^
      A           B                 C               *
      <  token  > <   lookahead   > <   putback   >

      There are 3 bufferes associated with the scanner

      - token buffer [A,B): The buffer associated with the most recent
      accepted token. This is emptied when the scanner passes the token off to
      the parser. That is, A = B after handoff.

      - lookahead buffer [B,C): The lookahead buffer is the buffer that
      represents the "next" token read. Although technically not
      necessary, having a lookahead buffer prevents a token from being
      read twice unnecessarily. For a single byte token, the cost of
      reading the token twice is negligible but for complex tokens this
      can have a non-trivial cost impact. For example, suppose that we
      are currently parsing an escaped text field. The token that ends
      the escaped field is the CLOSE_FIELD_ESCAPE. Once found, the
      scanner returns a FIELDDATA token. The issue is that the next
      token that needs to be returned is the CLOSE_FIELD_ESCAPE which
      has already been read. Although we could just put the bytes back
      to be read again, if this token is complex (multibyte, many
      repeats) then we've added an unnecessary amount of overhead.
      Instead it is kept and just returned.

      - putback buffer [C,*) where the * is the current read location: This
      is the buffer that that represents the mid-acceptance number of bytes
      during scanning and is most used for repeats where the the token is
      accepted but could potentially be longer. For example, if the token
      sequence is "foo" repeated an infinite number of times, and the input
      sequence is "foofoofobar", then mid-scanning, [B,C) contains the first
      "foofoo" and the read location (*) points to "f". After scanning 'f' and
      'o', it gets 'b' which isn't another sequence of "foo" and puts back the
      "fo" to be read again.
   */
  template<typename ByteT>
  class basic_scanner_state {
    public:
      typedef ByteT byte_type;
      typedef typename std::vector<byte_type>::iterator iterator;
      typedef typename std::vector<byte_type>::const_iterator const_iterator;

      /*
          \c min_buff_size must be at least twice as large as the largest
          possible putback size or scanning will infinite loop! The largest
          putback size is a known constant for regular scanners though.
      */
      basic_scanner_state(const char *str, FILE *in=0,
        std::size_t min_buff_size=256);

      const char * filename(void) const;

      /*
          get character and advance the read location

          Get the current character from the input and advance the read
          location but do not advance the putback buffer. That is, calling
          advancec multiple consecutive times will not return the same value
          until the end of stream is reached and then EOF is returned.
       */
      int getc(void);

      /*
          Set the putback marker to the current read position. That is, in a
          call sequence of:
            set_lookahead();
            a = getc();
            putback();
            b = getc();

            'a' and 'b' will have the same value

          The \c identifier parameter is the id associated with the lookahead.
          For example, if the lookahead is parsed as a string of ASCII chars
          with the \c identifier ASCII_STR, then when the previous token
          buffer is accepted, ASCII_STR is returned to indicate that the
          lookahead buffer was of this type
       */
      void set_lookahead(int identifier = 0);

      /*
          Putback \c nbytes from the putback buffer to be read again. If the
          putback buffer is empty, this has no effect.
      */
      void putback(std::size_t nbytes);

      /*
          The [first,last) bytes representing the token buffer. The return
          values are only guaranteed to point to valid locations until the
          next call to \c getc, \c set_lookahead, or \c accept
      */
      std::pair<const_iterator,const_iterator> token(void) const;

      /*
          Accept the token currently contained in the token buffer. This
          empties the token buffer by setting A to B and B to C.

          Returns the readahead (now token) buffer identifier previously set
          with set_lookahead()
      */
      int accept(void);

      /*
          Return nonzero if the input has reached EOF. That is, any additional
          calls to [f]getc() will return EOF.

          NB that this behaves like normal C feof(). That is, \c eof() will not
          return true until the stream is queried and not character is found.
          For example, for a newly initialized but empty stream, \c eof()
          will return false until a \c getc() is called (which will return
          EOF) and then subsequent calls to \c eof() will return true. This
          is a limitation (or feature depending on how you look at it) of normal
          unix end-of-file handling
      */
      bool eof(void) const;

    private:
      std::string fname;
      std::shared_ptr<FILE> stream;

      std::vector<byte_type> buff;
      int _readahead_id;

      std::size_t min_buf_sz;

      std::size_t token_off;      // "A" in the example
      std::size_t lookahead_off;  // "B" in the example
      std::size_t putback_off;    // "C" in the example
      std::size_t read_off;       // "*" in the example
      std::size_t end_off;

      /*
          Refill the buffer returning whether or not we reached EOF
      */
      bool refill(void);
  };

  template<typename ByteT>
  basic_scanner_state<ByteT>::basic_scanner_state(const char *str, FILE *in,
    std::size_t min_buff_size) :buff(min_buff_size,0), _readahead_id(0),
      min_buf_sz(min_buff_size), token_off(0), lookahead_off(0), putback_off(0),
      read_off(0), end_off(0)
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

  template<typename ByteT>
  inline const char * basic_scanner_state<ByteT>::filename(void) const
  {
    return fname.c_str();
  }

  template<typename ByteT>
  inline int basic_scanner_state<ByteT>::getc(void)
  {
    if(read_off == end_off && !refill())
      return EOF;

    return buff[read_off++];
  }

  template<typename ByteT>
  inline void basic_scanner_state<ByteT>::set_lookahead(int identifier)
  {
    putback_off = read_off;
    _readahead_id = identifier;
  }

  template<typename ByteT>
  inline void basic_scanner_state<ByteT>::putback(std::size_t nbytes)
  {
    if((read_off-putback_off) < nbytes) {
      std::cerr << "Attempt to putback " << nbytes << " when there is only "
        "room for " << (read_off-putback_off) << "\n";
      assert((read_off-putback_off) >= nbytes);
    }

    read_off -= nbytes;
  }

  template<typename ByteT>
  inline std::pair<typename basic_scanner_state<ByteT>::const_iterator,
    typename basic_scanner_state<ByteT>::const_iterator>
  basic_scanner_state<ByteT>::token(void) const
  {
    return std::make_pair(buff.begin()+token_off,buff.begin()+lookahead_off);
  }

  template<typename ByteT>
  inline int basic_scanner_state<ByteT>::accept(void)
  {
    token_off = lookahead_off;
    lookahead_off = putback_off;
    return _readahead_id;
  }


  template<typename ByteT>
  inline bool basic_scanner_state<ByteT>::eof(void) const
  {
    return (read_off == end_off) && std::feof(stream.get());
  }

  /**
      IMPORTANT! Buff MUST be bigger than twice the minimum putback size
      or this will likely infinite loop!
   */
  template<typename ByteT>
  bool basic_scanner_state<ByteT>::refill(void)
  {
    // Push the buffer to the beginning and reset the offset values
    //todo investigate how much we really refill with a nonempty putback buffer
    if(token_off != 0) {
      std::size_t active_len = (read_off - token_off);
      // reset the lookahead offset and mark offset to the beginning
      lookahead_off = (lookahead_off - token_off);
      putback_off = (putback_off - token_off);
      std::move(buff.begin()+token_off,buff.begin()+read_off,buff.begin());
      token_off = 0;
      read_off = end_off = active_len;
    }

    std::size_t avail_len = buff.size()-read_off;

    // ensure we are not left with a buffer smaller than the min buffer
    if(avail_len < min_buf_sz) {
      buff.resize(read_off+min_buf_sz,0);
      avail_len = min_buf_sz;
    }

    // code adapted from flex non-posix fread
    errno=0;
    std::size_t len;
    while ((len = std::fread(buff.data()+read_off,1,avail_len,stream.get()))==0
      && std::ferror(stream.get()))
    {
      if( errno != EINTR) {
        throw std::system_error(errno,std::system_category());
      }
      errno=0;
      std::clearerr(stream.get());
    }

    end_off = read_off + len;

    return read_off != end_off;
  }

  typedef basic_scanner_state<unsigned char> scanner_state;
}


#endif
