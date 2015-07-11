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
   *  The default value is \c dsv_newline_permissive
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
   *  \brief Set the required number of fields for future parsing with \c parser or
   *  allow a non-uniform number.
   *
   *  If the behavior specified by \c dsv_parser_fixed_field_columns is violated, 
   *  dsv_parse will immediately return a nonzero value and an error message will be
   *  logged with the code: \c dsv_column_count_error.
   *
   *  The default value is 0. This value is also appropriate for RFC4180-strict processing
   *
   *  \param parser A properly initialized dsv_parser_t object
   *  \param num_cols If > 0, the number of columns expected during future parsing. If
   *  during parsing, a row with less than \c num_cols is encountered, dsv_parse will
   *  immediately return with a nonzero value. If \c num_cols == 0, the parser will set
   *  the required number of columns based on the first row encountered. For example, if
   *  the first header row contains 5 columns, all subsequent rows must contain 5 columns
   *  otherwise the dsv_parse will immediately return a nonzero value. If 
   *  \c num_cols == -1, no restriction will be placed on the number of columns. This
   *  also means that rows with zero columns are acceptable. In this case, any registered
   *  callback will still be called.
   */
  void dsv_parser_set_field_columns(dsv_parser_t parser, ssize_t num_cols);

  /**
   *  \brief Get the required number of fields associated with future parsing with 
   *  \c parser
   *
   *  See \c dsv_parser_set_field_columns for an explanation of the return values
   *
   *  \retval num_cols The number of columns required for future parsing of \c parser
   */
  ssize_t dsv_parser_get_field_columns(dsv_parser_t parser);



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
   *          and control should return from the parse function. If a nonzero value
   *          is returned, the parse function will also return <0
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
   *  \brief Logging levels for parser messages
   */
  typedef enum {
    dsv_log_none = 0, /*!< Filter all messages */
    dsv_log_info = 1L, /*!< Filter all messages except info messages */
    dsv_log_warning = 1L << 1, /*!< Filter all messages except warning messages */
    dsv_log_error = 1L << 2, /*!< Filter all messages except error messages */
    dsv_log_all = (dsv_log_error|dsv_log_warning|dsv_log_info) /*!< Do not filter messages */
  } dsv_log_level;

  /**
   *  \brief Logging message codes
   */
  typedef enum {
    /*
      \brief An informational message indicating that the number of columns for each row
        of the records are not consistant. This message is also generated if the number
        of columns in the header and the number of columns in the records are not the
        same.

      This message is only applicable if the \c num_cols parameter of 
      \c dsv_parser_set_field_columns is set to -1 as all other values for this
      parameter will generate an error instead.
      
      This log code has four parameters;
        - The line that triggered the column inconsistency[*][**]
        - The number of fields that would allow the record columns to remain consistant[*]
        - The number of fields parsed for this row that triggered the inconsistency[*]
        - The location_str associated with the syntax error if it was supplied to
          \c dsv_parse

      * Numbers provided as a string are capable of being translated to
        a signed or unsigned integer value (ie strtoul). 
      
      **The line associated with the log is counted according to the applied parser
        behavior. For example, if the system newline is LF and the applied behavior is
        RFC4180-strict, then the lines will be counted based on the occurrence of CRLF.
        If the behavior is newline_permissive, the lines will be counted based on the
        first parsed occurrence of a newline. Note that this is different than the first
        seen occurence of a newline. For example, if a record is "..LF.."CRLF, the LF is
        parsed as a quoted field and not a newline therefore the registered newline
        behavior will be the CRLF.
    */
    dsv_nonrectangular_records_info,
    
    /*
      \brief An error strictly associated with incorrect syntax based on the current
      parser behavior.
      
      This log code has five parameters;
        - The offending line associated with the start of the syntax error[*][**]
        - The offending line associated with the end of the syntax error[*][**]
        - The offending character associated with the start of the syntax error[*]
        - The offending character associated with the end of the syntax error[*]
        - The location_str associated with the syntax error if it was supplied to
          \c dsv_parse

      * Numbers provided as a string are capable of being translated to
        a signed or unsigned integer value (ie strtoul). 
      
      **The line associated with the log is counted according to the applied parser
        behavior. For example, if the system newline is LF and the applied behavior is
        RFC4180-strict, then the lines will be counted based on the occurrence of CRLF.
        If the behavior is newline_permissive, the lines will be counted based on the
        first parsed occurrence of a newline. Note that this is different than the first
        seen occurence of a newline. For example, if a record is "..LF.."CRLF, the LF is
        parsed as a quoted field and not a newline therefore the registered newline
        behavior will be the CRLF.
    */
    dsv_syntax_error,
    
    /*
      \brief An error strictly associated with parsing a non-uniform number of fields
      when explicitly requested to do so. For example, if the header contains 5 fields
      but the first record only contains 3.
      
      This log code has 
        - The line number associated with the start of the offending row[*][**]
        - The line number associated with the end of the offending row[*][**]
        - The expected number of fields[*]
        - The number of fields parsed for this row[*]
        - The location_str associated with the syntax error if it was supplied to
          \c dsv_parse

      * Numbers provided as a string are capable of being translated to
        a signed or unsigned integer value (ie strtoul). 
      
      **The line associated with the log is counted according to the applied parser
        behavior. For example, if the system newline is LF and the applied behavior is
        RFC4180-strict, then the lines will be counted based on the occurrence of CRLF.
        If the behavior is newline_permissive, the lines will be counted based on the
        first parsed occurrence of a newline. Note that this is different than the first
        seen occurence of a newline. For example, if a record is "..LF.."CRLF, the LF is
        parsed as a quoted field and not a newline therefore the registered newline
        behavior will be the CRLF.
    */
    dsv_column_count_error
  } dsv_log_code;


  /**
   *  \brief This function will be called each time a message is logged by the parser
   *    according to the set logging level.
   *  
   *  \param[in] code The \c dsv_log_code associated with this message
   *  \param[in] level \parblock
   *  The \c dsv_log_level that this code was generated against. This value may be 
   *  different depending on the parser settings for a given \c dsv_log_code. For example,  
   *  under settings that define strict behavior, the message may be considered and error. 
   *  Under permissive settings the same message may just be a warning.
   *  \endparblock
   *  \param[in] params \parblock
   *  A c-array of null-terminated byte strings containing the parameters associated with 
   *  the given \c dsv_log_code (see \c dsv_log_code for an explanation of the different 
   *  parameters for each \c dsv_log_code).
   *  \endparblock
   *  \param[in] size The size of the params array
   *  \param[in] context A user-defined value associated with this callback set in
   *                      \c dsv_set_log_callback
   *
   *  \retval nonzero if procssing should continue or 0 if processing should cease
   *          and control should return from the parse function. If a nonzero value
   *          is returned, the parse function will also return <0. Use this return value 
   *          if an outside mechanism determines that parsing should cease based on the 
   *          log message. NB that the return value is ignored in the case of error
   *          messages as they will always cause processing to cease and a nonzero value
   *          to be returned from from the parse function.
   */
  typedef int (*log_callback_t)(dsv_log_code code, dsv_log_level level,
    const char *params[], size_t size, void *context);
  
  /**
   *  \brief Obtain the callback currently set for log messages
   *
   *  \retval 0 No callback is registered
   *  \retval nonzero The currently registered callback
   */
  log_callback_t dsv_get_logger_callback(dsv_parser_t parser);

  /**
   *  \brief Obtain the user-defined context currently set for header
   *
   *  \retval 0 No context is registered
   *  \retval nonzero The currently registered context
   */
  void * dsv_get_logger_context(dsv_parser_t parser);

  /**
   *  \brief Associate the logging callback \c fn and a user-specified value \c context
   *  with \c parser.
   *
   *  \note The value of \c context is passed in as the \c context parameter in \c fn
   */
  void dsv_set_logger_callback(log_callback_t fn, void *context, dsv_parser_t parser);



#if defined(__cplusplus)
}
#endif

#endif
