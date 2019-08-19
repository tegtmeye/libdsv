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

#ifndef LIBDSV_DETAIL_READ_BYTES_H
#define LIBDSV_DETAIL_READ_BYTES_H

#include "scanner_state.h"
#include "equiv_bytesequence.h"


namespace detail {

/**
    Convenience function for parser_lex

    Try to read the byte seq from the scanner input if possible. If
    successful, the current read position points to the first byte not
    part of [first,last] and the read bytes remain in the putback
    buffer. If not succesful, the read position remains unchanged.

    Handles case 1.
 */
template<typename ByteT, typename ForwardIterator>
inline std::size_t read_bytes(basic_scanner_state<ByteT> &scanner,
  ForwardIterator first, ForwardIterator last, bool repeat)
{
  std::size_t result = 0;

  ForwardIterator cur;
  do {
    cur = first;

    // use the comma operator because getc causes a read even if no match

    int byte;
    std::size_t read_bytes = 0;
    while(cur != last && (byte=scanner.getc()) != EOF
      && (++read_bytes, byte == static_cast<int>(*cur)))
    {
      ++cur;
    }

    if(cur != last) { // read some but not whole sequence
      scanner.putback(read_bytes);

      break;
    }

    result += (last-first);
  } while(repeat && !scanner.eof());

  return result;
}

/**
    Convenience function for parser_lex

    Forget any existing putback buffer and try to read any possible byte seq
    contained in the compiled structure \c comp_byte_seq from the scanner input
    if possible. If successful, the current read position points to the first
    byte not part of the successful comp_byte_seq read and the read bytes remain
    in the putback buffer. If not succesful, the read position remains
    unchanged.

    Since this is core scanning, the use of the unconventional goto statements
    are there for efficiency. Probably better that it was done in assembler...
 */
template<typename ByteT>
std::size_t read_bytes(basic_scanner_state<ByteT> &scanner,
  const std::vector<detail::byte_chunk> &comp_byte_seq, bool repeat)
{
  std::vector<detail::byte_chunk>::const_iterator chunk_iter;

  assert(!comp_byte_seq.empty());

  int in;
  std::size_t result = 0;
  std::size_t read_bytes;

  seq_repeat:
    chunk_iter = comp_byte_seq.begin();

    read_bytes = 0;

    read:
      in = scanner.getc();
      if(in == EOF)
        goto done;

      ++read_bytes;

    loop:
      if(in != chunk_iter->byte) {
        if(!chunk_iter->fail_skip)
          goto done;

        chunk_iter += chunk_iter->fail_skip;
        goto loop;
      }

      // in matches
      if(chunk_iter->accept) {
        result += read_bytes;
        read_bytes = 0;
      }

      if(chunk_iter->pass_skip) {
        chunk_iter += chunk_iter->pass_skip;
        goto read;
      }

      if(repeat)
        goto seq_repeat;

      // just return as read_bytes is (should) 0
      assert(read_bytes == 0);
      return result;

    done:
      // putback any partially acceptable sequences
      scanner.putback(read_bytes);
      return result;
}

}


#endif
