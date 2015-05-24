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

%code requires {
  /*
    Forward declarations for forward declarations of bison types
  */
  #include "parser.h"
  #include "scanner_state.h"
  #include "parse_operations.h"

  #include <memory>
  #include <string>

  // Change me with bison version > 3
  //%define api.value.type {std::string}
  #define YYSTYPE std::string
}


%code {
  #include "dsv_grammar.hh"

  /**
   *  Error reporting function as required by yacc
   */
  void parser_error(const detail::parser_state &scanner, const detail::parser &parser,
    const detail::parse_operations &operations,
    const std::unique_ptr<detail::parser_state> &context, const char *s)
  {
    std::cerr << "HERE!!!!!!!!'" << s << "'\n";
  }

int parser_lex(YYSTYPE *lvalp, const detail::parser_state &scanner,
  const detail::parser &parser);

//   void dsv_parser_error(yyscan_t scanner, detail::dsv_parser &parser,
//     const detail::parse_operations &operations, const char *s)
//   {
//     std::cerr << "HERE!!!!!!!!'" << s << "'\n";
//   }

  /**
   *  Use namespaces here to avoid multiple symbol name clashes
   */
  namespace dsv {
    /**
     *  convenience routines
     */
//    inline static dsv::parser_extra & get_state(yyscan_t scanner) {
//      return *static_cast<parser_extra*>(dsv_parser_get_extra(scanner));
//    }

  }

}

%define api.pure full



%lex-param {detail::parser_state &scanner}
%lex-param {const detail::parser &parser}

%parse-param {detail::parser_state &scanner}
%parse-param {const detail::parser &parser}
%parse-param {const detail::parse_operations &operations}
%parse-param {const std::unique_ptr<detail::parser_state> &context}

%token CR
%token LF




%% /* The grammar follows.  */

input:
  /* empty */
  | input record
  ;


record:
  newline
  ;

newline
  : LF
    {
//       if(parser.newline_behavior() == dsv_newline_crlf_strict) {
//         parser.push_msg("LFSeenOnCRLFStrict",dsv_log_error);
//         YYABORT;
//       }
    }
  | CR LF
    {
//       if(parser.newline_behavior() == dsv_newline_lf_strict) {
//         parser.push_msg("CRSeenOnLFStrict",dsv_log_error);
//         YYABORT;
//       }
    }

%%

int parser_lex(YYSTYPE *lvalp, detail::parser_state &scanner,
  const detail::parser &parser)
{
  std::string &value = *lvalp;

  detail::parser_state::iterator cur = scanner.begin();

  while(cur != scanner.end()) {
    if(scanner.seek_state() == detail::parser_state::initial) {
      switch(*cur) {
        case 0x0D: { // CR
          if(/*dsv_newline_crlf_strict*/true) {
            ++cur;
            if(*cur == 0x0A) { // LF
              // return NL but need to return the field value. ??
            }
            else {
              error.
            }
          }
          else if(/*dsv_newline_lf_strict*/true) {
            // just push. valid character? or error?
          }
        }
          break;

        default:
          value.push_back(*cur);
          break;
      };
    }
    else {
      abort();
    }

    ++cur;
  }

  return 0;
}
