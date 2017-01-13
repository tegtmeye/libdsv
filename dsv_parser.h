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

#ifndef LIBDSV_DSV_PARSER_H
#define LIBDSV_DSV_PARSER_H

#include <unistd.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

  /**
   * \file dsv_parser.h
   * \brief The main header file for the Delimited Seperated Values library
   *
   *
   */

  /**
   *  \brief An opaque handle for a dsv parser object
   */
  typedef struct {
    void *p;
  } dsv_parser_t;

  /**
      \brief Initialize a default dsv_parser_t object.

      This parser is set up with the following default settings:
        //todo

      \note You must eventually call dsv_parser_destroy.

      \param[in,out] parser A pointer to a dsv_parser_t object to initialize
        with default settings
      \retval 0 success
      \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_create(dsv_parser_t *parser);

  /**
   *  \brief Destroy the obj_parser_t object.
   *
   *  \post using \c parser with any function other than \c dsv_parser_create
   *    is undefined
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with \c dsv_parser_create*
   */
  void dsv_parser_destroy(dsv_parser_t parser);



  // LOW LEVEL INTERFACE

  // todo add supported expression syntax


  /**
    \brief Set a regular expression that represents a record delimiter.
    If the the exclusive flag is set, only the exact sequence first
    matched by the delimiter expression will be recognized in future
    parser.

    See the section on regular expressions regarding allowed
    expressions.

    The sequence \c utf8_regex is interpreted as an UTF8 string and is
    compatible with an ASCII string.

    \c regex_size may be zero. If this is the case, then there will be exactly
    one record in the parsed output.

    Parse exclusivity means that the first matched byte sequence parsed
    as a delimiter becomes the only acceptable delimiter for the
    remainder of the parsing.

    This delimiter is used to separate both headers and records
    depending on the settings

    \param[in] parser A pointer to a dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

    \param[in] utf8_regex A character sequence of length \c regex_size
      representing a well-formed UTF8-encoded regular expression

    \param[in] utf8_regex The size of the character sequence \c utf8_regex. May
      be zero to indicate no record delimiter.

    \param[in] exclusiveflag If nonzero, then when the first bytesequence
      matched by \c utf8_regex is encountered, it becomes the only valid
      bytesequence for the remainder of the parser operation.

    \retval 0 success

    \retval ENOMEM Could not allocate memory

    \retval EINVAL \c utf8_regex represents an ill-formed UTF8-encoded regular
      expression
   */
  int dsv_parser_set_record_delimiters(dsv_parser_t parser,
    const char *utf8_regex, size_t regex_size, int exclusiveflag);

  /**
    \brief Obtain whether or not the first parsed record delimiter assigned to
    \parser is the only permitted subsequent delimiter for the remainder of
    parsing

    The returned values is the same as the \c exclusiveflag parameter in
    \c dsv_parser_set_record_delimiters

    \param[in] parser A pointer to a dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions
    \retval If nonzero, the first matched record bytesequence shall be the only
      permitted record delimiter for the remainder of the parsing with
      \c parser.
   */
  int dsv_parser_get_record_delimiter_exclusiveflag(dsv_parser_t parser);

  /**
    \brief Copy the \c n th record delimiter to be used for future parsing
    with \c parser into the buffer \c buff of size \c buffsize and set the
    location pointed to by \c repeatflag as to if the delimiter was allowed to
    be repeated indefinitely.

    This delimiter is used to separate both headers and records depending on
    the settings.

    If \c buffsize is zero, return the number of bytes needed to hold the
    current set delimiter. This number is suitable for allocating memory for
    \c buf. If \c buffsize is nonzero, copy the bytes representing the
    delimiter or \c buffsize whichever is smaller and return this value. If \c
    buffsize is zero, \c buff is not referenced and may be zero for the call.

    If \c n is a valid value, and \c repeatflag is nonzero, it will be set to
    the repeat value of the \c n th delimiter regardless of the value of \c
    buff and \c buffsize

    \param[in] parser A pointer to a dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions
    \param[in] n The bytesequence number to return.
    \param[in,out] buff If \c buffsize is nonzero, an unsigned char buffer of
      size \buffsize to which the current bytesequence will be copied into.
      N.B. since the sequence of bytes contained in \c buff may not represent a
      string, no null terminator will be added to the end of the bytes.
    \param [in] buffsize The size of the unsigned char buffer pointed to
      by \c buf.
    \param [in,out] repeatflag If \c repeatflag is nonzero, set the location
      pointed to by \c repeatflag to a value if nonzero indicates that the
      \c nth bytesequence can be repeated indefinitely.
    \retval If \c buffsize is zero, return the number of bytes needed to hold
      the current bytesequence. If \c buffsize is nonzero, return the number of
      bytes copied to \c buff which is not necessarily the same size as \c
      buffsize.
    \retval 0 \c n is greater than the number of equivalent bytesequences
      currently set for \c parser
   */
  size_t dsv_parser_get_record_delimiters(dsv_parser_t parser, char *buff,
    size_t buffsize);

  /**
    \brief Set a regular expression that represents a field delimiter.
    If the the exclusive flag is set, only the exact sequence first
    matched by the delimiter expression will be recognized in future
    parser.

    See the section on regular expressions regarding allowed
    expressions.

    The sequence \c utf8_regex is interpreted as an UTF8 string and is
    compatible with an ASCII string.

    Parse exclusivity means that the first matched byte sequence parsed
    as a delimiter becomes the only acceptable delimiter for the
    remainder of the parsing.

    This delimiter is used to separate both headers and records fields
    depending on the settings

    \note There must always be either a record delimiter or a field
    delimiter. That is, setting \c size to zero is invalid if the record
    delimiter size is also zero.

    \param[in] parser A pointer to a dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions
    \param[in] utf8_regex A character sequence of length \c regex_size
      representing a well-formed UTF8-encoded regular expression
    \param[in] utf8_regex The size of the character sequence \c utf8_regex
    \param[in] exclusiveflag If nonzero, then when the first bytesequence
      matched by \c utf8_regex is encountered, it becomes the only valid
      bytesequence for the remainder of the parser operation.
    \retval 0 success
    \retval ENOMEM Could not allocate memory
    \retval EINVAL Either \c regex_size is zero or it represents an ill-formed
      UTF8-encoded regular expression
   */
  int dsv_parser_set_field_delimiters(dsv_parser_t parser,
    const char *utf8_regex, size_t regex_size, int exclusiveflag);


  /**
   *  \brief Obtain the number of field delimiters currently assigned to
   *  \c parser
   *
   *  The returned values is the same as the \c size parameter in
   *  \c dsv_parser_set_equiv_field_delimiters
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \retval The number of field delimiters set for \c parser
   */
  size_t dsv_parser_num_equiv_field_delimiters(dsv_parser_t parser);

  /**
   *  \brief Obtain whether or not the field delimiters assigned to \parser are
   *  allowed to repeat indefinitely
   *
   *  The returned values is the same as the \c repeatflag parameter in
   *  \c dsv_parser_set_equiv_field_delimiters
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \retval If nonzero, the currently assigned field delimiters to \c parser
   *    are allowed to repeat indefinitely
   */
  int dsv_parser_get_equiv_field_delimiters_repeatflag(dsv_parser_t parser);

  /**
   *  \brief Obtain whether or not the first parsed field delimiter assigned to
   *  \parser is the only permitted subsequent delimiter for the remainder of
   *  parsing
   *
   *  The returned values is the same as the \c exclusiveflag parameter in
   *  \c dsv_parser_set_equiv_field_delimiters
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \retval If nonzero, the first delimiter parsed among the currently
   *    assigned field delimiters in\c parser shall be the only permitted
   *    delimiter for the remainder of the parsing session.
   */
  int dsv_parser_get_equiv_field_delimiters_exclusiveflag(dsv_parser_t parser);

  /**
   *  \brief Copy the \c n th field delimiter to be used for future parsing with
   *  \c parser into the buffer \c buff of size \c buffsize and set the location
   *  pointed to by \c repeatflag as to if the delimiter was allowed to be
   *  repeated indefinitely.
   *
   *  This delimiter is used to separate both headers and fields depending on
   *  the settings.
   *
   *  If \c buffsize is zero, return the number of bytes needed to hold the
   *  current set delimiter. This number is suitable for allocating memory for
   *  \c buf. If \c buffsize is nonzero, copy the bytes representing the
   *  delimiter or \c buffsize whichever is smaller and return this value. If \c
   *  buffsize is zero, \c buff is not referenced and may be zero for the call.
   *
   *  If \c n is a valid value, and \c repeatflag is nonzero, it will be set to
   *  the repeat value of the \c n th delimiter regardless of the value of \c
   *  buff and \c buffsize
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in,out] buff If \c buffsize is nonzero, an unsigned char buffer of
   *    size \buffsize to which the current field delimiter will be copied into.
   *    N.B. since the sequence of bytes contained in \c buff may not represent a
   *    string, no null terminator will be added to the end of the bytes.
   *  \param [in] buffsize The size of the unsigned char buffer pointed to
   *    by \c buf.
   *  \param [in,out] repeatflag If \c repeatflag is nonzero, set the location
   *    pointed to by \c repeatflag to a value if nonzero indicates that the
   *    \c th delimiter can be repeated indefinitely.
   *  \retval If \c buffsize is zero, return the number of bytes needed to hold
   *    the current delimiter. If \c buffsize is nonzero, return the number of
   *    bytes copied to \c buff which is not necessarily the same size as \c
   *    buffsize.
   * \retval 0 \c n is greater than the number of field delimiters currently set
   *    for \c parser
   */
  size_t dsv_parser_get_equiv_field_delimiter(dsv_parser_t parser, size_t n,
    unsigned char *buff, size_t buffsize, int *repeatflag);





  /**
      Field Escapes:

      Field escapes are the opening and closing "tags" used to separate the
      contents of fields from contents that may otherwise delimit or indicate
      structure. For example, given an RFC-4180 parser, the double quotes,
      or \c " serve as both opening and closing field delimiters which allow,
      for example, a comma to be part of the field and not be considered a
      field delimiter.

      The library supports a wide variety of simple to very complex
      field escapes scenarios. At the core, there are a pair of
      opening and closing bytesequences matched by independent regular
      expressions that enclose a field. In the above example, the double
      quote '"' is both the opening and closing. Another example could
      be an opening '<' and a closing '>'. Because each opening and
      closing field escape has an independant regular expression,
      equivalent opening and closing values are possible. For example,
      an opening '<' OR '[' and a closing '>' OR ']' indicated by "<|["
      and ">|]" respectively not including the quotes.

      Suppose however that if an opening '<' is seen, the parser should only
      accept a closing '>' but if an opening '[' is seen, it should only accept
      a closing ']'. This is where multiple field escape pairs come in. In this
      example, there are two pairs of expressions, the first has the
      opening '<' and closing '>' and the second has the opening '[' and the
      closing ']'. The only bytesequence accepted for the closing field escape
      will be the pair associated with the escape that opened the field.

      Given that each opening and closing pair are equivalent bytesequences,
      multipbyte sequences are also supported along with parse exclusivity.

      Possibilities include:
        - any to any
        - any to one
        - one to any
        - one to one

      Where 'any' means multiple equivalent sequences and 'one' means a
      single byte sequence. For each of these possibilities, multiple
      pairings can exist. For example, suppose we have an opening
      sequence set A that if scanned must be closed by a sequence in
      closing set B or (A -> B). We can also have another pairing from
      opening sequence set C that must be closed by a sequence contained
      in set D or (C -> D), etc. Each of these pairings are contained in
      the field escapes pair list.

      Pair exclusivity is another supported possibility. That is, given
      the mappings above, if once a sequence from opening set A is seen, then
      a sequence from A and a sequence from the closing set B is the only valid
      opening and closing sequence for the remainder of the file.

      An even higher level of granularity is also supported. Suppose
      that the equivalent opening escaped field set A contains the
      expression "l|m|n|o", then if sequence 'm' is seen, it is the only
      valid opening sequence for the remainder of the parse. Optionally,
      then suppose that the paired set B contains sequences "p|q|r|s",
      then whichever closing sequence is scanned (suppose 'q'), it
      becomes the only valid closing sequence for the remainder of the
      file. That is to say; 'm' opens and 'q' closes for the rest of the
      file. Note that open and close exclusivity can be independently
      enabled for each open and close equivalent sequence set. That is,
      if both the mappings A -> B and C -> D are set as exclusive but if
      the individual bytesequence for each of C and D are also set, then
      if a bytesequence from A is scanned first, then any subsequent
      sequence from A is the only valid open sequence and any sequence
      from B is the only valid closing sequence for the rest of of the
      file. However if a sequence from C, 'm', is scanned first, and a
      corresponding closing sequence from D, 'q', is scanned, then 'm'
      and 'q' are the only valid opening and closing sequences for the
      rest of the file.

      \c field escapes manipulation functions get and appends pairs of
      expressions where each pair corresponds to an opening and closing
      set. That is, the A -> B example above. If one individual should
      be exclusive for each set then set the the associated \c
      exclusiveflag.

      \c field_escape_exclusiveflag gets and set a flag that indicates
      whether or not the first opening expression matched causes the
      pair to be considered exclusive for the remainder of the file.

      N.B. It is possible that an individual expression for a set can be
      exclusive for the remainder of the file without the set being
      exclusive for the remainder of the file. That is:
      field_escapes_exclusives == false. In this case, if \c m is seen and set
      to exclusive but field_escapes_exclusives is set to false, then only it
      will be considered when considering A before moving on to B.
   */

  /**
      \brief Assign the matching opening and closing field escape expressions and the associated exclusivity for future parsing with \c parser

      See Field Delimiters for a complete description of what a field delimiter
      is and how it is used.

      An 8 parameter function call is less that ideal but the
      alternative would be either error-prone or involve many function
      calls that present a verbose and clunky interface or worse, leave
      the parser in an invalid state if used incorrectly.

      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \param[in] open_utf8_regex \parablock
        An array of length \c pair_size such that the ith element is a pointer
        to a character sequence of length \c open_regex_size[i]
        representing a well-formed UTF8-encoded regular expression used to match
        an open field escape.
      \endparblock

      \param[in] open_regex_size \parablock
        An array of length \c pair_size such that the ith element is the size of
        the ith sequence of bytes pointed to by \c open_utf8_regex[i].
      \endparblock

      \param[in] open_exclusiveflag \parblock
        An array of length \c pair_size such that if the ith element is
        nonzero, the bytesequence matched by open_utf8_regex[i] becomes
        the only matched open field escape sequence for the remainder of
        the parser operation.
      \endparblock

      \param[in] close_utf8_regex \parablock
        An array of length \c pair_size such that the ith element is a pointer
        to a character sequence of length \c close_regex_size[i]
        representing a well-formed UTF8-encoded regular expression used to match
        an close field escape.
      \endparblock

      \param[in] close_regex_size \parablock
        An array of length \c pair_size such that the ith element is the size of
        the ith sequence of bytes pointed to by \c close_utf8_regex[i].
      \endparblock

      \param[in] close_exclusiveflag \parblock
        An array of length \c pair_size such that if the ith element is
        nonzero, the bytesequence matched by close_utf8_regex[i] becomes
        the only matched closing field escape sequence for the remainder of
        the parser operation.
      \endparblock

      \param[in] pair_size The length of each of \c open_utf8_regex,
        \c open_regex_size, \c open_exclusiveflag, \c close_utf8_regex,
        \c close_regex_size, \c close_exclusiveflag. If \c pair_size is zero,
        then no field escapes will be matched during parser.

      \param[in] pair_exclusiveflag If nonzero, the first bytesequence pair
        matched according becomes the only pair to be considered for future
        parsing with \c parser

      \retval 0 success
      \retval ENOMEM Could not allocate memory
      \retval EINVAL The value of \c open_utf8_regex, \c open_regex_size,
        \c open_exclusiveflag, \c close_utf8_regex, \c close_regex_size, or
        \c close_exclusiveflag
      \retval EINVAL An element of \c open_utf8_regex, \c open_regex_size,
        \c close_utf8_regex, or \c close_regex_size is zero
      \retval EINVAL A regular expression pointed to by an element of
        \c open_utf8_regex or \c close_utf8_regex is invalid.
   */
  int dsv_parser_set_field_escape_pair(dsv_parser_t parser,
    const char *open_utf8_regex[], const size_t open_regex_size[],
    const int open_exclusiveflag[],
    const char *close_utf8_regex[], const size_t close_regex_size[],
    const int close_exclusiveflag[],
    size_t pair_size, int pair_exclusiveflag);


  /**
      \brief Get the number of escape sequence pairs current assigned to
      \c parser

      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \retval non-negative Number of escape sequence pairs assigned to \c parser
  */
  size_t dsv_parser_num_field_escape_pairs(dsv_parser_t parser);

  /**
      \brief Return whether or not the \c ith open escape sequence associated
      with \c parser has the exclusive flag enabled.

      This value is the same as the \c open_exclusiveflag parameter provided in
      \c dsv_parser_set_field_escape_pair for the \c ith pair

      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \param[in] i A value smaller than the return value of
      \c dsv_parser_num_field_escape_pairs

      \retval negative means i does not point to a valid pair
      \retval 0 The open_exclusiveflag is NOT set
      \retval positive The open_exclusiveflag IS set
  */
  int dsv_parser_get_field_escape_pair_open_exclusiveflag(dsv_parser_t parser,
    size_t i);

  /**
      \brief Return whether or not the \c ith close escape sequence associated
      with \c parser has the exclusive flag enabled.

      This value is the same as the \c close_exclusiveflag parameter provided in
      \c dsv_parser_set_field_escape_pair for the \c ith pair

      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \param[in] i A value smaller than the return value of
      \c dsv_parser_num_field_escape_pairs

      \retval negative means i does not point to a valid pair
      \retval 0 The close_exclusiveflag is NOT set
      \retval positive The close_exclusiveflag IS set
  */
  int dsv_parser_get_field_escape_pair_close_exclusiveflag(
    dsv_parser_t parser, size_t i);

  /**
      \brief Get the open expression associated with the ith field escape
      pair in with \c parser.


      \param[in] parser A dsv_parser_t object previously
        initialized with one of the \c dsv_parser_create* functions

      \param[in] pairi A nonzero value smaller than the return value of
        \c dsv_parser_num_field_escape_pairs

      \param[in,out] buff If \c buffsize is nonzero, a pointer to \c
        buffsize bytes to be overwritten by the first \c buffsize bytes of
        \c pairi field escape pair open expression. NB The expression is
        assumed to be UTF8 encoded. The buffer is filled with bytes, not
        characters. Although if the expression contains only ASCII
        characters these are the same, there are no checks to ensure if a
        \c buffsize smaller then the number of bytes necessary to hold the
        complete expresion does not split a multibyte character if UTF8
        encoded.

      \param[in] buffsize If nonzero, the size of the buffer pointed to
        by \c buff. If zero, return the number of bytes needed to hold the
        complete expression associated with the \c pairi open pair.

      \retval 0 If \c pairi is not smaller than the value returned by \c
        dsv_parser_num_field_escape_pairs

      \retval nonnegative If \c buffsize is zero, then return the the number of
        bytes needed to store the expression associated with the
        \c pairi-th pair. If \c buffsize is nonzero, copy the first
        \c buffsize bytes of the expression associated with the \c pairi-th
        open pair.
  */
  size_t dsv_parser_get_field_escape_pair_open_expression(dsv_parser_t parser,
    size_t pairi, char *buff, size_t buffsize);

  /**
      \brief Get the close expression associated with the ith field escape
      pair in with \c parser.


      \param[in] parser A dsv_parser_t object previously
        initialized with one of the \c dsv_parser_create* functions

      \param[in] pairi A nonzero value smaller than the return value of
        \c dsv_parser_num_field_escape_pairs

      \param[in,out] buff If \c buffsize is nonzero, a pointer to \c
        buffsize bytes to be overwritten by the first \c buffsize bytes of
        \c pairi field escape pair close expression. NB The expression is
        assumed to be UTF8 encoded. The buffer is filled with bytes, not
        characters. Although if the expression contains only ASCII
        characters these are the same, there are no checks to ensure if a
        \c buffsize smaller then the number of bytes necessary to hold the
        complete expresion does not split a multibyte character if UTF8
        encoded.

      \param[in] buffsize If nonzero, the size of the buffer pointed to
        by \c buff. If zero, return the number of bytes needed to hold the
        complete expression associated with the \c pairi close pair.

      \retval 0 If \c pairi is not smaller than the value returned by \c
        dsv_parser_num_field_escape_pairs

      \retval nonnegative If \c buffsize is zero, then return the the number of
        bytes needed to store the expression associated with the
        \c pairi-th pair. If \c buffsize is nonzero, copy the first
        \c buffsize bytes of the expression associated with the \c pairi-th
        close pair.
  */
  size_t dsv_parser_get_field_escape_pair_close_expression(dsv_parser_t parser,
    size_t pairi, char *buff, size_t buffsize);


  /**
      \brief Return whether or not the parser will only accept future occurances
      of the first open and close escape pair seen with \c parser.

      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \retval nonnegative If the first occurrence of a particular field escape
      pair will be the only valid pair for the remainder of the parsing.
  */
  int dsv_parser_get_field_escape_exclusiveflag(dsv_parser_t parser);

  /**
      \brief Set whether or not the parser will only accept future occurances
      of the first open and close escape pair seen with \c parser.

      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \param[in] flag Nonnegative indicates the first occurrence of a particular
      field escape pair will be the only valid pair for the remainder of the
      parsing.
  */
  void dsv_parser_set_field_escape_exclusiveflag(dsv_parser_t parser, int flag);


  /**
    \brief Set the escaped field escapes with replacement to be used
    with the chosen field escape pair for future parsing with \c parser.

    An escaped field escape is a bytesequence used inside of an escaped
    field to indicate that the sequence is part of the field
    bytesequence rather than the field closing escape sequence.

    For example, in an RFC 4180-strict parser, a single non-repeating
    ASCII double quote \c " is the only valid field escape sequence and
    a twice repeating double quote \c "" is the only valid escape field
    escape sequence. For example the field \c "foo bar" contains seven
    characters, \c 'foo' and 'bar' separated by a space character. If
    one wanted the field to actually contain the quotes, the field would
    contain """foo bar""", which has nine characters: the first \c "
    opens the field (not part of the nine), the two repeating \c "" is a
    escaped field escape sequence which is replaced in the actual field
    by a single \c ", the words \c 'foo' and \c 'bar' separated by a
    space, two more repeating \c "" which is replaced in the actual
    field by a single \c ", and the final \c " to close the field (also
    not part of the nine).

    Like other places in this library, it is possible to specify
    multiple, equivalent bytesequences to mean the same thing. For
    example, suppose some fictional format allows the use of either two
    double quotes \c "" OR the hex representation of the single double
    quote---ie \c 0x22. To which a valid version of the previous example
    would be \c """foo bar0x22". Specifying each of these as valid
    sequences is possible.

    For each expression representing valid escaped field escapes there
    is a replacement bytesequence that will be substituted for the
    parsed escaped field escape. For example, in an RFC 4180-strict
    parser a twice repeating double quote \c "" is replaced by the
    single double quote in the field. That is, if the raw field contains
    \c "foo""bar", the parsed field field contains \c 'foo"bar' without
    the single tics. Turning again to the previous fictional example, if
    a valid escaped field escape is two double quotes OR the string
    '0x22' which corresponds to ASCII representation of the double quote
    both would map a replacement to a single double quote. That is, for
    the raw field \c """foo bar0x22" the parsed value would be \c '"foo
    bar"' without the single tics.

    By allowing multiple equivalent bytesequence -> replacement
    mappings, this matches the flexibility in the field escape pairs.
    For example it is possible to have a field opened and closed by a
    single tic; \c ' OR a double quote; \c " and have each be twice
    repeating to be an escaped field escape. That is, support the raw
    sequence \c "foo""bar" and \c 'foo''bar' as parsed fields \c foo"bar
    and \c foo'bar respectively in the same file. This is done by
    assigning a escaped field escape to a particular open and closing
    field escape. For the previous example, there is a open and closing
    field escape pair for a double quote ("foo") with a escaped field
    escape of two double quotes ("") ie "foo""bar" is parsed as the 7
    characters foo"bar and another open and closing field escape pair
    for the single quote ('foo') with and escaped field escape of two
    single quotes ('') ie 'foo''bar' is parsed as the 7 characters
    foo'bar.

    N.B. It is not required that a field escape pair has an escaped
    field escape. It is also legal to map an escape field escape to
    multiple field escape pairs. For example, two double quotes \c "" ->
    \c " can be mapped to every set field escape pair. Additionally, it
    is not required that a escape field has a replacement. In this case,
    the matched escape expression will be removed from the field value.

    \param[in] parser A pointer to a dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

    \para,[in] pairi The field escape pair to associate this
      escaped field escape with. If the return value of
      dsv_parser_num_field_escape_pairs is less than \c field_escape_pair,
      the function returns EINVAL and no changes are made.

    \param[in] utf8_regex \parablock
      An array of length \c nescapes such that the ith element is a pointer
      to a character sequence of length \c regex_size[i]
      representing a well-formed UTF8-encoded regular expression used to match
      an escape field escape.
    \endparblock

    \param[in] regex_size \parablock
      An array of length \c nescapes such that the ith element is the size of
      the ith sequence of bytes pointed to by \c utf8_regex[i].
    \endparblock

    \param[in] replacement \parablock
      An array of length \c nescapes such that the ith element is a
      pointer to a character sequence of length \c replacement_size[i]
      that will be wholly substituted in place of the matched expression
      contained in utf8_regex[i]
    \endparblock

    \param[in] replacement_size \parablock
      An array of length \c nescapes such that the ith element is the size of
      the ith sequence of bytes pointed to by \c replacement[i].
    \endparblock

    \param[in] nescapes The size of each array \c utf8_regex, \c
      regex_size, \c replacement, and \c replacement_size. If \c nescapes
      is zero, all existing escape field escapes will be removed for the
      indicated escape field pair.


    \retval 0 success

    \retval negative If \c pairi is not smaller than the value returned by \c
      dsv_parser_num_field_escape_pairs

    \retval ENOMEM Could not allocate memory

    \retval EINVAL nescapes is nonzero and any of \c utf8_regex, \c
      regex_size, \c replacement, and \c replacement_size is zero

    \retval EINVAL An element of \c utf8_regex or \c regex_size is zero

    \retval EINVAL An element of \c replacement is zero and the corresponding
      element of replacement_size is nonzero

    \retval EINVAL A regular expression pointed to by an element of
      \c open_utf8_regex or \c close_utf8_regex is invalid.
  */
  int dsv_parser_set_escape_field_escapes(dsv_parser_t parser,
    size_t pairi, const char *utf8_regex[], const size_t regex_size[],
    const char *replacement[], size_t replacement_size[], size_t nescapes);

  /**
    \brief Obtain the number of escaped field escapes and replacements
    associated with the indicated field escape pair

    \param[in] parser A pointer to a dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

    \param[in] pairi The field escape pair to query. If the return
      value of dsv_parser_num_field_escape_pairs is less than
      \c field_escape_pair, the function returns SIZE_MAX

    \retval number of escaped field escapes and replacements
      associated with the \c pairi-th field escape pair

    \retval SIZE_MAX \c field_escape_pair does not index a valid
      field escape pair
  */
  size_t dsv_parser_num_escape_field_escapes(dsv_parser_t parser,
    size_t pairi);

  /**
      \brief Obtain the \c nth escaped field escape expression
      associated with the indicated field escape pair and escape index


      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \param[in] pairi A nonzero value smaller than the return value of
      \c dsv_parser_num_field_escape_pairs

      \param[in] idx A nonzero value smaller than the return value of
      \c dsv_parser_num_escape_field_escapes

      \param[in,out] buff If \c buffsize is nonzero, a pointer to \c
        buffsize bytes to be overwritten by the first \c buffsize bytes of
        \c pairi escaped field escape expression. NB The expression is
        assumed to be UTF8 encoded. The buffer is filled with bytes, not
        characters. Although if the expression contains only ASCII
        characters these are the same, there are no checks to ensure if a
        \c buffsize smaller then the number of bytes necessary to hold the
        complete expresion does not split a multibyte character if UTF8
        encoded.

      \param[in] buffsize If nonzero, the size of the buffer pointed to by
        \c buff. If zero, return the size of the \c idx-th expression
        associated with the \c pairi escaped field.

      \retval SIZE_MAX If \c pairi is not smaller than the value
        returned by \c dsv_parser_num_field_escape_pairs OR \c idx is
        not smaller than the value returned by
        \c dsv_parser_num_escape_field_escapes

      \retval SIZE_MAX If buffsize is nonzero and buff is zero

      \retval nonnegative If buffsize is zero, then the number of
        bytes needed to store the \c idx-th expression associated with
        the \c pairi-th pair. If nonzero, copy the first \buffsize bytes
        of the \c idx-th expression associated with the \c pairi-th pair
        and return the lesser of buffsize and the maximum number of
        bytes needed to hold the expression.
  */
  size_t dsv_parser_get_escaped_field_escape_expression(dsv_parser_t parser,
    size_t pairi, size_t idx,  unsigned char *buff, size_t buffsize);

  /**
      \brief Obtain the \c nth escaped field escape replacement
      associated with the indicated field escape pair and escape index


      \param[in] parser A dsv_parser_t object previously
      initialized with one of the \c dsv_parser_create* functions

      \param[in] pairi A nonzero value smaller than the return value of
      \c dsv_parser_num_field_escape_pairs

      \param[in] idx A nonzero value smaller than the return value of
      \c dsv_parser_num_escape_field_escapes

      \param[in,out] buff If \c buffsize is nonzero, a pointer to \c
        buffsize bytes to be overwritten by the first \c buffsize bytes
        of \c pairi escaped field escape replacement. NB the replacement
        value may be empty

      \param[in] buffsize If nonzero, the size of the buffer pointed to by
        \c buff. If zero, return the size of the \c idx-th replacement
        associated with the \c pairi escaped field.

      \retval SIZE_MAX If \c pairi is not smaller than the value
        returned by \c dsv_parser_num_field_escape_pairs OR \c idx is
        not smaller than the value returned by
        \c dsv_parser_num_escape_field_escapes

      \retval SIZE_MAX If buffsize is nonzero and buff is zero

      \retval nonnegative If buffsize is zero, then the number of
        bytes needed to store the \c idx-th replacement associated with
        the \c pairi-th pair. If nonzero, copy the first \buffsize bytes
        of the \c idx-th replacement associated with the \c pairi-th pair
        and return the lesser of buffsize and the maximum number of
        bytes needed to hold the replacement.
  */
  size_t dsv_parser_get_escaped_field_escape_replacement(dsv_parser_t parser,
    size_t pairi, size_t idx,  unsigned char *buff, size_t buffsize);

  /**
     \brief Set the required number of fields for future parsing with \c parser
     or allow a non-uniform number.

     If the behavior specified by \c dsv_parser_set_field_columns is
     violated, dsv_parse will immediately return a nonzero value and an error
     message will be logged with the code: \c dsv_column_count_error.

     The default value is 0. This value is also appropriate for RFC4180-strict
     processing

     \param[in] parser A pointer to a dsv_parser_t object previously
       initialized with one of the \c dsv_parser_create* functions
     \param[in] num_cols \parblock
       If > 0, the number of columns expected during future
       parsing. If during parsing, a row with less than \c num_cols is
       encountered, dsv_parse will immediately return with a nonzero value. If
       \c num_cols == 0, the parser will set the required number of columns
        based on the first row encountered. For example, if the first header
       row contains 5 columns, all subsequent rows must contain 5 columns
       otherwise the dsv_parse will immediately return a nonzero value. If
       \c num_cols == (size_t)-1, no restriction will be placed on the number of
       columns. This also means that rows with zero columns are acceptable. In
       this case, any registered callback will still be called.
     \endparblock
   */
  void dsv_parser_set_field_columns(dsv_parser_t parser, size_t num_cols);

  /**
   *  \brief Get the required number of fields associated with future parsing
   *    with \c parser
   *
   *  See \c dsv_parser_set_field_columns for an explanation of the return
   *    values
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *
   *  \retval num_cols The number of columns required for future parsing of \c
   *    parser
   */
  size_t dsv_parser_get_field_columns(dsv_parser_t parser);







































  /**
   *  An opaque handle for a dsv operations object
   */
  typedef struct {
    void *p;
  } dsv_operations_t;

  /**
   *  \brief Initialize a dsv_operations_t object.
   *
   *  \note You must eventually call dsv_parser_destroy.
   *
   *  \param[in,out] operations A pointer to a dsv_operations_t object to
   *    initialize with default settings

   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_operations_create(dsv_operations_t *operations);

  /**
   *  \brief Destroy the dsv_operations_t object.
   *
   *  \post using \c parser with any function other than \c dsv_parser_create
   *    is undefined
   *
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   */
  void dsv_operations_destroy(dsv_operations_t operations);

  /**
   *  Callback operatons type definitions
   */

  /**
   *  \brief This function will be called for each header parsed in the file.
   *
   *  \note Certain settings such as RFC4180 will always produce exactly one
   *  header callback and it is up to the library caller to determine if the
   *  values of the header fields actually correspond to header values or record
   *  values. For example. RFC4180 uses an external mechanism to determine if
   *  the header is present. Therefore, if the parser behavior is set to
   *  strictly comply with RFC4180, the first line in the file will always be
   *  interpreted as a header.
   *
   *  An example method of traversing the fields is:
   *
   *  // assume the fields are known to be strings of ASCII characters
   *  for(size_t i=0; i<size; ++i) {
   *    // allocate space for copying--make sure we add space for the null
   *    // terminator
   *    size_t str_size = lengths[i]+1;
   *    char *str = (char*)malloc(str_size*sizeof(char));
   *    memcpy(str,fields[i],lengths[i]);
   *    str[lengths[i]] = '\0';
   *
   *    // str now contains a copy of the field, now do something interesting
   *  }
   *
   *  \param[in] fields \parblock
   *    An array of pointers to byte arrays each representing the parsed field.
   *    If the parsed field was surrounded by quotes, the byte array does not
   *    include these. Although each byte array may be an ASCII string depending
   *    on the assigned behavior, the array is NOT null-terminated.
   *    Additionally, no attempt is made at understanding the content of the
   *    string. For example, if the parsed field was the ASCII-representation of
   *    a number, no attempt is made at determining if the number will overflow
   *    or is a valid representation. Additionally, based on the assigned
   *    behavior and/or if quoted properly (see format documentation), other
   *    non-printing or non-ASCII characters may appear in the field such as
   *    newlines.
   *  \endparblock
   *  \param[in] lengths \parblock
   *    An array of values of type size_t representing the lengths of the
   *    respective byte array contained in \c fields. Each value is the exact
   *    number of the contained bytes.
   *  \endparblock
   *  \param[in] size The size of the field and length array
   *  \param[in] context A user-defined value associted with this callback set
   *  in \c dsv_set_header_callback
   *
   *  \retval nonzero if procssing should continue or 0 if processing should
   *  cease and control should return from the parse function. If a nonzero
   *  value is returned, the parse function will also return <0
   */
  typedef int (*header_callback_t)(const unsigned char *fields[],
    const size_t lengths[], size_t size, void *context);

  /**
   *  \brief Obtain the callback currently set for headers
   *
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   *
   *  \retval 0 No callback is registered
   *  \retval nonzero The currently registered callback
   */
  header_callback_t dsv_get_header_callback(dsv_operations_t operations);

  /**
   *  \brief Obtain the user-defined context currently set for header
   *
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   *
   *  \retval 0 No context is registered
   *  \retval nonzero The currently registered context
   */
  void * dsv_get_header_context(dsv_operations_t operations);

  /**
   *  \brief Associate the callback \c fn and a user-specified value \c context
   *  with \c operation.
   *
   *  \note The value of \c context is passed in as the \c context parameter in
   *  \c fn
   *
   *  \param[in] fn A function pointer conforming to \c header_callback_t
   *  \param[in] context A user defined pointer to be supplied in future
   *    calls to \c fn
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   */
  void dsv_set_header_callback(header_callback_t fn, void *context, dsv_operations_t operations);









  /**
   *  \brief This function will be called for each record parsed in the file.
   *  See the documentation for the definition of a record.
   *
   *  An example method of traversing the fields is:
   *
   *  // assume the fields are known to be strings of ASCII characters
   *  for(size_t i=0; i<size; ++i) {
   *    // allocate space for copying--make sure we add space
   *    // for the null terminator
   *    size_t str_size = lengths[i]+1;
   *    char *str = (char*)malloc(str_size*sizeof(char));
   *    memcpy(str,fields[i],lengths[i]);
   *    str[lengths[i]] = '\0';
   *
   *    // str now contains a copy of the field, now do something interesting
   *  }
   *
   *  \param[in] fields \parblock
   *    An array of pointers to byte arrays each representing the parsed field.
   *    If the parsed field was surrounded by quotes, the byte array does not
   *    include these. Although each byte array may be an ASCII string depending
   *    on the assigned behavior, the array is NOT null-terminated.
   *    Additionally, no attempt is made at understanding the content of the
   *    string. For example, if the parsed field was the ASCII-representation of
   *    a number, no attempt is made at determining if the number will overflow
   *    or is a valid representation. Additionally, based on the assigned
   *    behavior and/or if quoted properly (see format documentation), other
   *    non-printing or non-ASCII characters may appear in the field such as
   *    newlines.
   *  \endparblock
   *  \param[in] lengths \parblock
   *    An array of values of type size_t representing the lengths of the
   *    respective byte array contained in \c fields. Each value is the exact
   *    number of the contained bytes.
   *  \endparblock
   *  \param[in] size The size of the field and length array
   *  \param[in] context A user-defined value associted with this callback set
   *  in \c dsv_set_header_callback
   *
   *  \retval nonzero if procssing should continue or 0 if processing should
   *  cease and control should return from the parse function. If a nonzero
   *  value is returned, the parse function will also return <0
   */
  typedef int (*record_callback_t)(const unsigned char *fields[],
    const size_t lengths[], size_t size, void *context);

  /**
   *  \brief Obtain the callback currently set for records
   *
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create

   *  \retval 0 No callback is registered
   *  \retval nonzero The currently registered callback
   */
  record_callback_t dsv_get_record_callback(dsv_operations_t operations);

  /**
   *  \brief Obtain the user-defined context currently set for records
   *
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   *
   *  \retval 0 No context is registered
   *  \retval nonzero The currently registered context
   */
  void * dsv_get_record_context(dsv_operations_t operations);

  /**
   *  \brief Associate the callback \c fn and a user-specified value \c context
   *  with \c operation.
   *
   *  \note The value of \c context is passed in as the \c context parameter
   *  in \c fn
   *
   *  \param[in] fn A function pointer conforming to \c record_callback_t
   *  \param[in] context A user defined pointer to be supplied in future
   *    calls to \c fn
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   */
  void dsv_set_record_callback(record_callback_t fn, void *context, dsv_operations_t operations);

  /**
   *  \brief Parse the file stream \c stream with description \location_str with
   *  \c parser, using the operations contained in \c operations. If \c stream
   *  is \c NULL, then attempt to open the location \location_str using fopen.
   *
   *

   *  If the location_str begins with the '/' character, then the file is
   *  understood to be an absolute path starting at the root directory. If the
   *  filename does not begin with a '/' character, the file is understood to
   *  be relative to the current working directory. In either case any
   *  additional relative file inclusion mechanism will be relative to the
   *  directory that holds the input file.
   *
   *  If unable to successfully parse 'filename' a negative value will be
   *  returned and the reason can be obtained by \c dsv_parse_error.
   *
   *  Warnings or information about non-implemented features will return success
   *  and be reported via \c dsv_parse_error.
   *
   *  \param[in] location_str \parblock
   *    A null-terminated byte string (NTBS) that is used to identify and
   *    potentially locate the content to be parsed. Regardless of how the value
   *    is used to initiate parsing, it is supplied as a reference value for
   *    logging messages. See \c dsv_log_code.
   *      - If the \c stream parameter is zero, the \c location_str parameter is
   *        taken to mean the location of a file to be opened and parsed. If the
   *        location_str begins with the '/' character, then the file is
   *        understood to be an absolute path starting at the root directory. If
   *        the filename does not begin with a '/' character, the file is
   *        understood to be relative to the current working directory. The
   *        parse function will open the file for reading and parse the
   *        contents.
   *      - If the \c stream parameter is non-zero, the \c location_str
   *        parameter is not referenced and simply sued as an identifier for log
   *        messages.
   *  \endparblock
   *  \param[in] stream \parblock
   *    - If nonzero, the value is assumed to be a valid file stream opened for
   *      reading and the content will be parsed. \c dsv_parse does not close
   *      the stream when finished.
   *    - If zero, \c dsv_parse will attempt to use the value of \c location_str
   *      to open a file location for parsing.
   *  \endparblock
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] operations A pointer to a dsv_operations_t object previously
   *    initialized with \c dsv_operations_create
   *
   *  \retval 0 success
   *  \retval ENOMEM out of memory
   *  \retval >0 Any error code returned by fopen
   *  \retval <0 failure, see dsv_parse_error
   */
  int dsv_parse(const char *location_str, FILE *stream, dsv_parser_t parser,
                dsv_operations_t operations);


  /**
   *  \brief Logging levels for parser messages
   */
  typedef enum {
    dsv_log_none = 0,
    dsv_log_error = 1L,
    dsv_log_warning = (1L << 1),
    dsv_log_info = (1L << 2),
    dsv_log_debug = (1L << 3),
    dsv_log_all = (dsv_log_error|dsv_log_warning|dsv_log_info|dsv_log_debug)
  } dsv_log_level;

  /**
   *  \brief Logging message codes
   */
  typedef enum {
    /**
     *  \brief An error strictly associated with incorrect syntax based on the
     *  current parser behavior.
     *
     *  This log code has the following parameters:
     *    - The offending line associated with the start of the syntax
     *      error[*][**]
     *    - The offending line associated with the end of the syntax
     *      error[*][**]
     *    - The offending character associated with the start of the syntax
     *      error[*]
     *    - The offending character associated with the end of the syntax
     *      error[*]
     *    - The location_str associated with the syntax error if it was supplied
     *      to \c dsv_parse
     *
     *  [*] Numbers provided as a string are capable of being translated to
     *  a signed or unsigned integer value (ie strtoul).
     *
     *  [**] The line associated with the log is counted according to the
     *  applied parser behavior. For example, if the system newline is LF and
     *  the applied behavior is RFC4180-strict, then the lines will be counted
     *  based on the occurrence of CRLF. If the behavior is newline_permissive,
     *  the lines will be counted based on the first parsed occurrence of a
     *  newline. Note that this is different than the first seen occurence of a
     *  newline. For example, if a record is "..LF.."CRLF, the LF is parsed as a
     *  quoted field and not a newline therefore the registered newline behavior
     *  will be the CRLF.
    */
    dsv_syntax_error,

    /**
     *  \brief An error strictly associated with parsing a non-uniform number of
     *  fields when explicitly requested to do so. For example, if the header
     *  contains 5 fields but the first record only contains 3.
     *
     *  This log code has the following parameters:
     *    - The line number associated with the start of the offending
     *      row[*][**]
     *    - The line number associated with the end of the offending row[*][**]
     *    - The expected number of fields[*]
     *    - The number of fields parsed for this row[*]
     *    - The location_str associated with the syntax error if it was supplied
     *      to \c dsv_parse
     *
     *  [*] Numbers provided as a string are capable of being translated to
     *  a signed or unsigned integer value (ie strtoul).
     *
     *  [**] The line associated with the log is counted according to the
     *  applied parser behavior. For example, if the system newline is LF and
     *  the applied behavior is RFC4180-strict, then the lines will be counted
     *  based on the occurrence of CRLF. If the behavior is newline_permissive,
     *  the lines will be counted based on the first parsed occurrence of a
     *  newline. Note that this is different than the first seen occurence of a
     *  newline. For example, if a record is "..LF.."CRLF, the LF is parsed as a
     *  quoted field and not a newline therefore the registered newline behavior
     *  will be the CRLF.
    */
    dsv_inconsistant_column_count,

    /**
     *  \brief An message associated with settings that prohibit non-ASCII
     *  characters appearing in quoted fields. This error is also thrown if a
     *  invalid newline representation appears in RFC4180-strict mode.
     *
     *  This log code has the following parameters:
     *    - The offending line associated with the start of the syntax
     *      error[*][**]
     *    - The offending line associated with the end of the syntax
     *      error[*][**]
     *    - The offending character associated with the start of the syntax
     *      error[*]
     *    - The offending character associated with the end of the syntax
     *      error[*]
     *    - A byte-oriented string containing the hexadecimal representation of
     *      the offending binary content.[***]
     *    - The location_str associated with the syntax error if it was supplied
     *      to \c dsv_parse
     *
     *  [*] Numbers provided as a string are capable of being translated to
     *  a signed or unsigned integer value (ie strtoul).
     *
     *  [**] The line associated with the log is counted according to the
     *  applied parser behavior. For example, if the system newline is LF and
     *  the applied behavior is RFC4180-strict, then the lines will be counted
     *  based on the occurrence of CRLF. If the behavior is newline_permissive,
     *  the lines will be counted based on the first parsed occurrence of a
     *  newline. Note that this is different than the first seen occurence of a
     *  newline. For example, if a record is "..LF.."CRLF, the LF is parsed as a
     *  quoted field and not a newline therefore the registered newline behavior
     *  will be the CRLF.
     *
     *  [***] Each byte of the hexadecimal representation of binary content is
     *  prefixed by a '0x' and therefore is capable of being translated to a
     *  signed or unsigned integer value (ie strtol and family).
    */
    dsv_unexpected_binary
  } dsv_log_code;


  /**
   *  \brief This function will be called each time a message is logged by the
   *  parser according to the set logging level.
   *
   *  \param[in] code The \c dsv_log_code associated with this message
   *  \param[in] level \parblock
   *    The \c dsv_log_level that this code was generated against. This value
   *    may be different depending on the parser settings for a given \c
   *    dsv_log_code. For example, under settings that define strict behavior,
   *    the message may be considered and error. Under permissive settings the
   *    same message may just be a warning.
   *  \endparblock
   *  \param[in] params \parblock
   *    A c-array of null-terminated byte strings containing the parameters
   *    associated with the given \c dsv_log_code (see \c dsv_log_code for an
   *    explanation of the different parameters for each \c dsv_log_code).
   *  \endparblock
   *  \param[in] size The size of the params array
   *  \param[in] context A user-defined value associated with this callback set
   *  in \c dsv_set_log_callback
   *
   *  \retval nonzero if procssing should continue or 0 if processing should
   *  cease and control should return from the parse function. If a nonzero
   *  value is returned, the parse function will also return <0. Use this return
   *  value if an outside mechanism determines that parsing should cease based
   *  on the log message. NB that the return value is ignored in the case of
   *  error messages as they will always cause processing to cease and a nonzero
   *  value to be returned from from the parse function.
   */
  typedef int (*log_callback_t)(dsv_log_code code, dsv_log_level level,
    const char *params[], size_t size, void *context);

  /**
   *  \brief Obtain the callback currently set for log messages
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *
   *  \retval 0 No callback is registered
   *  \retval nonzero The currently registered callback
   */
  log_callback_t dsv_get_logger_callback(dsv_parser_t parser);

  /**
   *  \brief Obtain the user-defined context currently set for header
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *
   *  \retval 0 No context is registered
   *  \retval nonzero The currently registered context
   */
  void * dsv_get_logger_context(dsv_parser_t parser);

  /**
   *  \brief Obtain the log level set for future parsing with \c parser
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *
   *  \retval level The current set log level
   */
  dsv_log_level dsv_get_log_level(dsv_parser_t parser);

  /**
   *  \brief Associate the logging callback \c fn, a user-specified \c context,
   *  for logging \c level with \c parser.
   *
   *  \note The value of \c context is passed in as the \c context parameter
   *  in \c fn
   *
   *  \note The value of \c level may not be the value passed in \c fn. See
   *  \c log_callback_t
   *
   *  \param[in] fn A function pointer conforming to \c log_callback_t
   *  \param[in] context A user-defined value to associate with this callback
   *    and provide in future calls to \c fn
   *  \param[in] level \parablock
   *    A filter to apply to the generated log messages. When parsing encounters
   *    a condition that would result in a log messaged being generated, it is
   *    OR'ed against the value of \c level. If the potential messaged passes
   *    the check, \c fn is called with the supplied parameters.
   *  \endparblock
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *
   */
  void dsv_set_logger_callback(log_callback_t fn, void *context,
    dsv_log_level level, dsv_parser_t parser);





  // HIGH LEVEL INTERFACE

  /**
   *  \brief Initialize a dsv_parser_t object with strict RFC4180 settings
   *
   *  This parser is set up with the following settings:
   *    :dsv_newline_behavior = dsv_newline_RFC4180_strict
   *    :dsv_parser_set_field_columns(...,0)
   *    :dsv_parser_set_field_delimiter(...,',')
   *
   *  \note You must eventually call dsv_parser_destroy.
   *
   *  \param[in,out] parser A pointer to a dsv_parser_t object to initialize
   *    with settings suitable for RFC4180 strict parsing
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_create_RFC4180_strict(dsv_parser_t *parser);

  /**
   *  \brief Initialize a dsv_parser_t object with permissive RFC4180 settings
   *
   *  This parser is set up with the following settings:
   *    :dsv_newline_behavior = dsv_newline_permissive
   *    :dsv_parser_set_field_columns(...,0)
   *    :dsv_parser_set_field_delimiter(...,',')
   *
   *  \note You must eventually call dsv_parser_destroy.
   *
   *  \param[in,out] parser A pointer to a dsv_parser_t object to initialize
   *    with settings suitable for RFC4180 permissive parsing
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_create_RFC4180_permissive(dsv_parser_t *parser);

  /**
   *  \brief Behavior flag shortcuts for handling newlines
   *
   *  There are three types of supported newline behavior shortcuts:
   *  permissive (\c dsv_newline_permissive), strict carriage-return followed by
   *  a line-feed (\c dsv_newline_crlf_strict), and strict line-feed
   *  (\c dsv_newline_lf_strict). A fourth supported designator
   *  \c dsv_newline_RFC4180_strict is a synonym for \c dsv_newline_crlf_strict.
   *
   *  Permissive mode (\c dsv_newline_permissive) means that the first
   *  occurrence of one of the supported newline types will set the newline
   *  behavior for the remainder of of the parsing. For example, an input of
   *  CRLF CRLF is accepted in permissive mode as is LF LF but CRLF LF is an
   *  error as is LF CRLF. Note: this also means newlines embedded in quoted
   *  fields. That is, "LF" LF is accepted but "LF" CRLF is an error.
   *
   *  These are shortcuts for more complicated record completing handling
   */
  typedef enum {
    /* Accept all supported newlines [DEFAULT] */
    dsv_newline_permissive = 0,

    /** Only accept lines terminated by the Line Feed (LF).  [*nix-based systems
     *  including Mac OSX]
     */
    dsv_newline_lf_strict = 1,

    /** Only accept lines terminated by both the Carriage Return (CRL) and the
     *  Line Feed (LF). [MS Windows]
     */
    dsv_newline_crlf_strict = 2,

    /** RFC 4180 strict. Equivalent to dsv_newline_crlf_strict
     */
    dsv_newline_RFC4180_strict = 2
  } dsv_newline_behavior;


  /**
   *  \brief Set the newline (record terminating) behavior for future parsing
   *  with \c parser
   *
   *  The default value is \c dsv_newline_permissive
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param behavior One of the possible \c dsv_newline_behavior enumerations
   *
   *  This is a convenience function for \c
   *  dsv_parser_set_equiv_record_delimiters with the appropriate newline
   *  sequence
   *
   *  \retval 0 Success
   *  \retval EINVAL \c behavior has a value not part of dsv_newline_behavior
   */
  int dsv_parser_set_newline_behavior(dsv_parser_t parser,
    dsv_newline_behavior behavior);



  /**
   *  \brief Set a single, nonrepeating byte field delimiter to be used for
   *  future parsing with \c parser
   *
   *  This is a convenience function and is equivalent to:
   *    dsv_parser_t parser; // assume exists
   *    unsigned char delim ; // assume exists
   *    const unsigned char *delim_arr[1] = {&delim};
   *    size_t delimsize_arr[1] = {1};
   *    int delimrepeat_arr[1] = {0};
   *    err = dsv_parser_set_equiv_field_delimiters(_parser,delim_arr,
   *      delimsize_arr,delimrepeat_arr,1,0,1);
   *
   *  The default is the ASCII comma ','
   *
   *  This delimiter is used to separate both headers and fields depending on
   *  the settings
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param delim The character to be used as a field delimiter
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_set_field_delimiter(dsv_parser_t parser, unsigned char delim);

  /**
   *  \brief Set a nonrepeating multibyte field delimiter to be used for future
   *  parsing with \c parser
   *
   *  This is a convenience function and is equivalent to:
   *    dsv_parser_t parser; // assume exists
   *    unsigned char delim[] ; // assume exists
   *    size_t delimsize; // assume exists and equals length of delim
   *    const unsigned char *delim_arr[1] = {delim};
   *    size_t delimsize_arr[1] = {size};
   *    int delimrepeat_arr[1] = {0};
   *    err = dsv_parser_set_equiv_field_delimiters(_parser,delim_arr,
   *      delimsize_arr,delimrepeat_arr,1,0,1);
   *
   *  The default is the ASCII comma ','
   *
   *  Multibyte sequences can be used to support other character encodings such
   *  as UTF-8
   *
   *  This delimiter is used to separate both headers and fields depending on
   *  the settings
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] delim A sequence of bytes to be used as a field delimiter
   *  \param[in] size The size of the delimiter sequence \c delim
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_set_field_wdelimiters(dsv_parser_t parser,
    const unsigned char *delim, size_t size);

















#if defined(__cplusplus)
}
#endif

#endif
