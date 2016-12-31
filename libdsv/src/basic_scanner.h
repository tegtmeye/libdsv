/*
 Copyright (c) 2014, Mike Tegtmeyer All rights reserved.

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

#ifndef LIBDSV_BASIC_SCANNER_H
#define LIBDSV_BASIC_SCANNER_H

#include <boost/iterator/iterator_facade.hpp>

#include <string>
#include <vector>
#include <iterator>
#include <cstdio>
#include <system_error>

// uncomment or add to compilation to disable checking
//#define NDEBUG
#include <cassert>

#ifndef DEFAULT_SCANNER_READ_SIZE
#define DEFAULT_SCANNER_READ_SIZE 2048
#endif


namespace detail {

template<typename CharT>
class basic_scanner {
  public:
    typedef CharT char_type;

    basic_scanner(const char *path, FILE *in=0,
      std::size_t read_size=DEFAULT_SCANNER_READ_SIZE);
    basic_scanner(const basic_scanner &) = delete;
    ~basic_scanner(void);

    basic_scanner & operator=(const basic_scanner &) = delete;

    const std::string & path(void) const;

    bool eof(void) const;

    /*
        get the next CharT from the input stream and adds it to the
        cache. Returns EOF if stream reaches end-of-file.
    */
    int getc(void);

    const CharT & at_cache(std::size_t n) const;

    std::size_t cache_size(void) const;

    void clear_cache(std::size_t n);

  private:
    friend class scanner_iterator;

    typedef std::vector<CharT> buffer_type;

    std::string _path;
    FILE *_stream;

    buffer_type _buff;
    std::size_t _start_offset;
    std::size_t _read_offset;
    std::size_t _end_offset;
    bool _eof;

    std::size_t _read_size;
    bool _should_close;

    bool refill(void);
};

template<typename CharT>
basic_scanner<CharT>::basic_scanner(const char *str, FILE *in,
    std::size_t read_size) :_stream(in), _buff(read_size,0),
    _start_offset(0),
    _read_offset(0),
    _end_offset(0),
    _eof(false),
    _read_size(read_size),
    _should_close(in == 0)
{
  if(str)
    _path = str;

  if(!in) {
    errno = 0;
    in = fopen(str,"rb");
    if(!in) {
      throw std::system_error(errno,std::system_category());
    }
  }
}

template<typename CharT>
inline basic_scanner<CharT>::~basic_scanner(void)
{
  if(_should_close)
    fclose(_stream);
}


template<typename CharT>
inline const std::string & basic_scanner<CharT>::path(void) const
{
  return _path;
}

template<typename CharT>
inline bool basic_scanner<CharT>::eof(void) const
{
  return _eof;
}

template<typename CharT>
inline int basic_scanner<CharT>::getc(void)
{
  if(_read_offset == _end_offset && (std::feof(_stream) || !refill())) {
    _eof = true;
    return EOF;
  }

  return _buff[_read_offset++];
}

template<typename CharT>
inline const CharT & basic_scanner<CharT>::at_cache(std::size_t n) const
{
  assert(n < (_read_offset-_start_offset));

  return _buff[_start_offset+n];
}

template<typename CharT>
std::size_t basic_scanner<CharT>::cache_size(void) const
{
  return (_read_offset - _start_offset);
}

template<typename CharT>
void basic_scanner<CharT>::clear_cache(std::size_t n)
{
  assert(_start_offset+n <= _read_offset);

  _start_offset += n;
}

template<typename CharT>
bool basic_scanner<CharT>::refill(void)
{
  std::size_t content_size = (_end_offset-_start_offset);
  if(!content_size) {
  // if no cache, just skip to refill
    _start_offset = _read_offset = 0;
  }
  else if(_buff.size() - _end_offset < _read_size) {
    // need to refill but not enough storage...
    if(_buff.size()-content_size < _read_size) {
      // not enough room to even with compaction, make new storage
      buffer_type tmp(content_size+_read_size);
      std::copy(_buff.begin()+_start_offset,_buff.begin()+_end_offset,
        tmp.begin());
      std::swap(tmp,_buff);
    }
    else {
      // just compact
      std::move(_buff.begin()+_start_offset,_buff.begin()+_end_offset,
        _buff.begin());
    }

    _start_offset=0;
    _read_offset = _end_offset = content_size;
  }

  // code adapted from flex non-posix fread
  errno=0;
  std::size_t len;
  while ((len = std::fread(_buff.data()+_read_offset,1,_read_size,_stream))==0
    && std::ferror(_stream))
  {
    if(errno != EINTR) {
      throw std::system_error(errno,std::system_category());
    }
    errno=0;
    std::clearerr(_stream);
  }

  _end_offset = _read_offset + len;

  return _read_offset != _end_offset;
}













template<typename CharT>
class basic_scanner_iterator
  :public boost::iterator_facade<basic_scanner_iterator<CharT>,
    const CharT,boost::bidirectional_traversal_tag>
{
  public:
    basic_scanner_iterator(void);
    basic_scanner_iterator(basic_scanner<CharT> &scanner);

    const CharT & dereference(void) const;

    bool equal(const basic_scanner_iterator<CharT> &rhs) const;

    void increment(void);

    void decrement(void);

  private:
    basic_scanner<CharT> *_scanner;
    std::size_t _offset;
    CharT _value;
};

template<typename CharT>
inline basic_scanner_iterator<CharT>::basic_scanner_iterator(void)
  :_scanner(0), _offset(0), _value(0)
{
}

template<typename CharT>
inline basic_scanner_iterator<CharT>::basic_scanner_iterator(
  basic_scanner<CharT> &scanner) :_scanner(&scanner), _offset(0), _value(0)
{
  if(!(_scanner->cache_size() == 0 && _scanner->getc() == EOF)) {
    _offset = _scanner->cache_size()-1;
    _value = _scanner->at_cache(_offset);
  }
}

template<typename CharT>
inline const CharT & basic_scanner_iterator<CharT>::dereference(void) const
{
  return _value;
}

template<typename CharT>
inline bool basic_scanner_iterator<CharT>::equal(
  const basic_scanner_iterator<CharT> &rhs) const
{
  bool val = ((_scanner == rhs._scanner) && (_offset == rhs._offset)) ||
    (_scanner == 0 && rhs._offset >= rhs._scanner->cache_size()) ||
    (rhs._scanner == 0 && _offset >= _scanner->cache_size());

  return val;
}

/*
    if the advanced iterator references a cached value in \c scanner,
    then set _value to the cached value in scanner. If the advanced
    iterator references one past the last cached value, then call \c
    _scanner->getc() to try and get the next value. If it is EOF, then
    mark the iterator to compare equal to the end-of-file
    scanner_iterator, otherwise set \c _value to the new scanner value.

    Dereferencing or incrementing the iterator once it equals the
    end-of-stream iterator invokes undefined behavior
*/
template<typename CharT>
inline void basic_scanner_iterator<CharT>::increment(void)
{
  if(!(++_offset >= _scanner->cache_size() && _scanner->getc() == EOF))
    _value = _scanner->at_cache(_offset);
}

template<typename CharT>
inline void basic_scanner_iterator<CharT>::decrement(void)
{
  _value = _scanner->at_cache(--_offset);
}



















}

#endif
