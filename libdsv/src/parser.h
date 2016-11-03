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
#include "equiv_bytesequence.h"

#include <list>
#include <vector>
#include <memory>

#include <iostream>

namespace detail {

// delete me
inline void print_packed(const std::vector<detail::byte_chunk> &packed_vec)
{
  for(std::size_t i=0; i<packed_vec.size(); ++i) {
    std::cerr << "Data @ " << i << ":\n"
      << "  Byte: " << char(packed_vec[i].byte)
      << "  Accept: " << int(packed_vec[i].accept)
      << "  Pass off: " << packed_vec[i].pass_skip
        << " (" << i+packed_vec[i].pass_skip << ")"
      << "  Fail off: " << packed_vec[i].fail_skip
        << " (";
    if(packed_vec[i].fail_skip==0)
      std::cerr << "reject";
    else
      std::cerr << i+packed_vec[i].fail_skip;
    std::cerr << ")\n\n";
  }
}


class log_description {
  private:
    typedef std::list<std::string> param_list_type;

  public:
    typedef param_list_type::const_iterator const_param_iterator;

    template<typename StringIteratorT>
    log_description(dsv_log_code c, StringIteratorT first,
      StringIteratorT last);

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
    typedef unsigned char byte_type;
    typedef std::vector<unsigned char> bytesequence_type;
    typedef basic_equiv_bytesequence<
      byte_type,bytesequence_type> equiv_bytesequence_type;
    typedef std::pair<
      equiv_bytesequence_type,bytesequence_type> replacement_pair_type;
    typedef std::vector<replacement_pair_type> replacement_pair_seq_type;

    struct escaped_field_desc {
      equiv_bytesequence_type open_equiv_bytesequence;
      equiv_bytesequence_type close_equiv_bytesequence;
      replacement_pair_seq_type escaped_field_escapes;

      std::size_t effective_escaped_field_escape_idx;
    };

    typedef std::vector<escaped_field_desc> escaped_field_desc_seq_type;

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

    /* RECORD COMPLETION DELIMITER */
    void record_delimiters(const equiv_bytesequence_type &byte_seq);
    const equiv_bytesequence_type & record_delimiters(void) const;
    void set_effective_record_delimiter(
      const std::shared_ptr<parser::bytesequence_type> &seq);

    /* FIELD COMPLETION DELIMITER */
    void field_delimiters(const equiv_bytesequence_type &byte_seq);
    const equiv_bytesequence_type & field_delimiters(void) const;
    void set_effective_field_delimiters(
      const std::shared_ptr<parser::bytesequence_type> &seq);

    /*
      FIELD ESCAPE  --- the sequence used to open and close an escaped field

      Possibilities include:
        - any to any
        - any to one
        - one to any
        - one to one

      Where 'any' means multiple equivalent sequences and 'one' means a single
      byte sequence. For each of these possibilities, multiple pairings can
      exist. For example, suppose we have an opening set A that if scanned must
      be closed by a sequence in closing set B or (A -> B). We can also have
      another pairing from opening set C that must be closed by a sequence
      contained in set D or (C -> D), etc. Each of these pairings are
      contained in the field_escapes pair vector.

      Remaining exclusivity is another supported possibility. That is, given
      the mappings above, if once a sequence from opening set A is seen, then
      a sequence from A and a sequence from the closing set B is the only valid
      opening and closing sequence for the remainder of the file.

      An even higher level of granularity is also supported.
      Suppose that the equivalent opening escaped field set A contains
      sequences {l,m,n,o}, then if sequence 'm' is seen, it is the only valid
      opening sequence for the remainder of the parse. Optionally, then suppose
      that the paired set B contains sequences {p,q,r,s}, then whichever
      closing sequence is scanned (suppose 'q'), it becomes the only valid
      closing sequence for the remainder of the file. That is to say;
      'm' opens and 'q' closes for the rest of the file. Note that open and
      close exclusivity can be independently enabled for each open and close
      equivalent sequence set. That is, if both the mappings A -> B and C -> D
      are set as exclusive but if the individual bytesequence for each of
      C and D are also set, then if a bytesequence from A is scanned first, then
      any subsequent sequence from A is the only valid open sequence and any
      sequence from B is the only valid closing sequence for the rest of
      of the file. However if a sequence from C, 'm', is scanned first, and
      a corresponding closing sequence from D, 'q', is scanned, then 'm' and 'q'
      are the only valid opening and closing sequences for the rest of the file.

      \c field_escapes gets and sets a vector of pairs of equiv_bytesequences
      where each pair corresponds to an equivalent opening and closing set.
      That is, the A -> B example above. If one individual should be exclusive
      for each set then set the \c exclusiveflag in each equiv_bytesequences.
      That is, the \c m in the \c {l,m,n,o} example above.

      \c effective_field_escapes_pair gets and sets the pair that currently
      matched. For example, if given sets A -> B, and C -> D then if a
      bytesequence from A is seen, then \c effective_field_escapes_pair
      should be set to zero indicating that the closing bytesequence should
      come from B.

      \c field_escapes_exclusives gets and set a flag that indicates whether
      or not the equiv_bytesequence_pair is considered exclusive for the
      remainder of the file. If so, the individual effective_bytesequence
      should be used to determine if it is the exclusive permanent bytesequence
      for the rest of the file.

      N.B. It is possible that an individual bytesequence for a set can be
      exclusive for the remainder of the file without the set being
      exclusive for the remainder of the file. That is:
      field_escapes_exclusives(false). In this case, if \c m is seen and set
      to exclusive but field_escapes_exclusives is set to false, then only it
      will be considered when considering A before moving on to B.
    */
    void field_escapes(const escaped_field_desc_seq_type &seq);
    const escaped_field_desc_seq_type & field_escapes(void) const;

    void set_escaped_field_escapes(std::size_t idx,
      const replacement_pair_seq_type &escaped_field_escapes);

    void set_escaped_field_escapes_repeat(std::size_t idx, std::size_t n,
      bool flag);
    void set_escaped_field_escapes_exclusives(std::size_t idx, std::size_t n,
      bool flag);



    // an idx of -1 or SIZE_MAX indicates exclusive was not set or it was set
    // but an open field escape was not seen yet
    void effective_field_escapes_pair(std::size_t idx);
    std::size_t effective_field_escapes_pair(void) const;

    // sets \c seq on the field_escapes pair indicated by
    // \c effective_field_escapes_pair
    void set_effective_open_field_escapes(
      const std::shared_ptr<parser::bytesequence_type> &seq);
    void set_effective_close_field_escapes(
      const std::shared_ptr<parser::bytesequence_type> &seq);

    void field_escapes_exclusives(bool flag);
    bool field_escapes_exclusives(void) const;
    bool escaped_field(void) const;
    bool escaped_field(bool val);




//////////////////////




    const dsv_newline_behavior & newline_behavior(void) const;
    dsv_newline_behavior newline_behavior(dsv_newline_behavior behavior);

    size_t field_columns(void) const;
    size_t field_columns(size_t num_cols);

    bool escaped_binary_fields(void) const;
    bool escaped_binary_fields(bool flag);







    size_t effective_field_columns(void) const;
    size_t effective_field_columns(size_t num_cols);

    bool effective_field_columns_set(void) const;
    bool effective_field_columns_set(bool flag);

    void reset(void);

  private:
    log_callback_t _log_callback;
    void *_log_context;
    dsv_log_level _log_level;

    log_list_type log_list;


    equiv_bytesequence_type _record_delimiters;
    equiv_bytesequence_type _field_delimiters;

    escaped_field_desc_seq_type _field_escapes;
    std::size_t _effective_field_escapes_idx;
    bool _field_escapes_exclusives;
    bool _escaped_field;



    size_t _field_columns;
    bool _escaped_binary_fields;

    size_t _effective_field_columns;
    bool _effective_field_columns_set;

    static const equiv_bytesequence_type & make_default_record_delimiters(void);
    static const equiv_bytesequence_type & make_default_field_delimiters(void);
    static const escaped_field_desc_seq_type & make_default_field_escapes(void);
};

inline parser::parser(void) :_log_callback(0), _log_context(0),
  _log_level(dsv_log_none),
  _record_delimiters(make_default_record_delimiters()),
  _field_delimiters(make_default_field_delimiters()),
  _field_escapes(make_default_field_escapes()),
  _effective_field_escapes_idx(-1),
  _field_escapes_exclusives(1),
  _field_columns(0), _escaped_binary_fields(false),
  _effective_field_columns(0),
  _effective_field_columns_set(false)
{
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




inline void
parser::record_delimiters(const equiv_bytesequence_type &byte_seq)
{
  _record_delimiters = byte_seq;
}

inline const parser::equiv_bytesequence_type &
parser::record_delimiters(void) const
{
  return _record_delimiters;
}

inline void parser::set_effective_record_delimiter(
  const std::shared_ptr<parser::bytesequence_type> &seq)
{
  _record_delimiters.effective_byteseq(seq);
}


inline void
parser::field_delimiters(const equiv_bytesequence_type &byte_seq)
{
  _field_delimiters = byte_seq;
}

inline const parser::equiv_bytesequence_type &
parser::field_delimiters(void) const
{
  return _field_delimiters;
}

inline void parser::set_effective_field_delimiters(
  const std::shared_ptr<parser::bytesequence_type> &seq)
{
  _field_delimiters.effective_byteseq(seq);
}






inline void
parser::field_escapes(const escaped_field_desc_seq_type &seq)
{
  _field_escapes = seq;
}

inline const parser::escaped_field_desc_seq_type & parser::field_escapes(void) const
{
  return _field_escapes;
}

inline void parser::set_escaped_field_escapes(std::size_t idx,
    const replacement_pair_seq_type &escaped_field_escapes)
{
  escaped_field_desc &desc = _field_escapes.at(idx);
  desc.escaped_field_escapes = escaped_field_escapes;
}

inline void
parser::set_escaped_field_escapes_repeat(std::size_t idx, std::size_t n,
  bool flag)
{
  escaped_field_desc &desc = _field_escapes.at(idx);
  desc.escaped_field_escapes.at(n).first.repeatflag(flag);
}

inline void
parser::set_escaped_field_escapes_exclusives(std::size_t idx,
  std::size_t n, bool flag)
{
  escaped_field_desc &desc = _field_escapes.at(idx);
  desc.escaped_field_escapes.at(n).first.exclusiveflag(flag);
}



inline void parser::effective_field_escapes_pair(std::size_t idx)
{
  _effective_field_escapes_idx = idx;
}

inline std::size_t parser::effective_field_escapes_pair(void) const
{
  return _effective_field_escapes_idx;
}

inline void parser::field_escapes_exclusives(bool flag)
{
  _field_escapes_exclusives = flag;
}

inline bool parser::field_escapes_exclusives(void) const
{
  return _field_escapes_exclusives;
}


















inline size_t parser::field_columns(void) const
{
  return _field_columns;
}

inline size_t parser::field_columns(size_t cols)
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

// nonexposed
















inline bool parser::escaped_field(void) const
{
  return _escaped_field;
}

inline bool parser::escaped_field(bool val)
{
  std::swap(val,_escaped_field);
  return val;
}

inline size_t parser::effective_field_columns(void) const
{
  return _effective_field_columns;
}

inline size_t parser::effective_field_columns(size_t cols)
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

// todo fix me
inline void parser::reset(void)
{
  log_list.clear();
  _escaped_field = false;
  _effective_field_columns = _field_columns;
  _effective_field_columns_set = (_field_columns > 0);
}

inline const parser::equiv_bytesequence_type &
parser::make_default_record_delimiters(void)
{
  // default is CRLF
  static const byte_type default_seq[] = {0x0D,0x0A};
  static const byte_type *default_equiv_seqs[1] = {default_seq};
  static const size_t default_equiv_seqs_size[1] = {2};
  static const int default_equiv_seqs_repeat[1] = {0};

  static const equiv_bytesequence_type equiv_sequence(default_equiv_seqs,
    default_equiv_seqs_size,default_equiv_seqs_repeat,1,0,1);

  return equiv_sequence;
}

inline const parser::equiv_bytesequence_type &
parser::make_default_field_delimiters(void)
{
  // default is comma
  static const byte_type default_seq[] = {','};
  static const byte_type *default_equiv_seqs[1] = {default_seq};
  static const size_t default_equiv_seqs_size[1] = {1};
  static const int default_equiv_seqs_repeat[1] = {0};

  static const equiv_bytesequence_type equiv_sequence(default_equiv_seqs,
    default_equiv_seqs_size,default_equiv_seqs_repeat,1,0,1);

  return equiv_sequence;
}

inline const parser::escaped_field_desc_seq_type &
parser::make_default_field_escapes(void)
{
  // default is double quote ie "
  static const byte_type default_seq[] = {'"'};
  static const byte_type *default_equiv_seqs[1] = {default_seq};
  static const size_t default_equiv_seqs_size[1] = {1};
  static const int default_equiv_seqs_repeat[1] = {0};

  static const escaped_field_desc default_escape_field_desc = {
      equiv_bytesequence_type(default_equiv_seqs,
        default_equiv_seqs_size,default_equiv_seqs_repeat,1,0,1),
      equiv_bytesequence_type(default_equiv_seqs,
        default_equiv_seqs_size,default_equiv_seqs_repeat,1,0,1)
    };

  static const escaped_field_desc_seq_type default_escape_field_desc_seq =
    {default_escape_field_desc};

  return default_escape_field_desc_seq;
}


}

#endif
