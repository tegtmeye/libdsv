#ifndef LIBDSV_PARSER_STATE_H
#define LIBDSV_PARSER_STATE_H


#include <string>
#include <sstream>
#include <list>

#include <boost/shared_ptr.hpp>

namespace detail {

  class file_error : public std::runtime_error {
    public:
      explicit file_error(const std::string &what_arg, int err)
        :std::runtime_error(what_arg), error_num(err) {}

      int error_code(void) const {
        return error_num;
      }

    private:
      int error_num;
  };


  template<typename CharT>
  struct basic_parse_state {
    typedef typename std::basic_string<CharT> string_type;

    string_type filename;
    string_type dir;
    std::size_t lineno;
    FILE *file;

    basic_parse_state(const std::basic_string<CharT> &fname);
    ~basic_parse_state(void);
  };

  template<typename CharT>
  basic_parse_state<CharT>::basic_parse_state(const std::basic_string<CharT> &fname)
    :filename(fname)
  {
    typename string_type::size_type loc = filename.find_last_of('/');
    if(loc == string_type::npos)
      dir = ".";
    else
      dir = filename.substr(0,loc);

    lineno = 1;

    file = fopen(filename.c_str(),"r");
    if(!file)
      throw file_error("Unable to open file",errno);
  }

  template<typename CharT>
  basic_parse_state<CharT>::~basic_parse_state(void)
  {
    // If no filename, then it was created directly from a stream
    if(file && !filename.empty())
      fclose(file);
  }

  // Helper to prevent closing of the stream
//  struct no_close {
//    void operator()(FILE*) {}
//  };

//  template<typename charT, typename ObjContentsT>
//  static int set_stream(FILE *stream, basic_parser_state<charT,ObjContentsT> &state)
//  {
//    typedef basic_parser_state<charT,ObjContentsT> state_type;
//    typedef typename state_type::file_node file_node_type;
//    typedef typename state_type::string_type string_type;
//
//    file_node_type node;
//    node.dir = ".";
//    node.lineno = 1;
//    node.file.reset(stream,no_close());
//    state.file_stack.push_back(node);
//
//    return 0;
//  }




  /**
   *  Composition object for dealing with lex/lacc reentrant interface.
   *  ie all extra info gets attached via a void * in lex/yacc
   *
   *  Lifetime is only until the parse completion. That is, do not retain parser
   *  state that only lasts the length of the parse operation
   */
  template<typename CharT>
  class basic_dsv_parser {
    public:
      typedef basic_parse_state<CharT> parser_state_type;

      int parse_file(const char *filename);

      parser_state_type & state(void) const {
        return state_stack.back()->parser_state;
      }

    private:
      struct state_wrapper {
        yyscan_t scanner;
        parser_state_type parser_state;

        state_wrapper(const char *filename);
        ~state_wrapper(void);
      };

      std::list<boost::shared_ptr<state_wrapper> > state_stack;
  };

  template<typename CharT>
  basic_dsv_parser<CharT>::state_wrapper::state_wrapper(const char *filename)
    :parser_state(filename)
  {
    dsv_parser_lex_init(&scanner);
  }

  template<typename CharT>
  basic_dsv_parser<CharT>::state_wrapper::~state_wrapper(void)
  {
    dsv_parser_lex_destroy(scanner);
  }

  template<typename CharT>
  int basic_dsv_parser<CharT>::parse_file(const char *filename)
  {
    boost::shared_ptr<state_wrapper> new_wrapper(new state_wrapper(filename));
    state_stack.push_back(new_wrapper);

    state_wrapper &wrapper = *state_stack.back();

    dsv_parser_set_in(wrapper.parser_state.file,wrapper.scanner);
    dsv_parser_set_extra(&wrapper.parser_state,wrapper.scanner);

    // start parsing
    int err = dsv_parser_parse(wrapper.scanner,this);

    state_stack.pop_back();

    if(err == 2)
      err = ENOMEM;

    if(err != 0)
      err = -1;

    return err;
  }

  typedef basic_dsv_parser<char> dsv_parser;











}


#endif
