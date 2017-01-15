/*
  Copyright (c) 2014-2017, Mike Tegtmeyer All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"
#include "api_detail.h"



/** \file
 *  \brief Unit tests for dsv parser low level interface
 */


BOOST_AUTO_TEST_SUITE( api_parser_low_object_suite )

/**
    \test Basic parser object checks. Checks low API getting and setting
    behavior only. Does not check for proper operation. For add checks of
    proper operation, place them into the appropriate api_XXX check file.

    These unit tests are scheduled before checks of the logging functionality.
    That is, do not place unit tests here that require logging functionality.

 */

/**
    \test Create and destroy uninitialized parser object
 */
BOOST_AUTO_TEST_CASE( parser_create_and_destroy )
{
  dsv_parser_t parser;
  int result = dsv_parser_create(&parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_create failed with exit code: " << result);

  dsv_parser_destroy(parser);

  BOOST_REQUIRE_MESSAGE(result == 0,
    "obj_parser_destroy failed with exit code: " << result);
}





// LOW API OBJECT CHECKS


// RECORD DELIMITER PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( record_delimiters_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int iresult = dsv_parser_set_record_delimiters(parser,0,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  size_t sresult = dsv_parser_get_record_delimiters(parser,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "delimiters expression size " << sresult << " != 0");

  iresult = dsv_parser_get_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "delimiters exclusiveflag " << iresult << " != 0");

  char bad_buff[100];
  /*
    SIZE_MAX is equivalent to std::basic_string<> npos which throws a
    std::length error exception. There really isn't a good way to check for
    a memory allocation failure with the exception of maybe an embedded system.
    It is just a possible return value for correctness.
  */
  iresult = dsv_parser_set_record_delimiters(parser,bad_buff,SIZE_MAX,0);

  BOOST_REQUIRE_MESSAGE(iresult == ENOMEM,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  std::string expr = "Fancy Expression";

  iresult = dsv_parser_set_record_delimiters(parser,expr.data(),expr.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  sresult = dsv_parser_get_record_delimiters(parser,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  // buff is filled with '*' chars
  // check buff is filled with '*' chars with exp in the middle
  std::vector<char> buff(expr.size()*3,'*');
  std::vector<char> check_buff = buff;
  std::copy(expr.begin(),expr.end(),check_buff.begin()+expr.size());

  sresult = dsv_parser_get_record_delimiters(parser,buff.data()+expr.size(),
    sresult);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  // reset buff
  std::fill(buff.begin(),buff.end(),'*');

  // check for larger then expr size
  sresult = dsv_parser_get_record_delimiters(parser,buff.data()+expr.size(),
    expr.size()*2); // note 2 since offset by expr.size()

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  // CHECK FOR BAD REGEX
  std::string bad_expr = "[*"; // bad expression
  iresult = dsv_parser_set_record_delimiters(parser,bad_expr.data(),
    bad_expr.size(),0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check to make sure the old value is still there
  std::fill(buff.begin(),buff.end(),'*');

  sresult = dsv_parser_get_record_delimiters(parser,buff.data()+expr.size(),
    sresult);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  iresult = dsv_parser_get_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "delimiters exclusiveflag " << iresult << " != 1");



  // reset buff
  std::fill(buff.begin(),buff.end(),'*');
  check_buff = buff;

  // check for smaller then expr size
  std::copy(expr.begin(),expr.begin()+expr.size()/2,
    check_buff.begin()+expr.size());

  sresult = dsv_parser_get_record_delimiters(parser,buff.data()+expr.size(),
    expr.size()/2);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size()/2,
    "delimiters expression size " << sresult << " != " << expr.size()/2);

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  iresult = dsv_parser_get_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "delimiters exclusiveflag " << iresult << " != 1");
}



// FIELD DELIMITER PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( field_delimiters_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int iresult = dsv_parser_set_field_delimiters(parser,0,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  size_t sresult = dsv_parser_get_field_delimiters(parser,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "delimiters expression size " << sresult << " != 0");

  iresult = dsv_parser_get_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "delimiters exclusiveflag " << iresult << " != 0");

  char bad_buff[100];
  /*
    SIZE_MAX is equivalent to std::basic_string<> npos which throws a
    std::length error exception. There really isn't a good way to check for
    a memory allocation failure with the exception of maybe an embedded system.
    It is just a possible return value for correctness.
  */
  iresult = dsv_parser_set_field_delimiters(parser,bad_buff,SIZE_MAX,0);

  BOOST_REQUIRE_MESSAGE(iresult == ENOMEM,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  std::string expr = "Fancy Expression";

  iresult = dsv_parser_set_field_delimiters(parser,expr.data(),expr.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  sresult = dsv_parser_get_field_delimiters(parser,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  // buff is filled with '*' chars
  // check buff is filled with '*' chars with exp in the middle
  std::vector<char> buff(expr.size()*3,'*');
  std::vector<char> check_buff = buff;
  std::copy(expr.begin(),expr.end(),check_buff.begin()+expr.size());

  sresult = dsv_parser_get_field_delimiters(parser,buff.data()+expr.size(),
    sresult);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  // reset buff
  std::fill(buff.begin(),buff.end(),'*');

  // check for larger then expr size
  sresult = dsv_parser_get_field_delimiters(parser,buff.data()+expr.size(),
    expr.size()*2); // note 2 since offset by expr.size()

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  // CHECK FOR BAD REGEX
  std::string bad_expr = "[*"; // bad expression
  iresult = dsv_parser_set_field_delimiters(parser,bad_expr.data(),
    bad_expr.size(),0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check to make sure the old value is still there
  std::fill(buff.begin(),buff.end(),'*');

  sresult = dsv_parser_get_field_delimiters(parser,buff.data()+expr.size(),
    sresult);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size(),
    "delimiters expression size " << sresult << " != " << expr.size());

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  iresult = dsv_parser_get_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "delimiters exclusiveflag " << iresult << " != 1");



  // reset buff
  std::fill(buff.begin(),buff.end(),'*');
  check_buff = buff;

  // check for smaller then expr size
  std::copy(expr.begin(),expr.begin()+expr.size()/2,
    check_buff.begin()+expr.size());

  sresult = dsv_parser_get_field_delimiters(parser,buff.data()+expr.size(),
    expr.size()/2);

  BOOST_REQUIRE_MESSAGE(sresult == expr.size()/2,
    "delimiters expression size " << sresult << " != " << expr.size()/2);

  BOOST_REQUIRE_MESSAGE(std::equal(buff.begin(),buff.end(),check_buff.begin()),
    "failed to copy correct values. expected '" << check_buff << "' received '"
    << buff );

  iresult = dsv_parser_get_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "delimiters exclusiveflag " << iresult << " != 1");
}


// FIELD ESCAPES PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( field_escapes_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::pair<std::string,std::string> > exp_seq = {
    {"open1","close1"},{"open2","close2"},{"open3","close3"},{"open4","close4"}
  };

  std::vector<const char *> open_regex_seq;
  std::vector<std::size_t> open_regex_size_seq;
  std::vector<int> open_excl_seq;
  std::vector<const char *> close_regex_seq;
  std::vector<std::size_t> close_regex_size_seq;
  std::vector<int> close_excl_seq;

  for(std::size_t i=0; i<exp_seq.size(); i++) {
    open_regex_seq.push_back(exp_seq[i].first.data());
    open_regex_size_seq.push_back(exp_seq[i].first.size());
    open_excl_seq.push_back(i%2);
    close_regex_seq.push_back(exp_seq[i].second.data());
    close_regex_size_seq.push_back(exp_seq[i].second.size());
    close_excl_seq.push_back(i%2);
  }

  int iresult;

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  std::size_t sresult = dsv_parser_num_field_escape_pairs(parser);

  BOOST_REQUIRE(sresult == exp_seq.size());

  // CHECK FOR CONTENTS
  std::size_t exp_size;
  std::vector<char> buff;
  std::vector<char> check_buff;
  for(std::size_t i=0; i<exp_seq.size(); ++i) {
    // **** CHECK OPEN ****
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,0,0);

    exp_size = exp_seq[i].first.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].first.begin(),exp_seq[i].first.end());
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));


    // CHECK USING LARGER BUFFER SIZE
    buff.assign(exp_size*3,'*');
    check_buff = buff;
    std::copy(exp_seq[i].first.begin(),exp_seq[i].first.end(),
      check_buff.begin()+exp_size);

    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,
      buff.data()+exp_size,exp_size*2);

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK USING SMALLER BUFFER SIZE
    buff.assign(exp_size*3,'*');
    check_buff = buff;
    std::copy(exp_seq[i].first.begin(),exp_seq[i].first.begin()+exp_size/2,
      check_buff.begin()+exp_size);

    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,
      buff.data()+exp_size,exp_size/2);

    BOOST_REQUIRE(sresult == exp_size/2);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));


    // **** CHECK CLOSED ****
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,0,0);

    exp_size = exp_seq[i].second.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].second.begin(),exp_seq[i].second.end());
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));


    // CHECK USING LARGER BUFFER SIZE
    buff.assign(exp_size*3,'*');
    check_buff = buff;
    std::copy(exp_seq[i].second.begin(),exp_seq[i].second.end(),
      check_buff.begin()+exp_size);

    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,
      buff.data()+exp_size,exp_size*2);

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK USING SMALLER BUFFER SIZE
    buff.assign(exp_size*3,'*');
    check_buff = buff;
    std::copy(exp_seq[i].second.begin(),exp_seq[i].second.begin()+exp_size/2,
      check_buff.begin()+exp_size);

    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,
      buff.data()+exp_size,exp_size/2);

    BOOST_REQUIRE(sresult == exp_size/2);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK FOR EXCLUSIVEFLAGS
    iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,i);
    BOOST_REQUIRE_MESSAGE(iresult == open_excl_seq[i],
      "unexpected field escape pair open exclusiveflag " << iresult
        << " for index " << i);

    iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,i);
    BOOST_REQUIRE_MESSAGE(iresult == close_excl_seq[i],
      "unexpected field escape pair close exclusiveflag " << iresult
        << " for index " << i);
  }



  // CHECK FOR 0 arrays
  iresult = dsv_parser_set_field_escape_pair(parser,
    0,open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),0,open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),0,
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    0,close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),0,close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),0,
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);



  // RECHECK CONTENTS TO MAKE SURE THINGS DIDN'T CHANGE
  for(std::size_t i=0; i<exp_seq.size(); ++i) {
    // **** CHECK OPEN ****
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,0,0);

    exp_size = exp_seq[i].first.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].first.begin(),exp_seq[i].first.end());
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));


    // **** CHECK CLOSED ****
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,0,0);

    exp_size = exp_seq[i].second.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].second.begin(),exp_seq[i].second.end());
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK FOR EXCLUSIVEFLAGS
    iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,i);
    BOOST_REQUIRE(iresult == open_excl_seq[i]);

    iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,i);
    BOOST_REQUIRE(iresult == close_excl_seq[i]);
  }



  // CHECK FOR 0 ELEMENTS
  const char *char_tmp = 0;
  size_t size_tmp = 0;

  std::swap(char_tmp,open_regex_seq.front());

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  //reset
  std::swap(char_tmp,open_regex_seq.front());

  std::swap(size_tmp,open_regex_size_seq.front());

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // reset
  std::swap(size_tmp,open_regex_size_seq.front());

  std::swap(char_tmp,close_regex_seq.front());

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // reset
  std::swap(char_tmp,close_regex_seq.front());

  std::swap(size_tmp,close_regex_size_seq.front());

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  std::swap(size_tmp,close_regex_size_seq.front());

  // RECHECK CONTENTS TO MAKE SURE THINGS DIDN'T CHANGE
  for(std::size_t i=0; i<exp_seq.size(); ++i) {
    // **** CHECK OPEN ****
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,0,0);

    exp_size = exp_seq[i].first.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].first.begin(),exp_seq[i].first.end());
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));


    // **** CHECK CLOSED ****
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,0,0);

    exp_size = exp_seq[i].second.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].second.begin(),exp_seq[i].second.end());
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK FOR EXCLUSIVEFLAGS
    iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,i);
    BOOST_REQUIRE(iresult == open_excl_seq[i]);

    iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,i);
    BOOST_REQUIRE(iresult == close_excl_seq[i]);
  }

  // CHECK FOR BAD REGEX
  const char *bad_expr = "[*"; // bad expression

  std::swap(bad_expr,open_regex_seq.front());

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  std::swap(bad_expr,open_regex_seq.front());

  std::swap(bad_expr,close_regex_seq.front());

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_regex_seq.data(),open_regex_size_seq.data(),open_excl_seq.data(),
    close_regex_seq.data(),close_regex_size_seq.data(),close_excl_seq.data(),
    exp_seq.size(),1);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  std::swap(bad_expr,close_regex_seq.front());

  // RECHECK CONTENTS TO MAKE SURE THINGS DIDN'T CHANGE
  for(std::size_t i=0; i<exp_seq.size(); ++i) {
    // **** CHECK OPEN ****
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,0,0);

    exp_size = exp_seq[i].first.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].first.begin(),exp_seq[i].first.end());
    sresult = dsv_parser_get_field_escape_pair_open_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));


    // **** CHECK CLOSED ****
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,0,0);

    exp_size = exp_seq[i].second.size();
    BOOST_REQUIRE(sresult == exp_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(exp_size,'*');
    check_buff.assign(exp_seq[i].second.begin(),exp_seq[i].second.end());
    sresult = dsv_parser_get_field_escape_pair_close_expression(parser,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == exp_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK FOR EXCLUSIVEFLAGS
    iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,i);
    BOOST_REQUIRE(iresult == open_excl_seq[i]);

    iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,i);
    BOOST_REQUIRE(iresult == close_excl_seq[i]);
  }

  // CHECK FIELD ESCAPE CLEARING
  iresult = dsv_parser_set_field_escape_pair(parser,0,0,0,0,0,0,0,0);

  sresult = dsv_parser_num_field_escape_pairs(parser);
  BOOST_REQUIRE(sresult == 0);

  sresult = dsv_parser_get_field_escape_pair_open_expression(parser,0,0,0);
  BOOST_REQUIRE(sresult == 0);

  sresult = dsv_parser_get_field_escape_pair_close_expression(parser,0,0,0);
  BOOST_REQUIRE(sresult == 0);

  iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,0);
  BOOST_REQUIRE(iresult < 0);

  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,0);
  BOOST_REQUIRE(iresult < 0);
}


// ESCAPED FIELD ESCAPES PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( escaped_field_escapes_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int iresult;
  std::size_t sresult;

  // CHECK INVALID
  sresult = dsv_parser_num_escape_field_escapes(parser,0);
  BOOST_REQUIRE(sresult == SIZE_MAX);


  std::vector<std::pair<std::string,std::string> > exp_seq = {
    {"open1","close1"},{"open2","close2"},{"open3","close3"},{"open4","close4"}
  };


  std::string open_expr("open_expression");
  std::string close_expr("close_expression");

  const char * open_seq[] = {open_expr.data()};
  std::size_t open_size[] = {open_expr.size()};
  int open_excl[] = {1};
  const char * close_seq[] = {close_expr.data()};
  std::size_t close_size[] = {close_expr.size()};
  int close_excl[] = {1};

  iresult = dsv_parser_set_field_escape_pair(parser,
    open_seq,open_size,open_excl,close_seq,close_size,close_excl,1,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);


  // CHECK EMPTY
  sresult = dsv_parser_num_escape_field_escapes(parser,0);
  BOOST_REQUIRE(sresult == 0);


  std::vector<std::pair<std::string,std::string> > escaped_field_seq = {
    {"regex1","replacement1"},{"regex2","replacement2"},
    {"regex3","replacement3"},{"regex4","replacement4"}
  };

  std::vector<const char *> regex_seq;
  std::vector<std::size_t> regex_size;
  std::vector<const char *> replacement_seq;
  std::vector<std::size_t> replacement_size;

  for(std::size_t i=0; i<escaped_field_seq.size(); ++i) {
    regex_seq.push_back(escaped_field_seq[i].first.data());
    regex_size.push_back(escaped_field_seq[i].first.size());
    replacement_seq.push_back(escaped_field_seq[i].second.data());
    replacement_size.push_back(escaped_field_seq[i].second.size());
  }

  // FILL
  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // CHECK SIZE
  sresult = dsv_parser_num_escape_field_escapes(parser,0);
  BOOST_REQUIRE(sresult == escaped_field_seq.size());

  // CHECK CONTENTS
  std::size_t seq_size;
  std::vector<char> buff;
  std::vector<char> check_buff;
  for(std::size_t i=0; i<escaped_field_seq.size(); ++i) {
    // **** CHECK EXPRESSION ****
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].first.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.end());
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK USING LARGER BUFFER SIZE
    buff.assign(seq_size*3,'*');
    check_buff = buff;
    std::copy(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.end(),check_buff.begin()+seq_size);

    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data()+seq_size,seq_size*2);

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK USING SMALLER BUFFER SIZE
    buff.assign(seq_size*3,'*');
    check_buff = buff;
    std::copy(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.begin()+seq_size/2,
      check_buff.begin()+seq_size);

    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data()+seq_size,seq_size/2);

    BOOST_REQUIRE(sresult == seq_size/2);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // **** CHECK REPLACEMENT ****
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].second.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].second.begin(),
      escaped_field_seq[i].second.end());
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK USING LARGER BUFFER SIZE
    buff.assign(seq_size*3,'*');
    check_buff = buff;
    std::copy(escaped_field_seq[i].second.begin(),
      escaped_field_seq[i].second.end(),check_buff.begin()+seq_size);

    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
      buff.data()+seq_size,seq_size*2);

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // CHECK USING SMALLER BUFFER SIZE
    buff.assign(seq_size*3,'*');
    check_buff = buff;
    std::copy(escaped_field_seq[i].second.begin(),
      escaped_field_seq[i].second.begin()+seq_size/2,
      check_buff.begin()+seq_size);

    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
      buff.data()+seq_size,seq_size/2);

    BOOST_REQUIRE(sresult == seq_size/2);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));
  }


  // CHECK FOR 0 ARRAYS

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    0,regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),0,replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),0,
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      0,escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);


  // CHECK CONTENTS TO MAKE SURE NOTHING CHANGED
  for(std::size_t i=0; i<escaped_field_seq.size(); ++i) {
    // **** CHECK EXPRESSION ****
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].first.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.end());
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // **** CHECK REPLACEMENT ****
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].second.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].second.begin(),
      escaped_field_seq[i].second.end());
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));
  }


  // CHECK FOR 0 ELEMENTS
  const char *char_tmp = 0;
  size_t size_tmp = 0;

  std::swap(char_tmp,regex_seq.front());

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  //reset
  std::swap(char_tmp,regex_seq.front());


  std::swap(size_tmp,regex_size.front());

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  //reset
  std::swap(size_tmp,regex_size.front());


  std::swap(char_tmp,replacement_seq.front());

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  //reset
  std::swap(char_tmp,replacement_seq.front());


  // CHECK CONTENTS TO MAKE SURE NOTHING CHANGED
  for(std::size_t i=0; i<escaped_field_seq.size(); ++i) {
    // **** CHECK EXPRESSION ****
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].first.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.end());
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // **** CHECK REPLACEMENT ****
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].second.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].second.begin(),
      escaped_field_seq[i].second.end());
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));
  }


  // REPLACEMENTS CAN BE ZERO SIZE SO MAKE SURE IT CLEARED
  std::swap(size_tmp,replacement_size.front());

  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  //reset
  std::swap(size_tmp,replacement_size.front());

  // CHECK CONTENTS WITH NO REPLACEMENTS
  for(std::size_t i=0; i<escaped_field_seq.size(); ++i) {
    // **** CHECK EXPRESSION ****
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].first.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.end());
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // **** CHECK REPLACEMENT ****
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,0,0);

    if(i == 0)
      BOOST_REQUIRE(sresult == 0);
    else {
      seq_size = escaped_field_seq[i].second.size();
      BOOST_REQUIRE(sresult == seq_size);

      // CHECK USING EXACT BUFFER SIZE
      buff.assign(seq_size,'*');
      check_buff.assign(escaped_field_seq[i].second.begin(),
        escaped_field_seq[i].second.end());
      sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
        buff.data(),buff.size());

      BOOST_REQUIRE(sresult == seq_size);
      BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));
    }
  }

  // REFILL
  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),escaped_field_seq.size());

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // CHECK SIZE
  sresult = dsv_parser_num_escape_field_escapes(parser,0);
  BOOST_REQUIRE(sresult == escaped_field_seq.size());

  // CHECK CONTENTS
  for(std::size_t i=0; i<escaped_field_seq.size(); ++i) {
    // **** CHECK EXPRESSION ****
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].first.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].first.begin(),
      escaped_field_seq[i].first.end());
    sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));

    // **** CHECK REPLACEMENT ****
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,0,0);

    seq_size = escaped_field_seq[i].second.size();
    BOOST_REQUIRE(sresult == seq_size);

    // CHECK USING EXACT BUFFER SIZE
    buff.assign(seq_size,'*');
    check_buff.assign(escaped_field_seq[i].second.begin(),
      escaped_field_seq[i].second.end());
    sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,i,
      buff.data(),buff.size());

    BOOST_REQUIRE(sresult == seq_size);
    BOOST_REQUIRE(std::equal(buff.begin(),buff.end(),check_buff.begin()));
  }


  //CHECK CLEARING
  iresult = dsv_parser_set_escape_field_escapes(parser,0,
    regex_seq.data(),regex_size.data(),replacement_seq.data(),
      replacement_size.data(),0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // CHECK SIZE
  sresult = dsv_parser_num_escape_field_escapes(parser,0);
  BOOST_REQUIRE(sresult == 0);

  // CHECK NO EXPRESSIONS
  sresult = dsv_parser_get_escaped_field_escape_expression(parser,0,0,0,0);
  BOOST_REQUIRE(sresult == SIZE_MAX);

  // CHECK NO REPLACEMENTS
  sresult = dsv_parser_get_escaped_field_escape_replacement(parser,0,0,0,0);
  BOOST_REQUIRE(sresult == SIZE_MAX);
}


// FIELD COLUMNS PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( field_columns_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  BOOST_REQUIRE(dsv_parser_get_field_columns(parser) == 0);

  dsv_parser_set_field_columns(parser,42);

  BOOST_REQUIRE(dsv_parser_get_field_columns(parser) == 42);

  dsv_parser_set_field_columns(parser,SIZE_MAX);

  BOOST_REQUIRE(dsv_parser_get_field_columns(parser) == SIZE_MAX);
}




BOOST_AUTO_TEST_SUITE_END()

