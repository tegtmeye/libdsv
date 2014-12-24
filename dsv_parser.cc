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

typedef void* yyscan_t;
#include "dsv_grammar.hh"
#include "dsv_rules.h"

#include "dsv_parser.h"
#include "parser_state.h"

#include <errno.h>
#include <stdlib.h>




namespace detail {

//  struct lex_sentry {
//    lex_sentry(yyscan_t s) :scanner(s) {}
//    ~lex_sentry(void) {
//      dsv_parser_lex_destroy(scanner);
//    }
//
//    yyscan_t scanner;
//  };

}


extern "C" {

  int obj_parser_create(dsv_parser_t *parser)
  {
    int err = 0;

    try {
      parser->p = new detail::dsv_parser;
    }
    catch (std::bad_alloc &) {
      err = ENOMEM;
    }
    catch (...) {
      abort();
    }

    return err;
  }

  void dsv_parser_destroy(dsv_parser_t parser)
  {
    try {
      delete static_cast<detail::dsv_parser*>(parser.p);
      parser.p = 0;
    }
    catch(...) {
      abort();
    }
  }

  int dsv_parse(const char *filename, dsv_parser_t parser,
                dsv_contents_t *contents)
  {
    detail::dsv_parser *p = static_cast<detail::dsv_parser*>(parser.p);

    int err = 0;

    try {

      // make sure the contents get destroyed if there is an exception
//      std::auto_ptr<detail::dsv_contents> sentry(new detail::dsv_contents);

      err = p->parse_file(filename);

    }
    catch(std::bad_alloc &) {
      err = ENOMEM;
    }
    catch(std::exception &ex) {
//      p->last_error = ex.what();
      err = -1;
    }
    catch(...) {
      abort();
    }
    
    return err;
  }


}
