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

/**
 *  \file This file is used to provide convenience functions across test cases
 */

#ifndef LIBDSV_TEST_TEST_DETAIL_H
#define LIBDSV_TEST_TEST_DETAIL_H


#ifndef TESTDATA_DIR
#error TESTDATA_DIR not defined
#endif

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#include <iostream>

namespace dsv {
namespace test {


namespace detail {


static const std::string testdatadir(QUOTEME(TESTDATA_DIR));

struct field_context {
  std::vector<std::vector<std::string> > field_matrix;
  std::size_t invocation_count;
  ssize_t invocation_limit; // -1 means do not stop before field_matrix is complete
  
  field_context(void) :invocation_count(0), invocation_limit(-1) {}
};

static int field_callback(const char *fields[], size_t size, void *context)
{
  field_context &check_context = *static_cast<field_context*>(context);

std::cerr << "CALLING field_callback\n";

  BOOST_REQUIRE_MESSAGE(check_context.invocation_limit > check_context.invocation_count,
    "field_callback called too many times. Called: " << check_context.invocation_count
    << " times before and limit was " << check_context.invocation_limit);

  BOOST_REQUIRE_MESSAGE(check_context.invocation_count < check_context.field_matrix.size(),
    "UNIT TEST FAILURE: field_callback field_matrix has less than the requested number "
    "of invocations. On invocation " << check_context.invocation_count << " and field_matrix "
    "contains " << check_context.field_matrix.size() << " fields");

  const std::vector<std::string> row = 
    check_context.field_matrix[check_context.invocation_count];

  BOOST_REQUIRE_MESSAGE(size == row.size(),
    "field_callback called with an incorrect number of fields. Should be "
    << row.size() << " received " << size);

  for(std::size_t i=0; i<size; ++i) {
    BOOST_REQUIRE_MESSAGE(row[i] == fields[i],
      "field_callback saw incorrect field. Should be '"
      << row[i] << "' received '" << fields[i] << "'");
  }

  return (++check_context.invocation_count < check_context.invocation_limit);
}

#if 0
// context is an  pointer to an existing std::vector<std::string>
int fill_vector_with_fields(const char *fields[], size_t size, void *context)
{
  std::vector<std::string> &field_vec = *static_cast<std::vector<std::string>*>(context);
  
  field_vec.clear();
  for(std::size_t i=0; i<size; ++i)
    field_vec.push_back(std::string(fields[i]));
  
  return size;
}
#endif
}


}
}

#endif
