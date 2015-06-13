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
   *  \brief Initialize a dsv_parser_t object.
   *
   *  \note You must eventually call dsv_parser_destroy.
   *
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_parser_create(dsv_parser_t *parser);

  /**
   *  \brief Destroy the obj_parser_t object.
   *
   *  \pre \c parser has been previously initialized with \c dsv_parser_create
   */
  void dsv_parser_destroy(dsv_parser_t parser);

  /**
   *  \brief Behavior flag for handling newlines
   *
   *  There are three types of supported newline behaviors:
   *  permissive (\c dsv_newline_permissive), strict carriage-return followed by a
   *  line-feed (\c dsv_newline_crlf_strict), and strict line-feed
   *  (\c dsv_newline_lf_strict). A fourth supported designator
   *  \c dsv_newline_RFC4180_strict is a synonym for \c dsv_newline_crlf_strict.
   *
   *  Permissive mode (\c dsv_newline_permissive) means that the first occurrence of one
   *  of the supported newline types will set the newline behavior for the remainder of
   *  of the parsing. For example, an input of CRLF CRLF is accepted in permissive mode
   *  as is LF LF but CRLF LF is an error as is LF CRLF.
   *
   */
  typedef enum {
    /* Accept all supported newlines [DEFAULT] */
    dsv_newline_permissive = 0,

    /** Only accept lines terminated by the Line Feed (LF).  [*nix-based systems
     *  including Mac OSX]
     */
    dsv_newline_lf_strict = 1,

    /** Only accept lines terminated by both the Carriage Return (CRL) and the the
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
   *  \param parser A properly initialized dsv_parser_t object
   *  \param behavior One of the possible \c dsv_newline_behavior enumerations
   *
   *  \retval 0 Success
   *  \retval EINVAL \c behavior has a value not part of dsv_newline_behavior
   */
  int dsv_parser_set_newline_handling(dsv_parser_t parser, dsv_newline_behavior behavior);

  /**
   *  \brief Get the current newline behavior for future parsing with \c parser
   *
   *  \param parser A properly initialized dsv_parser_t object
   *
   *  \retval behavior One of the possible \c dsv_newline_behavior enumerations
   */
  dsv_newline_behavior dsv_parser_get_newline_handling(dsv_parser_t parser);





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
   *  \retval 0 success
   *  \retval ENOMEM Could not allocate memory
   */
  int dsv_operations_create(dsv_operations_t *operations);

  /**
   *  \brief Destroy the dsv_operations_t object.
   *
   *  \pre \c operations has been previously initialized with \c dsv_operations_t
   */
  void dsv_operations_destroy(dsv_operations_t operations);

  /**
   *  Callback operatons type definitions
   */

  /**
   *  \brief This function will be called for each header parsed in the file.
   *
   *  \note Certain settings such as RFC4180 will always produce exactly 1 header
   *  callback and it is up to the library caller to determine if the values of the
   *  header fields actually correspond to header values or record values. For example.
   *  RFC4180 uses an external mechanism to determine if the header is present.
   *  Therefore, if the parser behavior is set to strictly comply with RFC4180, the
   *  first line in the file will always be interpreted as a header.
   *
   *  \param[in] fields \parblock
   *  A c-array of null-terminated byte strings containing each parsed field. If
   *  the field was surrounded by quotes, the string does not inlcude this. No
   *  attempt is made at understanding the content of the string. For example, if
   *  the string contains a number, no attempt is made at determining if the
   *  number will not overflow. Additionally, if quoted properly (see format
   *  documentation), certain non-printing characters may appear in the field
   *  such as newlines. In all cases however, since delimited seperated values
   *  are strictly an ASCII format, no binary shall appear in the field.
   *  \endparblock
   *  \param[in] size The size of the field array
   *  \param[in] context A user-defined value associted with this callback set in
   *                      \c dsv_set_header_callback
   *
   *  \retval nonzero if procssing should continue or 0 if processing should cease
   *          and control should return from the parse function.
   */
  typedef int (*header_callback_t)(const char *fields[], size_t size, void *context);

  /**
   *  \brief Obtain the callback currently set for headers
   *
   *  \retval 0 No callback is registered
   *  \retval nonzero The currently registered callback
   */
  header_callback_t dsv_get_header_callback(dsv_operations_t operations);

  /**
   *  \brief Obtain the user-defined context currently set for header
   *
   *  \retval 0 No context is registered
   *  \retval nonzero The currently registered context
   */
  void * dsv_get_header_context(dsv_operations_t operations);

  /**
   *  \brief Associate the callback \c fn and a user-specified value \c context
   *  with \c operation.
   *
   *  \note The value of \c context is passed in as the \c context parameter in \c fn
   */
  void dsv_set_header_callback(header_callback_t fn, void *context, dsv_operations_t operations);









  /**
   *  \brief This function will be called for each record parsed in the file. See
   *  the documentation for the definition of a record.
   *
   *  \param[in] fields \parblock
   *  A c-array of null-terminated byte strings containing each parsed field. If
   *  the field was surrounded by quotes, the string does not inlcude this. No
   *  attempt is made at understanding the content of the string. For example, if
   *  the string contains a number, no attempt is made at determining if the
   *  number will not overflow. Additionally, if quoted properly (see format
   *  documentation), certain non-printing characters may appear in the field
   *  such as newlines. In all cases however, since delimited seperated values
   *  are strictly an ASCII format, no binary shall appear in the field.
   *  \endparblock
   *  \param[in] size The size of the field array
   *  \param[in] context A user-defined value associted with this callback set in
   *                      \c dsv_set_record_callback
   *
   *  \retval nonzero if procssing should continue or 0 if processing should cease
   *          and control should return from the parse function.
   */
  typedef int (*record_callback_t)(const char *fields[], size_t size, void *context);

  /**
   *  \brief Obtain the callback currently set for records
   *
   *  \retval 0 No callback is registered
   *  \retval nonzero The currently registered callback
   */
  record_callback_t dsv_get_record_callback(dsv_operations_t operations);

  /**
   *  \brief Obtain the user-defined context currently set for records
   *
   *  \retval 0 No context is registered
   *  \retval nonzero The currently registered context
   */
  void * dsv_get_record_context(dsv_operations_t operations);

  /**
   *  \brief Associate the callback \c fn and a user-specified value \c context
   *  with \c operation.
   *
   *  \note The value of \c context is passed in as the \c context parameter in \c fn
   */
  void dsv_set_record_callback(record_callback_t fn, void *context, dsv_operations_t operations);

  /**
   *  \brief Parse the file stream \c stream with description \location_str with
   *  \c parser, using the operations contained in \c operations. If \c stream is \c NULL,
   *  then attempt to open the location \location_str using fopen.
   *
   *  If the filename begins with the '/' character, then the file is
   *  understood to be an absolute path starting at the root directory. If the
   *  filename does not begin with a '/' character, the file is understood to
   *  be relative to the current working directory. In either case any additional
   *  relative file inclusion mechanism will be relative to the directory that
   *  holds the input file.
   *
   *  If unable to successfully parse 'filename' a negative value will be
   *  returned and the reason can be obtained by \c dsv_parse_error.
   *
   *  Warnings or information about non-implemented features will return success
   *  and be reported via \c dsv_parse_error.
   *
   *  \retval 0 success
   *  \retval ENOMEM out of memory
   *  \retval >0 Any error code returned by fopen
   *  \retval <0 failure, see dsv_parse_error
   */
  int dsv_parse(const char *location_str, FILE *stream, dsv_parser_t parser,
                dsv_operations_t operations);


  /**
   *  \brief Logging levels for filtering parser messages
   */
  typedef enum {
    dsv_log_none = 0, /*!< Filter all messages */
    dsv_log_error = 1L, /*!< Filter all messages except error messages */
    dsv_log_warning = 1L << 1, /*!< Filter all messages except warning messages */
    dsv_log_info = 1L << 2, /*!< Filter all messages except info messages */
    dsv_log_all = (dsv_log_error|dsv_log_warning|dsv_log_info) /*!< Do not filter messages */
  } dsv_log_level;

  /**
   *  \brief Logging message codes
   */
  typedef enum {
    /*
      \brief An error strictly associated with incorrect syntax based on the current
      parser behavior.

      \param string A string containing the unexpected value
    */
    dsv_parse_error = 0
  } dsv_log_code;

  /**
   *  \brief Return the number of log messages associated with the last parse operation
   *  according to the given \c log_level.
   *
   *  \param[in] parser A \c dsv_parser_t object previously initialized with
   *                    \c dsv_parser_create
   *  \param[in] log_level A valid value of \c dsv_log_level indicating how the
   *                        the list of all log messages should be filtered.
   *  \retval num Number of message filtered according to \c log_level
   */
  size_t dsv_parse_log_count(dsv_parser_t _parser, dsv_log_level log_level);

  /**
   *  \brief Filter the log messages associated with the last parse operation
   *  according to the given \c log_level retrieve the \c num message and copy the code
   *  into the location pointed to by \c code and copy the message into the formatted
   *  string pointed to \c buf limited it to the first \c len characters. Providing
   *  a 0 \c buf value is permitted.
   *
   *  A log message has two attributes; a dsv_log_code that discribes the type of
   *  log message it is as well as a list of parameters that make up that particular
   *  message. The number of parameters is determined by the dsv_log_code.
   *
   *  An instance of a log message is tagged with a log level that can be filtered
   *  for easy retrieval.
   *
   *  \note The nth log message is determined by the application of the given filter.
   *  For example, if there are 10 total log_messages but only 4 tagged as
   *  \c dsv_log_warning, the 3rd message (\c num=3) will be the 3rd message tagged as
   *  \c dsv_log_warning not necessarily the 3rd message in the sequence of 10.
   *
   *  The provided string pointed to by \c buf is a format string to which the parameters
   *  defined by the log_code are replaced based on the placeholders it contains. The
   *  placeholders are identified by "$n" where \c n is the parameter number. For
   *  example, if the format string is: "param 1 is $2 and param 2 is $1" and the
   *  the two paramers associated with the log message are "foo" and "bar", \c buf
   *  will then contain "param 1 is bar and param 2 is foo". Each $n may appear anywhere
   *  within the format string. If there are more placeholders than parameters, they
   *  will be ignored. There is no requirement to have a placeholder for each param.
   *  If \c buf is 0, the return value is the number of characters needed to hold all
   *  parameter strings concatenated together.
   *
   *  To avoid memory ownership and potential buffer overflow issues while minimizing
   *  the number of function calls to obtain the needed information, the recommended
   *  pattern for use of \c dsv_parse_log is:
   *
   *      // Get the number of log messages for \c log_level
   *      size_t num_msgs = dsv_parse_log_count(aParser,aLevel);
   *
   *      // assume want msg number \c num < \c num_msgs
   *      // This message contains 2 parameters "foo" and "bar" determined by the
   *      // error code
   *
   *      // Get the code and minimum storage requirement for the msg parameters
   *      dsv_log_code msg_code;
   *      ssize_t len = dsv_parse_log(aParser,aLevel,num,&msg_code,0,0);
   *
   *      // msg_code now holds the code associated with the msg
   *      // len now holds the storage needed for the the total number of parameters
   *      // associated with msg_code.
   *
   *      const char *fmt_str;
   *      switch(msg_code) {  // or equivalent
   *        case ...:
   *          fmt_str = "param 1 is $2 and param 2 is $1";
   *          break;
   *
   *        ...
   *      }
   *
   *      size_t fmt_len = strlen(fmt_str);
   *
   *      // Allocate buf based on minimum needed to store the parameters and the
   *      // length needed to store the format string potentially trimmed of the
   *      // placeholders. Do not forget to add space for the null terminator
   *      size_t storage_len = sizeof(char)*(fmt_len+len+1);
   *      char *buf = (char*)malloc(storage_len);
   *
   *      // copy the formated messages to buf
   *      len = dsv_parse_log(aParser,aLevel,num,&msg_code,buff,storage_len);
   *
   *  \param[in] parser A \c dsv_parser_t object previously initialized with
   *                    \c dsv_parser_create
   *  \param[in] log_level A valid value of \c dsv_log_level indicating how the
   *                        the list of all log messages should be filtered.
   *  \param[in] num The zero-indexed nth log_message where n is less than the value
   *                 returned by dsv_parse_log_count for the given \c log_level.
   *  \param[in,out] code A pointer to a dsv_log_code. The error code associated with
   *                        the error message will be stored in this location.
   *  \param[in,out] buf A pointer to a null-terminated character buffer appropriately
   *                      sized. If \c buf is nonzero, the first \c len characters of
   *                      messages are copied to buf including the terminating null
   *                      character.
   *  \param[in] len The size of the character buffer pointed to by \c buf.
   *  \retval <0 An error has occurred. errno is set.
   *                      EINVAL Either num is greater than the number of messages
   *                             filtered by \c log_level OR \c code is zero
   *                      ENOMEM Out of memory
   *  \retval 0 There are no parameters associated with the log_message with the
   *              stated error code. This log message is nonetheless valid.
   *  \retval nonnegative The size of the message buffer including the null terminator.
   *                      If \c buf is zero, this value is the is the number of characters
   *                      needed to hold all parameter strings concatenated together
   *                      and is appropriate for allocating the character buffer \c buf
   *                      in subsequent calls. If \c buf is non-zero, the return value
   *                      contains the length of the fully formatted string contained in
   *                      \c buf including the null terminator.
   */
  ssize_t dsv_parse_log(dsv_parser_t parser, dsv_log_level log_level, size_t num,
    dsv_log_code *code, char *buf, size_t len);



#if defined(__cplusplus)
}
#endif

#endif
