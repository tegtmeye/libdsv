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

#ifndef LIBDSV_PARSER_STATE_H
#define LIBDSV_PARSER_STATE_H

#include "dsv_parser.h"

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

namespace fs=boost::filesystem;
namespace bs=boost::system;


namespace detail {

  struct parse_operations {
    record_callback_t record_callback;
    void *record_context;
  };



  /**
   *  Composition object for dealing with lex/lacc reentrant interface.
   *  ie all extra info gets attached via a void * in lex/yacc
   *
   */
  class parser_state {
    public:
      parser_state(const fs::path &filepath);

      FILE * file(void) const;
      const fs::path & filepath(void) const;

      const dsv_newline_behavior & newline_behavior(void) const;
      dsv_newline_behavior newline_behavior(dsv_newline_behavior behavior);

    private:
      boost::shared_ptr<FILE> file_ptr;
      fs::path file_path;

      dsv_newline_behavior newline_flag;
  };

  inline parser_state::parser_state(const fs::path &filepath) :file_path(filepath)
  {
    errno = 0;
    FILE *tmp = fopen(filepath.c_str(),"r");
    if(!tmp)
      throw fs::filesystem_error("Unable to open file",filepath,
        bs::error_code(errno,bs::system_category()));

    file_ptr = boost::shared_ptr<FILE>(tmp,fclose);
  }

  inline FILE * parser_state::file(void) const
  {
    return file_ptr.get();
  }

  inline const fs::path & parser_state::filepath(void) const
  {
    return file_path;
  }

  inline const dsv_newline_behavior &
  parser_state::newline_behavior(void) const
  {
    return newline_flag;
  }

  inline dsv_newline_behavior
  parser_state::newline_behavior(dsv_newline_behavior behavior)
  {
    dsv_newline_behavior tmp = newline_flag;
    newline_flag = behavior;
    return tmp;
  }

}


#endif
