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
  #include <vector>

  // Change me with bison version > 3
  struct YYSTYPE {
    typedef std::shared_ptr<std::string> str_ptr_type;
    typedef std::vector<str_ptr_type> str_vec_type;
    typedef std::shared_ptr<str_vec_type> str_vec_ptr_type;

    str_ptr_type str_ptr;
    str_vec_ptr_type vec_ptr;
  };

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
  namespace detail {
    /**
     *  convenience declares
     */
     
    void process_header(const YYSTYPE::str_vec_ptr_type &str_vec_ptr,
      const detail::parse_operations &operations)
    {
      std::cerr << "CALLING PROCESS_HEADER\n";
      if(operations.header_callback) {
        operations.header_field_storage.clear();
        operations.header_field_storage.reserve(str_vec_ptr->size());
        for(size_t i=0; i<str_vec_ptr->size(); ++i)
          operations.header_field_storage.push_back((*str_vec_ptr)[i]->c_str());
        
        operations.header_callback(&*(operations.header_field_storage.begin()),
          operations.header_field_storage.size(),operations.header_context);
      }
    }
  }

}

%define api.pure full

%debug
%error-verbose

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
%token D2QUOTE
%token <str_ptr> TEXTDATA

// file
%type <vec_ptr> header
%type <vec_ptr> name_list
%type <str_ptr> name
%type <str_ptr> field
%type <str_ptr> escaped_field;
%type <str_ptr> escaped_textdata_list
%type <str_ptr> escaped_textdata
%type <str_ptr> non_escaped_field;
// record_list
// record
// field_list



%% /* The grammar follows.  */

file:
    /* empty */
  | header CR LF { detail::process_header($1,operations); }
  | header CR LF record_list {std::cerr << "HEADER with UNTERMINATED RECORD LIST\n";}
  | header CR LF record_list CR LF {std::cerr << "HEADER with TERMINATED RECORD LIST\n";}
  ;

header:
    name_list { $$ = $1; }
  ;

name_list:
    name {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->push_back($1);
    }
  | name_list DELIMITER name {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->reserve($1->size()+1);
      $$->assign($1->begin(),$1->end());
      $$->push_back($3);
    }
  ;

name:
  field { $$ = $1; }
  ;

field:
    escaped_field { $$ = $1; }
  | non_escaped_field { $$ = $1; }
  ;

escaped_field:
    DQUOTE escaped_textdata_list DQUOTE { $$ = $2; }
  ;

escaped_textdata_list:  // need to aggregate so must use unique string
    escaped_textdata { $$ = $1; }
  | escaped_textdata_list escaped_textdata {
      $$ = std::shared_ptr<std::string>(new std::string());
      $$->reserve($1->size()+$2->size());
      $$->assign($1->begin(),$1->end());
      $$->append($2->begin(),$2->end());
    }
  ;

escaped_textdata:
    TEXTDATA { $$ = $1; }
  | DELIMITER {
      // delimiter must be recreated as it could change across parser invocations
      // todo, still can be cached in the parser...
      std::shared_ptr<std::string> tmp(new std::string());
      tmp->push_back(parser.delimiter());
      $$ = tmp;
    }
  | LF { $$ = scanner.linefeed_str(); }
  | CR { $$ = scanner.carriage_return_str(); }
  | D2QUOTE { $$ = scanner.quote_str(); }
  ;

non_escaped_field:
    TEXTDATA { $$ = $1; }
  ;

record_list:
    record
  | record_list CR LF record
  ;

record:
    field_list
  ;

field_list:
    field
  | field_list DELIMITER field

%%









/**
    There are only a few tokens to be lexicographically generated. Many are setting and
    contextually dependent. The only non-single character tokens are field data content
    and the double quote (ie "");

    Only TEXTDATA strings are returned in YYSTYPE
 */
int parser_lex(YYSTYPE *lvalp, detail::scanner_state &scanner,
 const detail::parser &parser)
{
  // <32 is non-printing
  // > 126 is non-printing
  // ' is 0x27
  // " is 0x22

  // cur holds the current value
  // next holds the lookahead
  unsigned char cur;
  while(scanner.getc(cur) && scanner.advance()) {
    //std::cerr << "Scanned '" << static_cast<unsigned int>(cur) << "'\n";;

    unsigned char next;
    scanner.getc(next);

    if(cur == parser.delimiter())
      return DELIMITER;
    else if(cur == 0x0A) //LF
      return LF;
    else if(cur == 0x0D) //CR
      return CR;
    else if(cur == 0x22) { //"
      if(next == 0x22) {
        scanner.advance();
        return D2QUOTE;
      }
      return DQUOTE;
    }
    else {
      std::shared_ptr<std::string> &str_ptr = lvalp->str_ptr;

      if(!str_ptr)
        str_ptr = std::shared_ptr<std::string>(new std::string());

      str_ptr->assign(&cur, &cur+1);

      for(; scanner.getc(cur); scanner.advance()) {
        if(cur == parser.delimiter() ||
          cur == 0x0A || //LF
          cur == 0x0D || //CR
          cur == 0x22) // DQUOTE
        {
          return TEXTDATA;
        }

      str_ptr->push_back(cur);
      }
    }
  }

  return 0;
}
