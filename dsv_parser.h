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
   *  \brief Initialize a default dsv_parser_t object.
   *
   *  This parser is set up with the following default settings:
   *    :dsv_newline_behavior = dsv_newline_permissive
   *    :dsv_parser_set_field_columns(...,0)
   *    :dsv_parser_set_field_delimiter(...,',')
   *
   *  \note You must eventually call dsv_parser_destroy.
   *
   *  \param[in,out] parser A pointer to a dsv_parser_t object to initialize
   *    with default settings
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_create(dsv_parser_t *parser);

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
   *  \brief Destroy the obj_parser_t object.
   *
   *  \post using \c parser with any function other than \c dsv_parser_create
   *    is undefined
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with \c dsv_parser_create*
   */
  void dsv_parser_destroy(dsv_parser_t parser);


  /**
   *  \brief Behavior flag for handling newlines
   *
   *  There are three types of supported newline behaviors:
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
   *  \brief Set the newline behavior for future parsing with \c parser
   *
   *  The default value is \c dsv_newline_permissive
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param behavior One of the possible \c dsv_newline_behavior enumerations
   *
   *  \retval 0 Success
   *  \retval EINVAL \c behavior has a value not part of dsv_newline_behavior
   */
  int dsv_parser_set_newline_behavior(dsv_parser_t parser,
    dsv_newline_behavior behavior);

  /**
   *  \brief Get the current newline behavior for future parsing with \c parser
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *
   *  \retval behavior One of the possible \c dsv_newline_behavior enumerations
   */
  dsv_newline_behavior dsv_parser_get_newline_behavior(dsv_parser_t parser);

  /**
   *  \brief Set the required number of fields for future parsing with \c parser
   *  or allow a non-uniform number.
   *
   *  If the behavior specified by \c dsv_parser_fixed_field_columns is
   *  violated, dsv_parse will immediately return a nonzero value and an error
   *  message will be logged with the code: \c dsv_column_count_error.
   *
   *  The default value is 0. This value is also appropriate for RFC4180-strict
   *  processing
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] num_cols \parblock
   *    If > 0, the number of columns expected during future
   *    parsing. If during parsing, a row with less than \c num_cols is
   *    encountered, dsv_parse will immediately return with a nonzero value. If
   *    \c num_cols == 0, the parser will set the required number of columns
   *     based on the first row encountered. For example, if the first header
   *    row contains 5 columns, all subsequent rows must contain 5 columns
   *    otherwise the dsv_parse will immediately return a nonzero value. If
   *    \c num_cols == -1, no restriction will be placed on the number of
   *    columns. This also means that rows with zero columns are acceptable. In
   *    this case, any registered callback will still be called.
   *  \endparblock
   */
  void dsv_parser_set_field_columns(dsv_parser_t parser, ssize_t num_cols);

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
  ssize_t dsv_parser_get_field_columns(dsv_parser_t parser);




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
   *    err = dsv_parser_set_field_wdelimiter_equiv(_parser,delim_arr,
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
   *    err = dsv_parser_set_field_wdelimiter_equiv(_parser,delim_arr,
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
  int dsv_parser_set_field_wdelimiter(dsv_parser_t parser,
    const unsigned char *delim, size_t size);


  /**
   *  \brief Set equivalent, potentially repeating, and optionally
   *  parse-exclusive multibyte field delimiters to be used for future parsing
   *  with \c parser.
   *
   *  Equivalent delimiters are single or multibyte sequences such that if any
   *  sequence is seen, it is considered an acceptable delimiter. For example,
   *  in an RFC 4180-strict parser, a single non-repeating ASCII comma is the
   *  only acceptable delimiter. However, in other formats, any single or
   *  repeating sequence of white spaces are considered in whole as an
   *  acceptable delimiter. For example, repeating ASCII space or tab
   *  characters. Said another way, given some sequence representing a delimiter
   *  \c DELIM, it is equivalent to the regular expression DELIM*. It is
   *  possible to set the delimiter as repeating on a per-delimiter sequence
   *  basis. For example, given several sequences representing equivalent
   *  delimiters \c {DELIM1,DELIM2,DELIM3,...}, it is equivalent to the regular
   *  expression (DELIM1*|DELIM2*|DELIM3*|...). It is also possible to allow the
   *  entire equivalent delimiter set to repeat. That is, for some delimiter
   *  sequence set \c {DELIM1,DELIM2,DELIM3,...} it is equivalent to the regular
   *  expression \c (DELIM1|DELIM2|DELIM3|...)*. These repeating flag can be
   *  mixed in arbitrary ways. For example, setting \c DELIM1 and DELIM3 to
   *  repeat only along with the entire equivalent delimiter set is equivalent
   *  to the regular expression \c (DELIM1*|DELIM2|DELIM3*|...)*.
   *
   *  Parse exclusivity means that the first equivalent byte sequence parsed as a delimiter becomes the only acceptable delimiter for the remainder of the parsing.

   Setting a single delimiter sequence to repeat is equivalent to setting the entire delimiter set consisting of a single delimiter to repeat but the parser achieves the same result in different ways. That is \c (DELIM*) achieves the same result as \c (DELIM)*. It is also possible to enable parse-level exclusivity. That is, once one of the given delimiters is parsed, it becomes the only valid delimiter for the remainder of the parse operation.
   *
   *  \note If unlimited repeating delimiter sequences is enabled, it becomes
   *  impossible to represent an empty field based solely on repeating the
   *  delimiter. For example, given the default delimiter of an ASCII comma ',',
   *  two consecutive commas would represent an empty field. That is:
   *
   *    "foo",,"bar"
   *
   *  would have three fields, "foo", [[empty]], and "bar". If, for example.
   *  repeating whitespace is set as the delimiter, the input "foo" and "bar"
   *  separated by two spaces:
   *
   *    "foo"  "bar"
   *
   *  represents two fields "foo" and "bar"
   *
   *  The default delimiter is a single, non-repeating ASCII comma ','
   *
   *  Multibyte sequences can be used to support other character encodings such
   *  as UTF-8
   *
   *  This delimiter is used to separate both headers and fields depending on
   *  the settings
   *
   *  \note There must always be a field delimiter. That is, setting \c size to
   *  zero is invalid.
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] delim \parablock
   *    An array of length  \c size such that the ith element is a pointer to a
   *    sequence of bytes of size \c delimsize[i] to be used as a field
   *    delimiter.
   *  \endparblock
   *  \param[in] delimsize \parablock
   *    An array of length \c size such that the ith element is the size of the
   *    ith sequence of bytes pointed to by \c delim[i].
   *  \endparblock
   *  \param[in] delim_repeat \parablock
   *    An array of length \c size such that if the ith element is nonzero, the
   *    ith delimiter in \c delim may be repeated indefinitely.
   *  \endparblock
   *  \param[in] size The size of each arrays \c delim, \c delimsize,
   *    \c repeatflag. If \c size is zero, this call has no effect on the
   *    previous delimiter state.
   *  \param[in] repeatflag If nonzero, any delimiter chosen from \c delim my be
   *    repeated an indefinite number of times.
   *  \param[in] exclusiveflag \parblock
   *    If nonzero, then the first delimiter sequence represented by \c delim
   *    according to the repeat rules set in \c repeat is encountered, it
   *    becomes the only valid delimiter sequence for the remainder of the
   *    parser operation. This is reset when a new parse operation begins.
   *  \endparblock
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   *  \retval EINVAL Either \c size or an element of delimsize is zero
   */
  int dsv_parser_set_field_wdelimiter_equiv(dsv_parser_t parser,
    const unsigned char *delim[], const size_t delimsize[],
    const int delim_repeat[], size_t size, int repeatflag, int exclusiveflag);








  /**
   *  \brief Obtain the number of field delimiters currently assigned to
   *  \c parser
   *
   *  The returned values is the same as the \c size parameter in
   *  \c dsv_parser_set_field_wdelimiter_equiv
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \retval The number of field delimiters set for \c parser
   */
  size_t dsv_parser_num_field_delimiters(dsv_parser_t parser);

  /**
   *  \brief Obtain whether or not the field delimiters assigned to \parser are
   *  allowed to repeat indefinitely
   *
   *  The returned values is the same as the \c repeatflag parameter in
   *  \c dsv_parser_set_field_wdelimiter_equiv
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \retval If nonzero, the currently assigned field delimiters to \c parser
   *    are allowed to repeat indefinitely
   */
  int dsv_parser_get_field_delimiters_repeatflag(dsv_parser_t parser);

  /**
   *  \brief Obtain whether or not the first parsed field delimiter assigned to
   *  \parser is the only permitted subsequent delimiter for the remainder of
   *  parsing
   *
   *  The returned values is the same as the \c exclusiveflag parameter in
   *  \c dsv_parser_set_field_wdelimiter_equiv
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \retval If nonzero, the first delimiter parsed among the currently
   *    assigned field delimiters in\c parser shall be the only permitted
   *    delimiter for the remainder of the parsing session.
   */
  int dsv_parser_get_field_delimiters_exclusiveflag(dsv_parser_t parser);

  /**
   *  \brief Copy the \c n th field delimiter to be used for future parsing with
   *  \c parser into the buffer \c buf of size \c bufsize and set the location
   *  pointed to by \c repeatflag as to if the delimiter was allowed to be
   *  repeated indefinitely.
   *
   *  This delimiter is used to separate both headers and fields depending on
   *  the settings.
   *
   *  If \c bufsize is zero, return the number of bytes needed to hold the
   *  current set delimiter. This number is suitable for allocating memory for
   *  \c buf. If \c bufsize is nonzero, copy the bytes representing the
   *  delimiter or \c bufsize whichever is smaller and return this value. If \c
   *  bufsize is zero, \c buf is not referenced and may be zero for the call.
   *
   *  If \c n is a valid value, and \c repeatflag is nonzero, it will be set to
   *  the repeat value of the \c n th delimiter regardless of the value of \c
   *  buf and \c buffsize
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in,out] buf If \c bufsize is nonzero, an unsigned char buffer of
   *    size \bufsize to which the current field delimiter will be copied into.
   *    N.B. since the sequence of bytes contained in \c buf may not represent a
   *    string, no null terminator will be added to the end of the bytes.
   *  \param [in] bufsize The size of the unsigned char buffer pointed to
   *    by \c buf.
   *  \param [in,out] repeatflag If \c repeatflag is nonzero, set the location
   *    pointed to by \c repeatflag to a value if nonzero indicates that the
   *    \c th delimiter can be repeated indefinitely.
   *  \retval If \c bufsize is zero, return the number of bytes needed to hold
   *    the current delimiter. If \c bufsize is nonzero, return the number of
   *    bytes copied to \c buf which is not necessarily the same size as \c
   *    bufsize.
   * \retval 0 \c n is greater than the number of field delimiters currently set
   *    for \c parser
   */
  size_t dsv_parser_get_field_delimiter(dsv_parser_t parser, size_t n,
    unsigned char *buf, size_t bufsize, int *repeatflag);

  /**
   *  \brief Enable or disable binary in double quoted fields for future parsing
   *  with \c parser
   *
   *  The default setting is 0 (false)
   *
   *  Under RFC-4180, the file shall only contain ASCII printable characters.
   *  When enabled, this turns off most character translation in double quoted
   *  fields. Double quotes are still recognized as well as double quote
   *  preceded by another double quote.
   *
   *  Enabling is useful if the fields contain non-printing but otherwise useful
   *  ASCII characters as well as allowing for non-ASCII encoding such as UTF-8.
   *  The library makes no attempt to translate any such characters.
   *
   *  The downside is that newline interpretation is also turned off. Therefore
   *  only newlines outside of the double quoted fields are counted for location
   *  tracking. This means that if syntax errors are encountered, it can make
   *  locating them difficult.
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] flag nonzero to enable, zero to disable
   */
  void dsv_parser_allow_escaped_binary_fields(dsv_parser_t parser, int flag);

  /**
   *  \brief Query the whether binary is recognized in double quoted fields for
   *  future parsing with \c parser
   *
   *  Under RFC-4180, the file shall only contain ASCII printable characters.
   *  When enabled, this turns off character translation in double quoted fields
   *  (double quotes are till recognized obviously--this includes double quote
   *  preceded by another double quote.)
   *
   *  Enabling is useful if the fields contain non-printing but otherwise useful
   *  ASCII characters as well as allowing for non-ASCII encoding such as UTF-8.
   *
   *  \param[in] parser A pointer to a dsv_parser_t object previously
   *    initialized with one of the \c dsv_parser_create* functions
   *  \param[in] flag nonzero to enable, zero to disable
   */
  int dsv_parser_escaped_binary_fields_allowed(dsv_parser_t parser);



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

#if defined(__cplusplus)
}
#endif

#endif
