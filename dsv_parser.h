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


#if defined(__cplusplus)
extern "C" {
#endif

  /**
   *  A structure containing a dsv parser
   */
  typedef struct {
    void *p;
  } dsv_parser_t;

  /**
   *  A structure for extra things. Replace with something usefull
   */
  typedef struct {
    void *p;
  } dsv_contents_t;



  /**
   *  Allocate a dsv_parser_t object.
   *
   *  You must eventually call dsv_parser_destroy.
   *
   *  Return values:
   *  0 - success
   *  ENOMEM - out of memory
   */
  int dsv_parser_create(dsv_parser_t *parser);

  /**
   *  Destroy the obj_parser_t object.
   */
  void dsv_parser_destroy(dsv_parser_t parser);

  /**
   *  Parse the file 'filename' with 'parser', create and place results into
   *  'contents'.
   *
   *  If the filename begins with the '/' character, then the file is
   *  understood to be an absolute path starting at the root directory. If the
   *  filename does not begin with a '/' character, the file is understood to
   *  be relative to the current working directory. In either case any additional
   *  relative file inclusion mechanism will be relative to the directory that
   *  holds the input file.
   *
   *  After a successful call to dsv_parse, you must eventually
   *  call dsv_contents_destroy.
   *
   *  If unable to successfully parse 'filename' a negative value will be
   *  returned and the reason can be obtained by dsv_parse_error.
   *
   *  Warnings or information about non-implemented features will return success
   *  and be reported via dsv_parse_error.
   *
   *  Return values:
   *  0 - success
   *  ENOMEM - out of memory
   *  Any error code returned by fopen
   *  <0 - failure, see dsv_parse_error
   */
  int dsv_parse(const char *filename, dsv_parser_t parser,
                dsv_contents_t *contents);


#if defined(__cplusplus)
}
#endif

#endif
