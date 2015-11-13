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

  #include <iostream>

  // Change me with bison version > 3
  struct YYSTYPE {
    // use vectors of unsigned characters instead of std::string so that we
    // can store 0s
    typedef std::vector<unsigned char> char_buff_type;
    typedef std::shared_ptr<char_buff_type> char_buff_ptr_type;

    typedef std::vector<char_buff_ptr_type> char_buff_vec_type;
    typedef std::shared_ptr<char_buff_vec_type> char_buff_vec_ptr_type;

    // shared_ptr to character buffer
    char_buff_ptr_type char_buf_ptr;

    // shared_ptr to vector of shared_ptrs to character buffers
    char_buff_vec_ptr_type char_buf_vec_ptr;
  };

}


%code {
  #include "dsv_grammar.hh"
  #include <iostream>
  #include <sstream>
  #include <iomanip>

  /**
   *  Error reporting function as required by Bison
   *  These are always errors
   */
  void parser_error(YYLTYPE *llocp, const detail::scanner_state &scanner,
    detail::parser &parser, const detail::parse_operations &operations,
    const std::unique_ptr<detail::scanner_state> &context, const char *s)
  {
    log_callback_t logger = parser.log_callback();
    if((parser.log_level() & dsv_log_error) && logger) {
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

      logger(dsv_syntax_error,dsv_log_error,fields,
        sizeof(fields)/sizeof(const char *),parser.log_context());
    }
  }

  bool column_count_message(const YYLTYPE &llocp,
    const detail::scanner_state &scanner, detail::parser &parser,
    std::size_t rec_cols, dsv_log_level level)
  {
//     std::cerr << "Generating column_count_message: LOG LEVEL: "
//       << parser.log_level() << "\n";

    bool result = !(level & dsv_log_error);

    // - The line number associated with the start of the offending row[*][**]
    // - The line number associated with the end of the offending row[*][**]
    // - The expected number of fields[*]
    // - The number of fields parsed for this row[*]
    // - The location_str associated with the syntax error if it was supplied to
    //   \c dsv_parse
    log_callback_t logger = parser.log_callback();
    if((parser.log_level() & level) && logger) {
      std::string first_line = std::to_string(llocp.first_line);
      std::string last_line = std::to_string(llocp.last_line);
      std::string exp_columns =
        std::to_string(parser.effective_field_columns());

      std::string rec_columns = std::to_string(rec_cols);
      std::string filename = scanner.filename();

      const char *fields[] = {
        first_line.c_str(),
        last_line.c_str(),
        exp_columns.c_str(),
        rec_columns.c_str(),
        filename.c_str()
      };

      bool user_res = logger(dsv_inconsistant_column_count,level,fields,
        sizeof(fields)/sizeof(const char *),parser.log_context());

      // only allow the parsing to continue if the leve was not an error and
      // logger returns true;
      result = (result & user_res);
    }

    return result;
  }

  bool unexpected_binary(const YYLTYPE &llocp,
    const detail::scanner_state &scanner, detail::parser &parser,
    const YYSTYPE::char_buff_type &char_buf, dsv_log_level level)
  {
    bool result = !(level & dsv_log_error);

    // - The offending line associated with the start of the syntax error[*][**]
    // - The offending line associated with the end of the syntax error[*][**]
    // - The offending character associated with the start of the syntax
    //    error[*]
    // - The offending character associated with the end of the syntax error[*]
    // - A byted-oriented string containing the hexadecimal representation of
    //    the
    //   offending binary content.[***]
    // - The location_str associated with the syntax error if it was supplied to
    //   \c dsv_parse
    log_callback_t logger = parser.log_callback();
    if((parser.log_level() & level) && logger) {
      std::string first_line = std::to_string(llocp.first_line);
      std::string last_line = std::to_string(llocp.last_line);
      std::string first_column = std::to_string(llocp.first_column);
      std::string last_column = std::to_string(llocp.last_column);
      std::string filename = scanner.filename();

      std::stringstream out;
      for(std::size_t i=0; i<char_buf.size(); ++i)
        out << std::hex << std::showbase << std::internal << std::setfill('0')
          << std::setw(4) << (unsigned int)(char_buf[i]);
      std::string out_str = out.str();

      const char *fields[] = {
        first_line.c_str(),
        last_line.c_str(),
        first_column.c_str(),
        last_column.c_str(),
        out_str.c_str(),
        filename.c_str()
      };

      bool user_res = logger(dsv_unexpected_binary,level,fields,
        sizeof(fields)/sizeof(const char *),parser.log_context());

      // only allow the parsing to continue if the leve was not an error and
      // logger returns true;
      result = (result & user_res);
    }

    return result;
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
      const YYSTYPE::char_buff_vec_ptr_type &char_buf_vec_ptr)
    {
      ssize_t columns = char_buf_vec_ptr->size();

      if(!parser.effective_field_columns_set()) {
//         std::cerr << "effective_field_columns_set NOT set\n";
        parser.effective_field_columns_set(true);
        parser.effective_field_columns(columns);
      }
      else if(parser.effective_field_columns() != columns) {
//         std::cerr << "effective_field_columns_set set && cols not equal\n";
        if(parser.field_columns() < 0) {
          return column_count_message(llocp,scanner,parser,columns,dsv_log_warning);
        }
        else {
          column_count_message(llocp,scanner,parser,columns,dsv_log_error);
          return false;
        }
      }

      return true;
    }

    bool process_header(const YYSTYPE::char_buff_vec_ptr_type &char_buf_vec_ptr,
      detail::parse_operations &operations)
    {
//         std::cerr << "CALLING PROCESS_HEADER\n";
      bool keep_going = true;
      if(operations.header_callback) {
//          std::cerr << "got size " << str_vec_ptr->size() << "\nGot:\n";
//          for(int i=0; i<str_vec_ptr->size(); ++i)
//            std::cerr << "\t" << (*str_vec_ptr)[i] << "\n";

        operations.field_storage.clear();
        operations.len_storage.clear();
        operations.field_storage.reserve(char_buf_vec_ptr->size());
        operations.len_storage.reserve(char_buf_vec_ptr->size());

        for(size_t i=0; i<char_buf_vec_ptr->size(); ++i) {
          operations.field_storage.push_back((*char_buf_vec_ptr)[i]->data());
          operations.len_storage.push_back((*char_buf_vec_ptr)[i]->size());
        }

//        std::cerr << "CALLING REGISTERED CALLBACK\n";
        keep_going = operations.header_callback(operations.field_storage.data(),
          operations.len_storage.data(),operations.field_storage.size(),
          operations.header_context);
      }
      return keep_going;
    }

    bool process_record(const YYSTYPE::char_buff_vec_ptr_type &char_buf_vec_ptr,
      detail::parse_operations &operations)
    {
//        std::cerr << "CALLING PROCESS_RECORD\n";
      bool keep_going = true;
      if(operations.record_callback) {
        operations.field_storage.clear();
        operations.len_storage.clear();
        operations.field_storage.reserve(char_buf_vec_ptr->size());
        operations.len_storage.reserve(char_buf_vec_ptr->size());

        for(size_t i=0; i<char_buf_vec_ptr->size(); ++i) {
          operations.field_storage.push_back((*char_buf_vec_ptr)[i]->data());
          operations.len_storage.push_back((*char_buf_vec_ptr)[i]->size());
        }

        keep_going = operations.record_callback(operations.field_storage.data(),
          operations.len_storage.data(),operations.field_storage.size(),
          operations.record_context);
      }
      return keep_going;
    }

    std::string to_string(const YYSTYPE::char_buff_type &buf)
    {
      std::stringstream out;

      for(std::size_t i=0; i<buf.size(); ++i) {
        if(buf[i] > 32 || buf[i] < 126)
          out << buf[i];
        else
          out << std::hex << std::showbase << buf[i];
      }

      return out.str();
    }


    static const YYSTYPE::char_buff_ptr_type empty_buf(
      new YYSTYPE::char_buff_type());
    static const YYSTYPE::char_buff_vec_ptr_type empty_vec(
      new YYSTYPE::char_buff_vec_type());
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
%parse-param {detail::parse_operations &operations}
%parse-param {const std::unique_ptr<detail::scanner_state> &context}

%token END 0 "end-of-file"
%token <char_buf_ptr> DELIMITER "delimiter"
//%token HEADER_DELIMITER "header delimiter"
%token <char_buf_ptr> LF "linefeed"
%token <char_buf_ptr> CR "carriage-return"
%token <char_buf_ptr> NL "newline"
%token DQUOTE "\""
%token <char_buf_ptr> D2QUOTE "\"\""
%token <char_buf_ptr> TEXTDATA
%token BINARYDATA "binary data"


// file
%type <char_buf_vec_ptr> field_list
%type <char_buf_ptr> field
%type <char_buf_ptr> escaped_field;
%type <char_buf_ptr> escaped_textdata_list
%type <char_buf_ptr> escaped_textdata
%type <char_buf_ptr> non_escaped_field;



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
        !operations.header_callback(0,0,0,operations.header_context))
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
      $$.reset(new YYSTYPE::char_buff_vec_type());
      $$->push_back($1);
    }
  | DELIMITER {
      $$.reset(new YYSTYPE::char_buff_vec_type());
      $$->push_back(detail::empty_buf);
      $$->push_back(detail::empty_buf);
    }
  | DELIMITER field {
      $$.reset(new YYSTYPE::char_buff_vec_type());
      $$->push_back(detail::empty_buf);
      $$->push_back($2);
    }
  | field_list DELIMITER {
      $$.reset(new YYSTYPE::char_buff_vec_type());
      $$->reserve($1->size()+1);
      $$->assign($1->begin(),$1->end());
      $$->push_back(detail::empty_buf);
    }
  | field_list DELIMITER field {
      $$.reset(new YYSTYPE::char_buff_vec_type());
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
    open_quote escaped_textdata_list close_quote {
      $$ = $2;
    }
  ;

open_quote:
    DQUOTE {
//       std::cerr << "TURNING ON ESCAPED FIELD\n";
      parser.escaped_field(true);
    }
  ;

close_quote:
    DQUOTE {
//       std::cerr << "TURNING OFF ESCAPED FIELD\n";
      parser.escaped_field(false);
    }
  ;

escaped_textdata_list:
    escaped_textdata { $$ = $1; }
  | escaped_textdata_list escaped_textdata {
      // need to aggregate so must use unique vector
      $$.reset(new YYSTYPE::char_buff_type());
      $$->reserve($1->size()+$2->size());
      $$->assign($1->begin(),$1->end());
      $$->insert($$->end(),$2->begin(),$2->end());
    }
  ;

escaped_textdata:
    TEXTDATA { $$ = $1; }
  | DELIMITER { $$ = $1; }
  | NL { $$ = $1; } // NL are always accepted
  | LF {
      // LF is returned if it wasn't already considered an NL
      if(!parser.escaped_binary_fields()) {
        unexpected_binary(@1,scanner,parser,*$1,dsv_log_error);
        YYABORT;
      }

    }
  | CR {
      // CR is returned if it wasn't already considered an NL, ie CRLF
      if(!parser.escaped_binary_fields()) {
        unexpected_binary(@1,scanner,parser,*$1,dsv_log_error);
        YYABORT;
      }

      $$ = $1;
    }
  | D2QUOTE { $$ = $1; }
  ;

non_escaped_field:
    TEXTDATA { $$ = $1; }
  ;

record_block:
    NL {  // A single NL means an empty record block
      // check to see if empty records are allowed
      if(!detail::check_or_update_column_count(@1,scanner,parser,
        detail::empty_vec))
      {
        YYABORT;
      }

      // manual process record cause we know it is empty, the return value
      // doesn't matter
      if(operations.record_callback)
        operations.record_callback(0,0,0,operations.record_context);

    }
  | record
  | record_list
  | record_list record
  ;

record_list:
    record NL
  | record_list NL {
      // Single NL means empty record
      if(!detail::check_or_update_column_count(@2,scanner,parser,
        detail::empty_vec))
      {
        YYABORT;
      }

      // do manual process record cause we know it is empty
      if(operations.record_callback &&
        !operations.record_callback(0,0,0,operations.record_context))
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

std::ostream & operator<<(std::ostream &out,
  const std::vector<unsigned char> &seq)
{
  std::vector<unsigned char>::const_iterator cur = seq.begin();

  while(cur != seq.end()) {
    if(*cur < 32 || *cur > 126)
      out << int(*cur);
    else
      out << char(*cur);

    ++cur;
  }

  return out;
}

std::string ascii(int c)
{
  std::stringstream out;

  if(c >= 32 && c <= 126)
    out << char(c);
  else
    out << c;

  return out.str();
}

/**
    Convenience function for parser_lex

    We don't copy into the actual delimiter representation here for efficiency

    Forget any existing putback buffer and try to read the delimiter from the
    scanner input if possible. The read characters remain in the putback buffer.
 */
template<typename ForwardIterator>
bool read_delimiter(detail::scanner_state &scanner,
  ForwardIterator first, ForwardIterator last)
{
  scanner.forget();
  while(first != last && scanner.advancec() == static_cast<int>(*first))
    ++first;

  return first == last;
}

/**
    Convenience function for parser_lex

    We don't copy into the actual delimiter representation here for efficiency

    Forget any existing putback buffer and search for a valid delimiter as
    represented by the compiled byte sequence \c comp_byte_seq
 */
std::size_t search_delimiter(detail::scanner_state &scanner,
  const std::vector<detail::byte_chunk> &comp_byte_seq)
{
  assert(!comp_byte_seq.empty());

  scanner.forget();

  std::size_t result = 0;

  std::size_t read_bytes = 0;
  std::ptrdiff_t byte_off = 0;
  for(int in = scanner.getc(); in != EOF; /* empty */) {
    const detail::byte_chunk &chunk = comp_byte_seq[byte_off];

    if(in != chunk.byte) {
      if(!chunk.fail_skip)
        break;

      byte_off += chunk.fail_skip;
    }
    else {
      ++read_bytes;
      scanner.advancec();
      in = scanner.getc();

      if(chunk.accept)
        result = read_bytes;

      if(!chunk.pass_skip)
        break;

      byte_off += chunk.pass_skip;
    }
  }

  return result;
}








/**
    There are only a few tokens to be lexicographically generated. Many are
    setting and contextually dependent. The only non-single character tokens are
    field data content and the double quote (ie "");

 */
int parser_lex(YYSTYPE *lvalp, YYLTYPE *llocp, detail::scanner_state &scanner,
 detail::parser &parser)
{
  static const unsigned char crlf_il[] = {0x0D,0x0A};
  static const YYSTYPE::char_buff_ptr_type
    lf_buf(new YYSTYPE::char_buff_type(1,0x0A));
  static const YYSTYPE::char_buff_ptr_type
    cr_buf(new YYSTYPE::char_buff_type(1,0x0D));
  static const YYSTYPE::char_buff_ptr_type
    crlf_buf(new YYSTYPE::char_buff_type(crlf_il,
      crlf_il+sizeof(crlf_il)/sizeof(unsigned char)));

  static const YYSTYPE::char_buff_ptr_type
    quote_buf(new YYSTYPE::char_buff_type(1,0x22));

  // <32 is non-printing
  // > 126 is non-printing
  // ' is 0x27
  // " is 0x22

  int cur;
  while((cur = scanner.getc()) != EOF) {
    // precondition is the putback buffer is empty
    // we know it isn't EOF but not sure how big the token is...


    llocp->first_line = llocp->last_line;
    // last_column is always 1-past as is C
    llocp->first_column = llocp->last_column;

//     {
//       std::cerr << "Top scanned ";
//       if(cur < 32 || cur > 126)
//         std::cerr << "'" << std::hex << std::showbase << int(cur) << std::dec
//           << std::noshowbase << "'";
//       else
//         std::cerr << "'" << char(cur) << "' [" << (unsigned int)(cur) << "]";
//
//       std::cerr << " at row: " << llocp->first_line << ":" << llocp->last_line
//         << " col: "
//         << llocp->first_column << ":" << llocp->last_column << "\n";
//     }

    auto effective_delimiter = parser.effective_delimiter();
    if(effective_delimiter) {
      // just read the effective delimiter
      if(read_delimiter(scanner,effective_delimiter->begin(),
        effective_delimiter->end()))
      {
//         std::cerr << "GOT EFFECTIVE: '" << *effective_delimiter << "'\n";
        scanner.forget();
        llocp->last_column += effective_delimiter->size();
        lvalp->char_buf_ptr = effective_delimiter;
        return DELIMITER;
      }
      else
        scanner.putback();
    }
    else {
      std::size_t search_len = search_delimiter(scanner,
        parser.compiled_delimiter_vec());
      scanner.putback();

//       std::cerr << "SEARCHED LEN: " << search_len << "\n";

      if(search_len) {
        std::shared_ptr<detail::parser::byte_vec_type> parsed_delimiter(
          new detail::parser::byte_vec_type());
        parsed_delimiter->reserve(search_len);

        for(std::size_t i=search_len; i != 0; --i)
          parsed_delimiter->push_back(scanner.advancec());

//         std::cerr << "SEARCHED: '" << *parsed_delimiter << "'\n";

        llocp->last_column += search_len;
        lvalp->char_buf_ptr = parsed_delimiter;

        if(parser.delimiter_parse_exclusive()) {
//           std::cerr << "Setting effective\n";
          parser.effective_delimiter(parsed_delimiter);
        }

        scanner.forget();

        return DELIMITER;
      }
    }

    if(cur == 0x0A) {//LF
      ++(llocp->last_column);
      scanner.fadvancec();

      if(parser.effective_newline() != dsv_newline_crlf_strict) {
        // only register the effective newline if we are not in a quoted field
        if(parser.effective_newline() == dsv_newline_permissive
          && !(parser.escaped_field() && parser.escaped_binary_fields()))
        {
          parser.effective_newline(dsv_newline_lf_strict);
//           std::cerr << "SETTING EFFECTIVE LF\n";
        }
//         else
//           std::cerr << "IGNORING SETTING EFFECTIVE LF\n";

        ++(llocp->last_line);
        llocp->last_column = 1;
        lvalp->char_buf_ptr = lf_buf;
        return NL;
      }

      lvalp->char_buf_ptr = lf_buf;
      return LF;
    }

    if(cur == 0x0D) { //CR
      ++(llocp->last_column);
      scanner.fadvancec();

      if(scanner.getc() == 0x0A // LF
        && parser.effective_newline() != dsv_newline_lf_strict)
      {
        scanner.fadvancec();
        ++(llocp->last_line);
        llocp->last_column = 1;
        lvalp->char_buf_ptr = crlf_buf;

        // only register the effective newline if we are not in a quoted field
        if(parser.effective_newline() == dsv_newline_permissive
          && !(parser.escaped_field() && parser.escaped_binary_fields()))
        {
          parser.effective_newline(dsv_newline_crlf_strict);
//           std::cerr << "SETTING EFFECTIVE CRLF\n";
        }
//         else
//           std::cerr << "IGNORING SETTING EFFECTIVE CRLF\n";

        return NL;
      }

      lvalp->char_buf_ptr = cr_buf;
      return CR;
    }

    if(cur == 0x22) { //"
      ++(llocp->last_column);
      scanner.fadvancec();
      lvalp->char_buf_ptr = quote_buf;

      if(scanner.getc() == 0x22) {
        ++(llocp->last_column);
        scanner.fadvancec();

        return D2QUOTE;
      }

      return DQUOTE;
    }

    if(parser.escaped_field() && parser.escaped_binary_fields()) {
      // straight textdata
      lvalp->char_buf_ptr.reset(new YYSTYPE::char_buff_type());

      // only a DQUOTE will terminate a binary enabled escaped field. Don't eat
      // until we know it is not a terminating byte
      while((cur = scanner.getc()) != EOF) {
        if(cur == 0x22) { // DQUOTE
          return TEXTDATA;
        }

// std::cerr << "TEXTDATA scanned '" << char(cur) << "' token now at row: "
//   << llocp->first_line << ":" << llocp->last_line << " col: "
//   << llocp->first_column << ":" << llocp->last_column << "\n";

        ++(llocp->last_column);
        lvalp->char_buf_ptr->push_back(cur);
        scanner.fadvancec();
      }

      return TEXTDATA;
    }

    if(cur < 32 || cur > 126) { // non-ASCII
      ++(llocp->last_column);
      scanner.fadvancec();
      return BINARYDATA;
    }

    // fallthrough
    // straight textdata
//     std::cerr << "Adding TEXTDATA: '" << ascii(cur) << "'\n";

    lvalp->char_buf_ptr.reset(
      new YYSTYPE::char_buff_type(1,scanner.fadvancec()));
    ++(llocp->last_column);

    // scan for anything that could terminate the field. Don't eat
    // until we know it is not a terminating byte
    while((cur = scanner.getc()) != EOF) {
// std::cerr << "Processing: '" << ascii(cur) << "'\n";

      bool lookahead_delimiter = false;

      if(parser.effective_delimiter()) {
//         std::cerr << "has effective. reading delimiter\n";
        lookahead_delimiter = read_delimiter(scanner,
          parser.effective_delimiter()->begin(),
          parser.effective_delimiter()->end());
      }
      else {
//         std::cerr << "no effective. searching for delimiter\n";
        lookahead_delimiter = search_delimiter(scanner,
          parser.compiled_delimiter_vec());
      }

      scanner.putback();

      if(lookahead_delimiter
        || cur == 0x0A //LF
        || cur == 0x0D //CR
        || cur == 0x22 // DQUOTE
        || cur < 32 || cur > 126) // non-ASCII
      {
//         std::cerr << "saw delimiter. returning TEXTDATA\n";
        return TEXTDATA;
      }

//       {
//         std::cerr << "TEXTDATA scanned '" << char(cur) << "' token now at row: "
//           << llocp->first_line << ":" << llocp->last_line << " col: "
//           << llocp->first_column << ":" << llocp->last_column << "\n";
//       }

//       std::cerr << "no delimiter. adding '" << ascii(cur) << "'\n";
      ++(llocp->last_column);
      scanner.fadvancec();
      lvalp->char_buf_ptr->push_back(cur);
    }

    return TEXTDATA;
  }

  return 0;
}
