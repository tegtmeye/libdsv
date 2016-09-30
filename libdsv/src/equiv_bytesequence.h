/*
 Copyright (c) 2015, Mike Tegtmeyer
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

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

#ifndef LIBDSV_BYTESEQUENCE_H
#define LIBDSV_BYTESEQUENCE_H

#include <vector>
#include <memory>

#include <cassert>
#include <iostream>

namespace detail {

struct byte_chunk {
  int byte;
  int accept; // nonzero means accept
  std::ptrdiff_t pass_skip; // repeats can cause the skip to be negative
  std::size_t fail_skip;  // fail_skip is always positive

  byte_chunk(int b, int id, std::ptrdiff_t ps, std::size_t fs)
    :byte(b), accept(id), pass_skip(ps), fail_skip(fs) {}
};

/**
 *  Return true if the sequence [first2,last2) consists of wholly repeated
 *  occurrences of [firt1,last1)
 */
template<typename BiDirectionalIterator>
bool is_repetative(BiDirectionalIterator first1, BiDirectionalIterator last1,
  BiDirectionalIterator first2, BiDirectionalIterator last2)
{
  while(first2 != last2) {
    BiDirectionalIterator cur = first1;

    while(cur != last1 && first2 != last2 && *cur == *first2) {
      ++cur;
      ++first2;
    }

    if(cur != last1)
      return false;
  }

  return true;
}

/**
 *  Inspect each bytesequence_desc and extend the seq_bytes with
 *  repeated instances of base_seq_bytes to ensure the tail of seq_bytes
 *  contains an instance of base_seq_bytes that shares no bytes with any other
 *  bytesequence_desc. That is; if bytesequence_desc (A) contains a
 *  base_seq_bytes of "foo" and bytesequence_desc (B) contains a base_seq_bytes
 *  of "foobar", the resulting compiled_sequence of (A) will contain "foofoo"
 *  [the last "foo" is not shared with any other bytesequence_desc] and the
 *  resulting bytesequence of (B) will contain "foobarfoobar" [the last "foobar"
 *  is has no shared content with "foofoo"]. This logic is extended to
 *  arbitrarily complex strings.
 *
 *  If a bytesequence_desc is made up of whole instances of another
 *  bytesequence_desc, an exception is thrown. For example (A) is "foo" and (B)
 *  is "foofoo".
 */
template<typename ByteSeqDescIter>
void normalize_seq(ByteSeqDescIter desc_first, ByteSeqDescIter desc_last)
{
  bool recheck=true;
  while(recheck) {
    recheck = false;

    // cur is the sequence under inspection
    for(ByteSeqDescIter cur=desc_first; cur != desc_last; ++cur) {
      if(!cur->repeat)
        continue;

      for(ByteSeqDescIter insp=desc_first; insp != desc_last; ++insp) {
        if(insp == cur)
          continue;

        assert(!is_repetative(cur->seq_bytes.begin(),cur->seq_bytes.end(),
          insp->seq_bytes.begin(),insp->seq_bytes.end()));


        if(cur->seq_bytes.size() < insp->seq_bytes.size()) {
          auto res = std::mismatch(cur->seq_bytes.begin(),cur->seq_bytes.end(),
            insp->seq_bytes.begin());

          std::size_t shared_len = res.first - cur->seq_bytes.begin();

          if(shared_len > cur->seq_bytes.size() - cur->base_seq_bytes.size()) {

            std::vector<unsigned char>
              new_delim(cur->seq_bytes);
            new_delim.insert(new_delim.end(),cur->base_seq_bytes.begin(),
              cur->base_seq_bytes.end());

            std::swap(cur->seq_bytes,new_delim);

            recheck = true;
            break;
          }
        }
        else {
          auto res = std::mismatch(insp->seq_bytes.begin(),
            insp->seq_bytes.end(),cur->seq_bytes.begin());

          std::size_t shared_len = res.first - insp->seq_bytes.begin();

          if(shared_len > cur->seq_bytes.size() - cur->base_seq_bytes.size()) {

            std::vector<unsigned char>
              new_delim(cur->seq_bytes);
            new_delim.insert(new_delim.end(),cur->base_seq_bytes.begin(),
              cur->base_seq_bytes.end());

            std::swap(cur->seq_bytes,new_delim);

            recheck = true;
            break;
          }
        }
      }
    }
  }
}

/**
 *  Return the compiled byte sequence from \c first to \c last. All the compiled
 *  byte_chunks are contiguous
 */
template<typename ByteIter>
std::vector<byte_chunk> assign_bytes(ByteIter first, ByteIter last,
  std::size_t base_len, bool repeat)
{
  if(first == last)
    return std::vector<byte_chunk>();

  assert(base_len);
  std::vector<byte_chunk> result;

  for(std::size_t i=1; first != last; ++i, ++first) {
    result.emplace_back(*first,0,1,0);
    if((i % base_len) == 0)
      result.back().accept = 1;
  }

  if(repeat)
    result.back().pass_skip = -base_len;
  else
    result.back().pass_skip = 0;

  return result;
}

/**
 *  Add the compiled byte sequence [first,last) into \c byte_sequence such that
 *  the minimum number of comparisons are necessary to traverse the resulting
 *  \c byte_sequence.
 *
 *  ByteIter is a model of a ForwardIterator to an intergral type
 *  {char, unsigned char, int, etc}
 *
 *  ByteChunkSequence is a model is RandomAccessSequence
 *
 *  \c base_len is the length of the stub of [first,last). That is, of the
 *  original sequence was 'foo' and it was normalied to 'foofoofoo', then the
 *  base-length is the length of the original sequence---or 3.
 *
 *  id is the identifier associated with this sequence
 *
 *  Precondition: byte_seq is a non-empty random access container of \c
 *  byte_chunks
 */
template<typename ByteIter, typename ByteChunkSequence>
void compile_bytes(ByteIter first, ByteIter last, std::size_t base_len,
  bool repeat, ByteChunkSequence &byte_sequence)
{
//   std::cerr << "Called compile_bytes\n";

  assert(!byte_sequence.empty());

  enum TestType {
    none = 0,
    pass,
    fail
  };

  //  byte_off is the offset into the delim_byte under inspection if any.
  //  last_off is the offset into the delim_byte that pointed to the delim_byte
  //  under inspection. If byte_off == last_off then there is no current
  //  follow-on delim_byte and one needs to be added. test_result is the result
  //  of the last test.
  ByteIter start = first;
  std::size_t byte_off = 0;
  std::size_t last_off = std::numeric_limits<std::size_t>::max();
  TestType test_result = none;
  while(first != last) {
    if(last_off == byte_off) {
// std::cerr << "adding " << *first << "\n";
      byte_off = byte_sequence.size();
      int accept = ((first-start+1)%base_len) == 0;
      byte_sequence.emplace_back(*first,accept,0,0);

      if(test_result == pass)
        byte_sequence[last_off].pass_skip = (byte_off - last_off);
      else if(test_result == fail)
        byte_sequence[last_off].fail_skip = (byte_off - last_off);

      last_off = byte_off;
      test_result = pass;
      ++first;
    }

    else if(byte_sequence[byte_off].byte == *first) {
// std::cerr << "before: " << byte_sequence[byte_off].byte << " == " << *first
//   << "-> byte_off: " << byte_off << " last_off: " << last_off << "\n";
      ++first;
      last_off = byte_off;

      if(((first-start)%base_len) == 0)
        byte_sequence[byte_off].accept = 1;

      byte_off += byte_sequence[byte_off].pass_skip;
      test_result = pass;
// std::cerr << "after: " << "*first = " << *first
//   << "-> byte_off: " << byte_off << " last_off: " << last_off << "\n";
    }
    else {
// std::cerr << byte_sequence[byte_off].byte << " != " << *first
//   << "-> byte_off: " << byte_off << " last_off: " << last_off << "\n";
      last_off = byte_off;
      byte_off += byte_sequence[byte_off].fail_skip;
      test_result = fail;
    }
  }

// std::cerr << "HERE1\n";
  if(repeat)
    byte_sequence[last_off].pass_skip = -base_len;
// std::cerr << "HERE2\n";
}


template<typename ByteSeqDescIter>
std::vector<byte_chunk> compile_seq(ByteSeqDescIter desc_first,
  ByteSeqDescIter desc_last)
{
  if(desc_first == desc_last)
    return std::vector<byte_chunk>();

  normalize_seq(desc_first,desc_last);

  std::vector<byte_chunk> compiled_result =
    assign_bytes(desc_first->seq_bytes.begin(),desc_first->seq_bytes.end(),
      desc_first->base_seq_bytes.size(),desc_first->repeat);

  for(++desc_first; desc_first != desc_last; ++desc_first)
    compile_bytes(desc_first->seq_bytes.begin(),desc_first->seq_bytes.end(),
      desc_first->base_seq_bytes.size(),desc_first->repeat,compiled_result);

  return compiled_result;
}




template<typename T, typename ByteSequenceT>
class basic_equiv_bytesequence {
  public:
    typedef T byte_type;
    typedef ByteSequenceT bytesequence_type;

    struct byteseq_desc;

    typedef std::vector<byteseq_desc> byteseq_desc_vec_type;
    typedef std::vector<byte_chunk> byte_chunk_vec_type;

    // may throw std::length_error if size or seq_repeat[i] is zero or greater
    // than vector.max_size();
    basic_equiv_bytesequence(const byte_type *bytes[],
      const size_t bytelen[], const int seq_repeat[], size_t size,
      bool repeatflag, bool exclusiveflag);

    const byteseq_desc_vec_type & byteseq_desc_vec(void) const;

    bool repeatflag(void) const;
    void repeatflag(bool flag);

    bool exclusiveflag(void) const;
    void exclusiveflag(bool flag);

    const byte_chunk_vec_type & compiled_seq_vec(void) const;

    std::shared_ptr<bytesequence_type> effective_byteseq(void) const;
    void effective_byteseq(const std::shared_ptr<bytesequence_type> &seq);

  private:
    byteseq_desc_vec_type _byteseq_desc_vec;
    bool _repeatflag;
    bool _exclusiveflag;

    byte_chunk_vec_type _compiled_seq_vec;
    std::shared_ptr<bytesequence_type> _effective_byteseq;
};


template<typename T, typename ByteSequenceT>
struct basic_equiv_bytesequence<T,ByteSequenceT>::byteseq_desc {
  bytesequence_type seq_bytes;
  bytesequence_type base_seq_bytes;
  bool repeat;

  byteseq_desc(const bytesequence_type &seq, bool rep)
    :seq_bytes(seq), base_seq_bytes(seq), repeat(rep) {}

  template<typename ForwardIterator>
  byteseq_desc(ForwardIterator first, ForwardIterator last, bool rep)
    :seq_bytes(first,last), base_seq_bytes(first,last), repeat(rep) {}
};

template<typename T, typename ByteSequenceT>
inline basic_equiv_bytesequence<T,ByteSequenceT>::
  basic_equiv_bytesequence(const byte_type *bytes[], const size_t bytelen[],
  const int seq_repeat[], size_t size, bool repeatflag, bool exclusiveflag)
{
  if(size == 0)
    throw std::length_error("basic_equiv_bytesequence<T,ByteSequenceT>::"
      "basic_equiv_bytesequence(bytes,bytelen,seq_repeat,size,repeatflag,"
      "exclusiveflag)");

  _byteseq_desc_vec.reserve(size);

  for(std::size_t i=0; i<size; ++i) {
    if(bytelen[i] == 0)
      throw std::length_error("basic_equiv_bytesequence<T,ByteSequenceT>::"
        "basic_equiv_bytesequence(bytes,bytelen,seq_repeat,size,repeatflag,"
        "exclusiveflag)");

    _byteseq_desc_vec.emplace_back(bytes[i],bytes[i]+bytelen[i],seq_repeat[i]);
  }

  if(size == 1 && !exclusiveflag) {
    // optimize for single delimiter case
    _repeatflag = repeatflag || seq_repeat[0];
    _exclusiveflag = false;
    _effective_byteseq.reset(
      new bytesequence_type(bytes[0],bytes[0]+bytelen[0]));
  }
  else {
    _repeatflag = repeatflag;
    _exclusiveflag = exclusiveflag;

    _compiled_seq_vec = compile_seq(_byteseq_desc_vec.begin(),
      _byteseq_desc_vec.end());
  }
}

template<typename T, typename ByteSequenceT>
inline const typename basic_equiv_bytesequence<T,ByteSequenceT>::byteseq_desc_vec_type &
basic_equiv_bytesequence<T,ByteSequenceT>::byteseq_desc_vec(void) const
{
  return _byteseq_desc_vec;
}

template<typename T, typename ByteSequenceT>
inline bool basic_equiv_bytesequence<T,ByteSequenceT>::repeatflag(void) const
{
  return _repeatflag;
}

template<typename T, typename ByteSequenceT>
inline void basic_equiv_bytesequence<T,ByteSequenceT>::repeatflag(bool flag)
{
  _repeatflag = flag;
}

template<typename T, typename ByteSequenceT>
inline bool basic_equiv_bytesequence<T,ByteSequenceT>::exclusiveflag(void) const
{
  return _exclusiveflag;
}

template<typename T, typename ByteSequenceT>
inline void basic_equiv_bytesequence<T,ByteSequenceT>::exclusiveflag(bool flag)
{
  _exclusiveflag = flag;
}

template<typename T, typename ByteSequenceT>
inline const typename basic_equiv_bytesequence<T,ByteSequenceT>::byte_chunk_vec_type &
basic_equiv_bytesequence<T,ByteSequenceT>::compiled_seq_vec(void) const
{
  return _compiled_seq_vec;
}

template<typename T, typename ByteSequenceT>
inline std::shared_ptr<typename basic_equiv_bytesequence<T,ByteSequenceT>::bytesequence_type>
basic_equiv_bytesequence<T,ByteSequenceT>::effective_byteseq(void) const
{
  return _effective_byteseq;
}

template<typename T, typename ByteSequenceT>
void basic_equiv_bytesequence<T,ByteSequenceT>::effective_byteseq(
  const std::shared_ptr<typename basic_equiv_bytesequence<T,ByteSequenceT>::bytesequence_type> &seq)
{
  _effective_byteseq = seq;
}

}

#endif
