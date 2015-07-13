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
   *  Error reporting function as required by Bison
   */
  void parser_error(YYLTYPE *llocp, const detail::scanner_state &scanner,
    detail::parser &parser, const detail::parse_operations &operations,
    const std::unique_ptr<detail::scanner_state> &context, const char *s)
  {
    log_callback_t logger = parser.log_callback();
    if(logger) {
      std::string first_line = std::to_string(llocp->first_line);
      std::string last_line = std::to_string(llocp->last_line);
      std::string first_column = std::to_string(llocp->first_column);
      std::string last_column = std::to_string(llocp->last_column);
      std::string filename = scanner.filename();
    
      const char *fields[] = {
        first_line.c_str(),
        last_line.c_str(),
        first_column.c_str(),
        last_column.c_str(),
        filename.c_str()
      };
      
      logger(dsv_syntax_error,dsv_log_error,fields,sizeof(fields)/sizeof(const char *),
        parser.log_context());
    }
  }

  void column_count_error(const YYLTYPE &llocp, const detail::scanner_state &scanner,
    detail::parser &parser, std::size_t rec_cols)
  {
    // - The line number associated with the start of the offending row[*][**]
    // - The line number associated with the end of the offending row[*][**]
    // - The expected number of fields[*]
    // - The number of fields parsed for this row[*]
    // - The location_str associated with the syntax error if it was supplied to
    //   \c dsv_parse
    log_callback_t logger = parser.log_callback();
    if(logger) {
      std::string first_line = std::to_string(llocp.first_line);
      std::string last_line = std::to_string(llocp.last_line);
      std::string exp_columns = std::to_string(parser.effective_field_columns());
      std::string rec_columns = std::to_string(rec_cols);
      std::string filename = scanner.filename();

      const char *fields[] = {
        first_line.c_str(),
        last_line.c_str(),
        exp_columns.c_str(),
        rec_columns.c_str(),
        filename.c_str()
      };
      
      logger(dsv_column_count_error,dsv_log_error,fields,
        sizeof(fields)/sizeof(const char *),parser.log_context());
    }    
  }

  bool nonrectangular_column_info(const YYLTYPE &llocp, const detail::scanner_state &scanner,
    detail::parser &parser, std::size_t rec_cols)
  {
    // - The line that triggered the column inconsistency[*][**]
    // - The number of fields that would allow the record columns to remain consistant[*]
    // - The number of fields parsed for this row that triggered the inconsistency[*]
    // - The location_str associated with the syntax error if it was supplied to
    //   \c dsv_parse
    log_callback_t logger = parser.log_callback();
    if(logger) {
      std::string last_line = std::to_string(llocp.last_line);
      std::string exp_columns = std::to_string(parser.effective_field_columns());
      std::string rec_columns = std::to_string(rec_cols);
      std::string filename = scanner.filename();

      const char *fields[] = {
        last_line.c_str(),
        exp_columns.c_str(),
        rec_columns.c_str(),
        filename.c_str()
      };
      
      return logger(dsv_nonrectangular_records_info,dsv_log_info,fields,
        sizeof(fields)/sizeof(const char *),parser.log_context());
    }
    
    return true;
  }


   int parser_lex(YYSTYPE *lvalp, YYLTYPE *llocp, detail::scanner_state &scanner,
     detail::parser &parser);

  /**
   *  Use namespaces here to avoid multiple symbol name clashes
   */
  namespace detail {
    /**
     *  convenience declares
     */
    bool check_or_update_column_count(const YYLTYPE &llocp, 
      const detail::scanner_state &scanner, detail::parser &parser, 
      const YYSTYPE::str_vec_ptr_type &str_vec_ptr)
    {
      ssize_t columns = str_vec_ptr->size();
      
      if(!parser.effective_field_columns_set()) {
//         std::cerr << "effective_field_columns_set NOT set\n";
        parser.effective_field_columns_set(true);
        parser.effective_field_columns(columns);
      }
      else if(parser.effective_field_columns() != columns) {
//         std::cerr << "effective_field_columns_set set && cols not equal\n";
        if(parser.effective_field_columns() < 0) {
          return nonrectangular_column_info(llocp,scanner,parser,columns);
        }
        else {
          column_count_error(llocp,scanner,parser,columns);
          return false;
        }
      }

      return true;
    }
    
    
    bool process_header(const YYSTYPE::str_vec_ptr_type &str_vec_ptr,
      const detail::parse_operations &operations)
    {
//         std::cerr << "CALLING PROCESS_HEADER\n";
      bool keep_going = true;
      if(operations.header_callback) {
//          std::cerr << "got size " << str_vec_ptr->size() << "\nGot:\n";
//          for(int i=0; i<str_vec_ptr->size(); ++i)
//            std::cerr << "\t" << (*str_vec_ptr)[i] << "\n";
          
        operations.field_storage.clear();
        operations.field_storage.reserve(str_vec_ptr->size());
        for(size_t i=0; i<str_vec_ptr->size(); ++i)
          operations.field_storage.push_back((*str_vec_ptr)[i]->c_str());
        
//        std::cerr << "CALLING REGISTERED CALLBACK\n";
        keep_going = operations.header_callback(operations.field_storage.data(),
          operations.field_storage.size(),operations.header_context);
      }
      return keep_going;
    }

    bool process_record(const YYSTYPE::str_vec_ptr_type &str_vec_ptr,
      const detail::parse_operations &operations)
    {
//        std::cerr << "CALLING PROCESS_RECORD\n";
      bool keep_going = true;
      if(operations.record_callback) {
        operations.field_storage.clear();
        operations.field_storage.reserve(str_vec_ptr->size());
        for(size_t i=0; i<str_vec_ptr->size(); ++i)
          operations.field_storage.push_back((*str_vec_ptr)[i]->c_str());
        
        keep_going = operations.record_callback(operations.field_storage.data(),
          operations.field_storage.size(),operations.record_context);
      }
      return keep_going;
    }

    static const std::shared_ptr<std::string> empty_str(new std::string(""));
    static const YYSTYPE::str_vec_ptr_type empty_vec(new YYSTYPE::str_vec_type());
  }

}

%define api.pure full
%locations

%debug
%error-verbose

%lex-param {detail::scanner_state &scanner}
%lex-param {const detail::parser &parser}

%parse-param {detail::scanner_state &scanner}
%parse-param {detail::parser &parser}
%parse-param {const detail::parse_operations &operations}
%parse-param {const std::unique_ptr<detail::scanner_state> &context}

%token END 0 "end-of-file"
%token DELIMITER "delimiter" 
//%token HEADER_DELIMITER "header delimiter"
%token <str_ptr> LF "linefeed"
%token <str_ptr> CR "carriage-return"
%token <str_ptr> NL "newline"
%token DQUOTE "\""
%token <str_ptr> D2QUOTE "\"\""
%token <str_ptr> TEXTDATA


// file
// %type <vec_ptr> header
%type <vec_ptr> field_list
%type <str_ptr> field
%type <str_ptr> escaped_field;
%type <str_ptr> escaped_textdata_list
%type <str_ptr> escaped_textdata
%type <str_ptr> non_escaped_field;
// record_list
// record



%% /* The grammar follows.  */

file:
    /* empty */
  | empty_header
  | empty_header record_block
  | header_block
  | header_block NL
  | header_block NL record_block 
  ;

empty_header:
    NL {
      // NL means no header. Check to see if empty records are allowed
//       std::cerr << "HERE!!!!!!!!!!!!!!\n";
      if(!detail::check_or_update_column_count(@1,scanner,parser,detail::empty_vec)) {
//         std::cerr << "ABORTING!!!!!!!!!!!!!!\n";
        YYABORT;
      }      

      // do manual process header cause we know it is empty
      if(operations.header_callback &&
        !operations.header_callback(0,0,operations.header_context)) 
      {
        YYABORT;
      }
    }
  ;

header_block:
  field_list {
      if(!detail::check_or_update_column_count(@1,scanner,parser,$1))
        YYABORT;
        
      if(!detail::process_header($1,operations))
        YYABORT;
    }
//   | delimited_header_list
  ;

field_list:
    field {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->push_back($1);
    }
  | DELIMITER {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->push_back(detail::empty_str);
      $$->push_back(detail::empty_str);
    }
  | DELIMITER field {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->push_back(detail::empty_str);
      $$->push_back($2);
    }
  | field_list DELIMITER {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->reserve($1->size()+1);
      $$->assign($1->begin(),$1->end());
      $$->push_back(detail::empty_str);
    }
  | field_list DELIMITER field {
      $$.reset(new YYSTYPE::str_vec_type());
      $$->reserve($1->size()+1);
      $$->assign($1->begin(),$1->end());
      $$->push_back($3);
    }
  ;

field:
    escaped_field { $$ = $1; }
  | non_escaped_field { $$ = $1; }
  ;

escaped_field:
    DQUOTE escaped_textdata_list DQUOTE { $$ = $2; }
  ;

escaped_textdata_list:  
    escaped_textdata { $$ = $1; }
  | escaped_textdata_list escaped_textdata {
      // need to aggregate so must use unique string
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
  | NL { $$ = $1; }
  | LF { $$ = $1; }
  | CR { $$ = $1; }
  | D2QUOTE { $$ = $1; }
  ;

non_escaped_field:
    TEXTDATA { $$ = $1; }
  ;

record_block:
    NL {  // A single NL means an empty record block
      // check to see if empty records are allowed
      if(!detail::check_or_update_column_count(@1,scanner,parser,detail::empty_vec))
        YYABORT;

      // manual process record cause we know it is empty, the return value doesn't matter
      if(operations.record_callback)
        operations.record_callback(0,0,operations.record_context);

    }
  | record
  | record_list
  | record_list record
  ;

record_list:
    record NL
  | record_list NL {
      // Single NL means empty record
      if(!detail::check_or_update_column_count(@2,scanner,parser,detail::empty_vec))
        YYABORT;
    
      // do manual process record cause we know it is empty
      if(operations.record_callback &&
        !operations.record_callback(0,0,operations.record_context))
      {
        YYABORT;
      }
    }
  | record_list record NL
  ;

record:
    field_list {
      if(!detail::check_or_update_column_count(@1,scanner,parser,$1))
        YYABORT;
        
      if(!detail::process_record($1,operations))
        YYABORT;
    }
  ;

%%









/**
    There are only a few tokens to be lexicographically generated. Many are setting and
    contextually dependent. The only non-single character tokens are field data content
    and the double quote (ie "");

    Only TEXTDATA strings are returned in YYSTYPE
 */
int parser_lex(YYSTYPE *lvalp, YYLTYPE *llocp, detail::scanner_state &scanner,
 detail::parser &parser)
{
  static const std::shared_ptr<std::string> lf_str(new std::string("\x0A"));
  static const std::shared_ptr<std::string> cr_str(new std::string("\x0D"));
  static const std::shared_ptr<std::string> crlf_str(new std::string("\x0D\x0A"));
  static const std::shared_ptr<std::string> quote_str(new std::string("\x22"));
  

  // <32 is non-printing
  // > 126 is non-printing
  // ' is 0x27
  // " is 0x22


  // typedef struct YYLTYPE
  // {
  //   int first_line;
  //   int first_column;
  //   int last_line;
  //   int last_column;
  // } YYLTYPE;

  // cur holds the current value
  // next holds the lookahead
  unsigned char cur;
  while(scanner.getc(cur) && scanner.advance()) {
    llocp->first_line = llocp->last_line;
    llocp->first_column = (llocp->last_column)++;

//     std::cerr << "Top scanned '";
//     if(cur > 32 && cur < 126)
//       std::cerr << cur;
//     else
//       std::cerr << static_cast<unsigned int>(cur);
//     std::cerr << "' at column " << llocp->first_column << ":" 
//       << llocp->last_column << "\n";
// 
    unsigned char next;
    scanner.getc(next);

    if(cur == parser.delimiter()) {
      return DELIMITER;
    }
    else if(cur == 0x0A) {//LF
      lvalp->str_ptr = lf_str;
      if(parser.effective_newline() != dsv_newline_crlf_strict) {
        if(parser.effective_newline() == dsv_newline_permissive)
          parser.effective_newline(dsv_newline_lf_strict);
        ++(llocp->last_line);
        llocp->last_column = 1;
        return NL;
      }
      return LF;
    }
    else if(cur == 0x0D) { //CR
      if(next == 0x0A && parser.effective_newline() != dsv_newline_lf_strict) { // LF
        scanner.advance();
        ++(llocp->last_line);
        llocp->last_column = 1;
        lvalp->str_ptr = crlf_str;
        
        if(parser.effective_newline() == dsv_newline_permissive)
          parser.effective_newline(dsv_newline_crlf_strict);
        
        return NL;
      }
      lvalp->str_ptr = cr_str;
      return CR;
    }
    else if(cur == 0x22) { //"
      lvalp->str_ptr = quote_str;
      if(next == 0x22) {
        scanner.advance();
        ++(llocp->last_column);
//         std::cerr << "got QUOTE column update " << llocp->last_column << "\n";
        return D2QUOTE;
      }
      return DQUOTE;
    }
    else {
      lvalp->str_ptr = std::shared_ptr<std::string>(new std::string(&cur, (&cur)+1));

      for(; scanner.getc(cur); scanner.advance()) {
        if(cur == parser.delimiter() ||
          cur == 0x0A || //LF
          cur == 0x0D || //CR
          cur == 0x22) // DQUOTE
        {
          return TEXTDATA;
        }

        ++(llocp->last_column);

//         std::cerr << "Textdata scanned '";
//         if(cur > 32 && cur < 126)
//           std::cerr << cur;
//         else
//           std::cerr << static_cast<unsigned int>(cur);
//         std::cerr << "' at column " << llocp->first_column << ":" 
//           << llocp->last_column << "\n";
// 
        lvalp->str_ptr->push_back(cur);
      }
      
      return TEXTDATA;
    }
  }

  return 0;
}
