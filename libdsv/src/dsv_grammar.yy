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
  #include <iostream>

  /**
   *  Error reporting function as required by yacc
   */
  void parser_error(const detail::scanner_state &scanner, const detail::parser &parser,
    const detail::parse_operations &operations,
    const std::unique_ptr<detail::scanner_state> &context, const char *s)
  {
    std::cerr << "HERE!!!!!!!!'" << s << "'\n";
  }

   int parser_lex(YYSTYPE *lvalp, detail::scanner_state &scanner,
     const detail::parser &parser);

  /**
   *  Use namespaces here to avoid multiple symbol name clashes
   */
  namespace dsv {
    /**
     *  convenience routines
     */

  }

}

%define api.pure full



%lex-param {detail::scanner_state &scanner}
%lex-param {const detail::parser &parser}

%parse-param {detail::scanner_state &scanner}
%parse-param {const detail::parser &parser}
%parse-param {const detail::parse_operations &operations}
%parse-param {const std::unique_ptr<detail::scanner_state> &context}

%token DELIMITER;
%token LF
%token CR
%token DQUOTE
%token TEXTDATA


%% /* The grammar follows.  */

file:
    /* empty */
  | header
  ;

header:
  name_list
  | name_list CR LF
  ;

name_list:
    name
  | name_list DELIMITER name
  ;

name:
  'a'
  ;

%%


#if 0
int parser_lex(YYSTYPE *lvalp, detail::scanner_state &scanner,
 const detail::parser &parser)
{
  std::string &value = *lvalp;

  // <32 is non-printing
  // > 126 is non-printing
  // ' is 0x27
  // " is 0x22

  // cur holds the current value
  // next holds the lookahead
  unsigned char cur;
  while(scanner.getc(cur) && scanner.advance()) {
    if(scanner.seek_state() == detail::scanner_state::start) {
      value.clear();

      // need to make decisions based on the current value rather than the lookahead
      if(cur == parser.delimiter()) {
        return DELIMITER;
      }
      else if(cur == 0x0D) { // CR
          if(parser.newline_behavior() == dsv_newline_permissive)
            scanner.seek_state(detail::scanner_state::nl_permissive);
          else if(parser.newline_behavior() == dsv_newline_crlf_strict)
            scanner.seek_state(detail::scanner_state::carrage_return);
          else {
            // error not sure what to do here, should the CR be an error or get added
            // as a token?
            //scanner.seek_state(detail::scanner_state::parse_error);
            abort();
          }
      }
      else if(cur == 0x0A) { //LF
        if(parser.newline_behavior() == dsv_newline_permissive ||
          parser.newline_behavior() == dsv_newline_lf_strict)
        {
          scanner.seek_state(detail::scanner_state::linefeed);
        }
        else {
            // error not sure what to do here, should the NL be an error or get added
            // as a token if expecting CRLF?
            //scanner.seek_state(detail::scanner_state::parse_error);
            abort();
        }
      }
      else if(cur == 0x22) { // double quote
        scanner.seek_state(detail::scanner_state::field_double_quote);
      }
      else if(cur == 0x27) { // single quote
        scanner.seek_state(detail::scanner_state::field_single_quote);
      }
      else if(parser.reject_nonprinting() && (cur < 32 || cur > 126)) {
        scanner.seek_state(detail::scanner_state::parse_error);
      }
      else {
        scanner.seek_state(detail::scanner_state::field);
      }
    }

    unsigned char next=0;
    bool is_last = scanner.getc(next)

    // make decisions based on lookahead
    if(scanner.seek_state() == detail::scanner_state::field) {
      value.push_back(cur);

      if(is_last || next == 0x0D || next == 0x0A || next == parser.delimiter()) { // CR or LF
        scanner.seek_state(detail::scanner_state::start);
        return FIELD;
      }
    }
    else if(scanner.seek_state() == detail::scanner_state::carrage_return) {
      scanner.seek_state(detail::scanner_state::start);
      return NEWLINE;
    }
    else if(scanner.seek_state() == detail::scanner_state::linefeed) {
      scanner.seek_state(detail::scanner_state::start);
      return NEWLINE;
    }

    else if(scanner.seek_state() == detail::scanner_state::nl_permissive) {
      if(!is_last && next == 0x0A) { //LF
        scanner.seek_state(detail::scanner_state::linefeed);
      }
      else {
        return NEWLINE;
      }
    }
    else {
      abort();
    }
  }

  return 0;
}



/**
    There are only a few tokens to be lexicographically generated. Many are setting and
    contextually dependent. The only non-single character token is field data content.
 */
int parser_lex(YYSTYPE *lvalp, detail::scanner_state &scanner,
 const detail::parser &parser)
{
  std::string &value = *lvalp;

  // <32 is non-printing
  // > 126 is non-printing
  // ' is 0x27
  // " is 0x22

  // cur holds the current value
  // next holds the lookahead
  unsigned char cur;
  while(scanner.getc(cur) && scanner.advance()) {
    unsigned char next=0;
    bool final_char = scanner.getc(next)

    if(scanner.seek_state() == detail::scanner_state::start) {
      value.clear();

      if(cur == parser.delimiter()) {
        return DELIMITER;
      }
      else if(cur == 0x0D && parser.newline_behavior() != dsv_newline_lf_strict) { // CR
        if(parser.newline_behavior() == dsv_newline_permissive) {
          if(!final_char && next == 0x0A) // LF
            scanner.seek_state(detail::scanner_state::linefeed)
          else {
            scanner.seek_state(detail::scanner_state::start);
            return NEWLINE;
          }
        }
        else if(parser.newline_behavior() == dsv_newline_crlf_strict) {
          scanner.seek_state(detail::scanner_state::linefeed)
        }
      }
      else if(cur == 0x0A) { //LF
        if(parser.newline_behavior() != dsv_newline_crlf_strict) {
          scanner.seek_state(detail::scanner_state::start);
          return NEWLINE;
        }
        else {
            // error not sure what to do here, should the NL be an error or get added
            // as a token if expecting CRLF?
            //scanner.seek_state(detail::scanner_state::parse_error);
            abort();
        }
      }
      else if(cur == 0x22) { // double quote
        scanner.seek_state(detail::scanner_state::field_double_quote);
      }
      else if(cur == 0x27) { // single quote
        scanner.seek_state(detail::scanner_state::field_single_quote);
      }
      else if(parser.reject_nonprinting() && (cur < 32 || cur > 126)) {
        scanner.seek_state(detail::scanner_state::parse_error);
      }
      else {
        scanner.seek_state(detail::scanner_state::field);
      }
    }


    // make decisions based on lookahead
    if(scanner.seek_state() == detail::scanner_state::field) {
      value.push_back(cur);

      if(is_last || next == 0x0D || next == 0x0A || next == parser.delimiter()) { // CR or LF
        scanner.seek_state(detail::scanner_state::start);
        return FIELD;
      }
    }
    else if(scanner.seek_state() == detail::scanner_state::carrage_return) {
      scanner.seek_state(detail::scanner_state::start);
      return NEWLINE;
    }
    else if(scanner.seek_state() == detail::scanner_state::linefeed) {
      scanner.seek_state(detail::scanner_state::start);
      return NEWLINE;
    }

    else if(scanner.seek_state() == detail::scanner_state::nl_permissive) {
      if(!is_last && next == 0x0A) { //LF
        scanner.seek_state(detail::scanner_state::linefeed);
      }
      else {
        return NEWLINE;
      }
    }
    else {
      abort();
    }
  }

  return 0;
}


#endif








/**
    There are only a few tokens to be lexicographically generated. Many are setting and
    contextually dependent. The only non-single character token is field data content.
 */
int parser_lex(YYSTYPE *lvalp, detail::scanner_state &scanner,
 const detail::parser &parser)
{
  std::string &value = *lvalp;
  value.clear();

  // <32 is non-printing
  // > 126 is non-printing
  // ' is 0x27
  // " is 0x22

  // cur holds the current value
  // next holds the lookahead
  unsigned char cur;
  while(scanner.getc(cur) && scanner.advance()) {
    if(cur == parser.delimiter())
      return DELIMITER;
    else if(cur == 0x0A) //LF
      return LF;
    else if(cur == 0x0D) //CR
      return CR;
    else if(cur == 0x22) //"
      return DQUOTE;
    else {
      value.push_back(cur);
      for(; scanner.getc(cur); scanner.advance()) {
        if(cur == parser.delimiter() ||
          cur == 0x0A || //LF
          cur == 0x0D || //CR
          cur == 0x22) // DQUOTE
        {
          return TEXTDATA;
        }

        value.push_back(cur);
      }
    }
  }

  return 0;
}
