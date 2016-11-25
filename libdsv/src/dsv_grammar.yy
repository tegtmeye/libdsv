/*
 Copyright (c) 2014, Mike Tegtmeyer
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

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
    typedef std::shared_ptr<const char_buff_type> const_char_buff_ptr_type;

    typedef std::vector<const_char_buff_ptr_type> const_char_buff_vec_type;
    typedef std::shared_ptr<const_char_buff_vec_type> char_buff_vec_ptr_type;

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
        if(parser.field_columns() == -1) {
          return column_count_message(llocp,scanner,parser,columns,
            dsv_log_warning);
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


    static const YYSTYPE::const_char_buff_ptr_type empty_buf(
      new YYSTYPE::char_buff_type());
    static const YYSTYPE::char_buff_vec_ptr_type empty_vec(
      new YYSTYPE::const_char_buff_vec_type());
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
%token <char_buf_ptr> FIELD_DELIMITER "field delimiter"
%token <char_buf_ptr> RECORD_DELIMITER "record delimiter"
%token <char_buf_ptr> OPEN_FIELD_ESCAPE "open field escape"
%token <char_buf_ptr> CLOSE_FIELD_ESCAPE "close field escape"
%token <char_buf_ptr> ESCAPED_ESCAPE "escaped escape"
%token <char_buf_ptr> FIELDDATA


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
  | header_block RECORD_DELIMITER
  | header_block RECORD_DELIMITER record_block
  ;

empty_header:
    RECORD_DELIMITER {
      // RECORD_DELIMITER means no header. Check to see if empty records are allowed
//       std::cerr << "HERE!!!!!!!!!!!!!!\n";
      if(!detail::check_or_update_column_count(@1,scanner,parser,
        detail::empty_vec))
      {
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
      $$.reset(new YYSTYPE::const_char_buff_vec_type());
      $$->push_back($1);
    }
  | FIELD_DELIMITER {
      $$.reset(new YYSTYPE::const_char_buff_vec_type());
      $$->push_back(detail::empty_buf);
      $$->push_back(detail::empty_buf);
    }
  | FIELD_DELIMITER field {
      $$.reset(new YYSTYPE::const_char_buff_vec_type());
      $$->push_back(detail::empty_buf);
      $$->push_back($2);
    }
  | field_list FIELD_DELIMITER {
      $$.reset(new YYSTYPE::const_char_buff_vec_type());
      $$->reserve($1->size()+1);
      $$->assign($1->begin(),$1->end());
      $$->push_back(detail::empty_buf);
    }
  | field_list FIELD_DELIMITER field {
      $$.reset(new YYSTYPE::const_char_buff_vec_type());
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
    OPEN_FIELD_ESCAPE escaped_textdata_list CLOSE_FIELD_ESCAPE {
      $$ = $2;
    }
  ;

escaped_textdata_list:
    escaped_textdata { $$ = $1; }
  | escaped_textdata_list escaped_textdata {
      // need to aggregate so must use unique vector
      YYSTYPE::char_buff_ptr_type tmp(new YYSTYPE::char_buff_type());
      tmp->reserve($1->size()+$2->size());
      tmp->assign($1->begin(),$1->end());
      tmp->insert($$->end(),$2->begin(),$2->end());

      $$ = tmp;
    }
  ;

escaped_textdata:
    FIELDDATA { $$ = $1; }
  | ESCAPED_ESCAPE { $$ = $1; }
  ;

non_escaped_field:
    FIELDDATA { $$ = $1; }
  ;

record_block:
    RECORD_DELIMITER {  // A single RECORD_DELIMITER means an empty record block
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
    record RECORD_DELIMITER
  | record_list RECORD_DELIMITER {
      // Single RECORD_DELIMITER means empty record
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
  | record_list record RECORD_DELIMITER
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
    delim_repeat: potentially repeat the delimiter as many times as possible. If
    exclusiveflag is set, the first occurrence of the parsed delimiter
    (including the repeats) is set as the exclusive delimiter.

    repeatflag: repeat any of the equivalent delimiters as many times as
    possible. If the excluiveflag is set, repeat this only this delimiter as
    many times as possible.

    Special cases of the above if only one delimiter:
      - If exclusiveflag is NOT set, setting delim_repeat and repeatflag have an
      equivalent effect. Internally the effective delimiter is set with
      delim_repeat set to false and repeatflag is set to true.

      - If exclusiveflag is set, normal multi-delim behavior takes place.
      Internally, the effective delimiter is set on the first occurrence as
      usual.

    Possible cases:

      - Single defined delimiter stored in effective delimiter, if delim_repeat
      is set, try to repeat when reading

      - Multi-delimiter no repeat, no exclusive: compile to bytesequence.

      - Multi-delimiter repeat, no exclusive: compile to bytesequence. Try and
      repeat as necessary.

      - Multi-delimiter no repeat, exclusive: compile to bytesequence, found
      sequence is copied to effective delimiter for future parsing.

      - Multi-delimiter repeat, exclusive: compile to bytesequence, found
      sequence is copied to effective delimiter for future parsing.
 */





/**
    Convenience function for parser_lex

    Forget any existing putback buffer and try to read the byte seq from the
    scanner input if possible. If successful, the current read position points
    to the first byte not part of [first,last] and the read bytes remain in the
    putback buffer. If not succesful, the read position remains unchanged.

    Handles case 1.
 */
template<typename ForwardIterator>
inline std::size_t read_bytes(detail::scanner_state &scanner,
  ForwardIterator first, ForwardIterator last, bool repeat)
{
  std::size_t result = 0;

  scanner.forget();

  ForwardIterator cur;
  do {
    cur = first;
    scanner.setmark();

    while(cur != last && scanner.getc() == static_cast<int>(*cur))
      ++cur;

    if(cur != last) {
      scanner.putbackmark();

      break;
    }

    result += (last-first);
  } while(repeat);

  return result;
}

/**
    Convenience function for parser_lex

    Forget any existing putback buffer and try to read any possible byte seq
    contained in the compiled structure \c comp_byte_seq from the scanner input
    if possible. If successful, the current read position points to the first
    byte not part of the successful comp_byte_seq read and the read bytes remain
    in the putback buffer. If not succesful, the read position remains
    unchanged.
 */
std::size_t read_bytes(detail::scanner_state &scanner,
  const std::vector<detail::byte_chunk> &comp_byte_seq, bool repeat)
{
  assert(!comp_byte_seq.empty());

  scanner.forget();

  int in;
  std::size_t result = 0;
  std::size_t read_bytes;

  do {
    read_bytes = 0;
    std::ptrdiff_t byte_off = 0;
    for(in = scanner.getc(); in != EOF; /* empty */) {
      const detail::byte_chunk &chunk = comp_byte_seq[byte_off];

      if(in != chunk.byte) {
        if(!chunk.fail_skip)
          break;

        byte_off += chunk.fail_skip;
      }
      else {
        ++read_bytes;

        if(chunk.accept) {
          scanner.setmark();
          result = read_bytes;
        }

        if(!chunk.pass_skip)
          break;

        in = scanner.getc();
        byte_off += chunk.pass_skip;
      }
    }

    // putback any partially acceptable sequences
    scanner.putbackmark();
  } while (repeat && read_bytes && in != EOF);

  return result;
}















/**

 */
int parser_lex(YYSTYPE *lvalp, YYLTYPE *llocp, detail::scanner_state &scanner,
 detail::parser &parser)
{
  typedef detail::parser parser_type;
  typedef parser_type::equiv_bytesequence_type equiv_bytesequence_type;
  typedef parser_type::escaped_field_desc_seq_type escaped_field_desc_seq_type;

  lvalp->char_buf_ptr.reset();

  if(scanner.eof())
    return 0;

  llocp->first_line = llocp->last_line;
  // last_column is always 1-past as is C
  llocp->first_column = llocp->last_column;

  if(parser.escaped_field()) {

  }
  else {
    // normal scanning
    while(!scanner.eof()) {



      // just textdata
      if(!lvalp->char_buf_ptr)
        lvalp->char_buf_ptr.reset(new YYSTYPE::char_buff_type());

      ++(llocp->last_column);
      lvalp->char_buf_ptr->push_back(scanner.fgetc());
    }
  }
}





























#if 0
/**

 */
int parser_lex(YYSTYPE *lvalp, YYLTYPE *llocp, detail::scanner_state &scanner,
 detail::parser &parser)
{
  typedef detail::parser parser_type;
  typedef parser_type::equiv_bytesequence_type equiv_bytesequence_type;
  typedef parser_type::escaped_field_desc_seq_type escaped_field_desc_seq_type;

  lvalp->char_buf_ptr.reset();

  if(scanner.eof())
    return 0;

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

  if(parser.escaped_field()) {
#if 0
    while(!scanner.eof()) {
      // SEARCH FOR CLOSING FIELD ESCAPE
      std::size_t eindex = parser.effective_field_escapes_pair();
      assert(eindex != -1);

      const std::vector<equiv_bytesequence_pair> &field_escapes =
        parser.field_escapes();

      const equiv_bytesequence_type &close_seq = field_escapes[eindex].second;
      if(close_seq.effective_byteseq()) {
        // An equivalent sequence was set, just consider it
        std::size_t bytes =
          read_bytes(scanner,close_seq.effective_byteseq()->begin(),
            close_seq.effective_byteseq()->end(),
            close_seq.repeatflag());

        if(bytes) {
          if(lvalp->char_buf_ptr) {
            scanner.putback();
            return FIELDDATA;
          }

          scanner.forget();

          ++(llocp->last_line);
          llocp->last_column = 1;
          lvalp->char_buf_ptr = close_seq.effective_byteseq();
          parser.escaped_field(false);

          if(!parser.field_escapes_exclusives())
            parser.effective_field_escapes_pair(-1);

          return CLOSE_FIELD_ESCAPE;
        }
      }
      else {
        // An effective sequence was not set, search through compiled
        std::size_t bytes =
          read_bytes(scanner,close_seq.compiled_seq_vec(),
            close_seq.repeatflag());

        if(bytes) {
          if(lvalp->char_buf_ptr) {
            scanner.putback();
            return FIELDDATA;
          }

          std::shared_ptr<detail::parser::bytesequence_type> parsed_seq(
            new detail::parser::bytesequence_type());
          parsed_seq->reserve(bytes);

          for(std::size_t i=bytes; i != 0; --i)
            parsed_seq->push_back(scanner.fgetc());

          ++(llocp->last_line);
          llocp->last_column = 1;
          lvalp->char_buf_ptr = parsed_seq;
          parser.escaped_field(true);

          if(close_seq.exclusiveflag()) {
            //copy over to effective seq
            parser.set_effective_close_field_escapes(parsed_seq);
          }

          // must be set _after_ the effective setting above!!
          if(!parser.field_escapes_exclusives())
            parser.effective_field_escapes_pair(-1);

          return CLOSE_FIELD_ESCAPE;
        }
      }

      // SEARCH FOR ESCAPED ESCAPE
      const equiv_bytesequence_type &escape_escapes =
        parser.field_escape_escapes();
      if(escape_escapes.effective_byteseq()) {
        // If there is an effective bytesequence then the bytesequence is a
        // single bytesequence or it was part of an equivalent sequence set
        // but the parse exclusive flag was set.
        std::size_t bytes =
          read_bytes(scanner,escape_escapes.effective_byteseq()->begin(),
            escape_escapes.effective_byteseq()->end(),
            escape_escapes.repeatflag());

        if(bytes) {
          if(lvalp->char_buf_ptr) {
            scanner.putback();
            return FIELDDATA;
          }

          scanner.forget();
          llocp->last_column += bytes;
          lvalp->char_buf_ptr = escape_escapes.effective_byteseq();
          return ESCAPED_ESCAPE;
        }
      }
      else {
        std::size_t bytes =
          read_bytes(scanner,escape_escapes.compiled_seq_vec(),
            escape_escapes.repeatflag());

        if(bytes) {
          if(lvalp->char_buf_ptr) {
            scanner.putback();
            return FIELDDATA;
          }

          std::shared_ptr<detail::parser::bytesequence_type> parsed_seq(
            new detail::parser::bytesequence_type());
          parsed_seq->reserve(bytes);

          for(std::size_t i=bytes; i != 0; --i)
            parsed_seq->push_back(scanner.fgetc());

          llocp->last_column += bytes;
          lvalp->char_buf_ptr = parsed_seq;

          if(escape_escapes.exclusiveflag()) {
            //copy over to effective seq
            parser.set_effective_field_escape_escapes(parsed_seq);
          }

          return ESCAPED_ESCAPE;
        }
      }

      if(!lvalp->char_buf_ptr)
        lvalp->char_buf_ptr.reset(new YYSTYPE::char_buff_type());

      ++(llocp->last_column);
      lvalp->char_buf_ptr->push_back(scanner.fgetc());
    }

    // if here, we reached EOF
    return FIELDDATA;
#endif
  }
  else {
    // normal scanning
    while(!scanner.eof()) {

      // SEARCH FOR FIELD DELIMITER
      const equiv_bytesequence_type &field_delimiters =
        parser.field_delimiters();
      if(field_delimiters.effective_byteseq()) {
        // If there is an effective bytesequence then the bytesequence is a
        // single bytesequence or it was part of an equivalent sequence set but
        // the parse exclusive flag was set.
        std::size_t bytes =
          read_bytes(scanner,field_delimiters.effective_byteseq()->begin(),
            field_delimiters.effective_byteseq()->end(),
            field_delimiters.repeatflag());

        if(bytes) {
          scanner.forget();
  //         std::cerr << "GOT EFFECTIVE: '" << *effective_delimiter << "'\n";
          llocp->last_column += bytes;
          lvalp->char_buf_ptr = field_delimiters.effective_byteseq();
          return FIELD_DELIMITER;
        }
      }
      else {
        std::size_t bytes =
          read_bytes(scanner,field_delimiters.compiled_seq_vec(),
            field_delimiters.repeatflag());

        if(bytes) {
          std::shared_ptr<detail::parser::bytesequence_type> parsed_seq(
            new detail::parser::bytesequence_type());
          parsed_seq->reserve(bytes);

          for(std::size_t i=bytes; i != 0; --i)
            parsed_seq->push_back(scanner.fgetc());

          llocp->last_column += bytes;
          lvalp->char_buf_ptr = parsed_seq;

          if(field_delimiters.exclusiveflag()) {
            //copy over to effective seq
            parser.set_effective_field_delimiters(parsed_seq);
          }

          return FIELD_DELIMITER;
        }
      }

      // SEARCH FOR RECORD DELIMITER
      const equiv_bytesequence_type &record_delimiters =
        parser.record_delimiters();
      if(record_delimiters.effective_byteseq()) {
        std::size_t bytes =
          read_bytes(scanner,record_delimiters.effective_byteseq()->begin(),
            record_delimiters.effective_byteseq()->end(),
            record_delimiters.repeatflag());

        if(bytes) {
          scanner.forget();

          ++(llocp->last_line);
          llocp->last_column = 1;
          lvalp->char_buf_ptr = record_delimiters.effective_byteseq();
          return RECORD_DELIMITER;
        }
      }
      else {
        std::size_t bytes =
          read_bytes(scanner,record_delimiters.compiled_seq_vec(),
            record_delimiters.repeatflag());

        if(bytes) {
          std::shared_ptr<detail::parser::bytesequence_type> parsed_seq(
            new detail::parser::bytesequence_type());
          parsed_seq->reserve(bytes);

          for(std::size_t i=bytes; i != 0; --i)
            parsed_seq->push_back(scanner.fgetc());

          ++(llocp->last_line);
          llocp->last_column = 1;
          lvalp->char_buf_ptr = parsed_seq;

          if(record_delimiters.exclusiveflag()) {
            //copy over to effective seq
            parser.set_effective_record_delimiter(parsed_seq);
          }

          return RECORD_DELIMITER;
        }
      }

      // SEARCH FOR OPEN FIELD ESCAPE
      const escaped_field_desc_seq_type &field_escapes =
        parser.field_escapes();

      std::size_t eindex = parser.effective_field_escapes_pair();
      if(eindex != -1) {
        // exclusive has been set on a certain pair
        const equiv_bytesequence_type &open_seq =
          field_escapes[eindex].open_equiv_bytesequence;

        if(open_seq.effective_byteseq()) {
          // An equivalent sequence was set, just consider it
          std::size_t bytes =
            read_bytes(scanner,open_seq.effective_byteseq()->begin(),
              open_seq.effective_byteseq()->end(),
              open_seq.repeatflag());

          if(bytes) {
            scanner.forget();

            llocp->last_column += bytes;
            lvalp->char_buf_ptr = open_seq.effective_byteseq();
            parser.escaped_field(true);
            return OPEN_FIELD_ESCAPE;
          }
        }
        else {
          // An effective sequence was not set, search through compiled
          std::size_t bytes =
            read_bytes(scanner,open_seq.compiled_seq_vec(),
              open_seq.repeatflag());

          if(bytes) {
            std::shared_ptr<detail::parser::bytesequence_type> parsed_seq(
              new detail::parser::bytesequence_type());
            parsed_seq->reserve(bytes);

            for(std::size_t i=bytes; i != 0; --i)
              parsed_seq->push_back(scanner.fgetc());

            llocp->last_column += bytes;
            lvalp->char_buf_ptr = parsed_seq;
            parser.escaped_field(true);

            if(open_seq.exclusiveflag()) {
              //copy over to effective seq
              parser.set_effective_open_field_escapes(parsed_seq);
            }

            return OPEN_FIELD_ESCAPE;
          }
        }
      }
      else {
        // search through and see if we find a valid open
        for(std::size_t i = 0; i<field_escapes.size(); ++i) {
          const equiv_bytesequence_type &open_seq =
            field_escapes[eindex].open_equiv_bytesequence;

          if(open_seq.effective_byteseq()) {
            // An equivalent sequence was set, just consider it
            std::size_t bytes =
              read_bytes(scanner,open_seq.effective_byteseq()->begin(),
                open_seq.effective_byteseq()->end(),
                open_seq.repeatflag());

            if(bytes) {
              scanner.forget();

              llocp->last_column += bytes;
              lvalp->char_buf_ptr = open_seq.effective_byteseq();
              parser.effective_field_escapes_pair(i);
              parser.escaped_field(true);
              return OPEN_FIELD_ESCAPE;
            }
          }
          else {
            // An effective sequence was not set, search through compiled
            std::size_t bytes =
              read_bytes(scanner,open_seq.compiled_seq_vec(),
                open_seq.repeatflag());

            if(bytes) {
              std::shared_ptr<detail::parser::bytesequence_type> parsed_seq(
                new detail::parser::bytesequence_type());
              parsed_seq->reserve(bytes);

              for(std::size_t i=bytes; i != 0; --i)
                parsed_seq->push_back(scanner.fgetc());

              llocp->last_column += bytes;
              lvalp->char_buf_ptr = parsed_seq;
              parser.effective_field_escapes_pair(i);
              parser.escaped_field(true);

              if(open_seq.exclusiveflag()) {
                //copy over to effective seq
                parser.set_effective_open_field_escapes(parsed_seq);
              }

              return OPEN_FIELD_ESCAPE;
            }
          }
        }
      }

      // just textdata
      if(!lvalp->char_buf_ptr)
        lvalp->char_buf_ptr.reset(new YYSTYPE::char_buff_type());

      ++(llocp->last_column);
      lvalp->char_buf_ptr->push_back(scanner.fgetc());
    }

    // if here, we reached EOF
    return FIELDDATA;
  }

  return 0;
}

#endif
