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
#include "api_detail.h"

#include <cstdlib>

/** \file
 *  \brief Unit tests for the default settings for the different parsers
 */

namespace fs=boost::filesystem;


BOOST_AUTO_TEST_SUITE( api_defaults_parse_suite )

/**
    \test Basic parser default checks. Checks for proper operation using
    the default parser settings

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

/** \test Attempt to parse a unnamed file with a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_unnamed_file_with_zero_stream )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  BOOST_REQUIRE(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  int result = dsv_parse(0,0,parser,operations);
  if(result < 0)
    std::cerr << output_logs(log_context.recd_logs) << "\n";

  /**
   *  It appears that the underlying fopen call can return error codes in
   *  platform dependent ways. For example, in POSIX it seems that fopen(0,...)
   *  would return 0 and set errno to ENOENT but on MAC it return EFAULT which
   *  is equally justifiable so just make sure it doesn't return 0 here.
   */
  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse attempted to open a nonexistent file and did not return "
    "nonzero");
}


/** \test Attempt to parse a named nonexistent file using a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_named_nonexistent_file_with_zero_stream )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  BOOST_REQUIRE(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  int result = dsv_parse("nonexistant_file.dsv",0,parser,operations);
  if(result < 0)
    std::cerr << output_logs(log_context.recd_logs) << "\n";

  BOOST_REQUIRE_MESSAGE(result == ENOENT,
    "dsv_parse attempted to open a nonexistent file and did not return ENOENT");
}

/** \test Attempt to parse an unnamed empty file opened with a stream
 */
BOOST_AUTO_TEST_CASE( parse_unnamed_empty_file_with_stream )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  BOOST_REQUIRE(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  std::vector<detail::field_storage_type> file_contents{
  };

  fs::path filepath = detail::gen_testfile(file_contents,
    "parse_unnamed_empty_file_with_stream");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  BOOST_REQUIRE_MESSAGE(in.get() != 0,
    "Unit test failure: could not open unit test data file \"" << filepath
      << "\"");

  int result = dsv_parse(0,in.get(),parser,operations);
  if(result < 0)
    std::cerr << output_logs(log_context.recd_logs) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid but empty file");

  in.reset(0);
  fs::remove(filepath);
}


/** \test Attempt to parse an empty file
 */
BOOST_AUTO_TEST_CASE( parse_empty_file )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  detail::check_compliance(parser,{},{},{},{},"parse_empty_file",0);
}

/**
    \test parse the "random_data_file". The default parser should fully
    parse and produce a single field containing the whole file.
 */
BOOST_AUTO_TEST_CASE( parse_random_data_file )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

//testing... remove me
BOOST_REQUIRE(dsv_parser_set_record_delimiters(parser,"hello",5,1)==0);
BOOST_REQUIRE(dsv_parser_set_field_delimiters(parser,"m",1,1)==0);

std::vector<std::pair<std::string,std::string> > exp_seq = {
  {"open1","close1"},{"open2","close2"},{"open3","close3"},{"open4","close4"}
};


std::string open_expr1("(open_expression1)|(fo(o)?)(bar)*");
std::string close_expr1("close_expression1");
std::string open_expr2("open_expression2");
std::string close_expr2("close_expression2");
std::string open_expr3("open_expression3");
std::string close_expr3("close_expression3");

const char * open_seq[] =
  {open_expr1.data(),open_expr2.data(),open_expr3.data()};
std::size_t open_size[] =
  {open_expr1.size(),open_expr2.size(),open_expr3.size()};
int open_excl[] = {1,1,1};
const char * close_seq[] =
  {close_expr1.data(),close_expr2.data(),close_expr3.data()};
std::size_t close_size[] =
  {close_expr1.size(),close_expr2.size(),close_expr3.size()};
int close_excl[] = {1,1,1};

int iresult = dsv_parser_set_field_escape_pair(parser,
  open_seq,open_size,open_excl,close_seq,close_size,close_excl,3,0);


  dsv_operations_t operations;
  BOOST_REQUIRE(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  srand(1);

  detail::field_storage_type data;
  std::cerr << "Making datafile of " << (2<<8) << " bytes\n";
//  for(std::size_t i=0; i<(2<<16); ++i)
  for(std::size_t i=0; i<(2<<8); ++i)
    data.push_back(char(rand()%94+32));


  std::vector<detail::field_storage_type> file_contents{data};


  std::vector<std::vector<detail::field_storage_type> > records{
    {data}
  };

  fs::path filepath = detail::gen_testfile(file_contents,
    "parse_random_data_file");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  BOOST_REQUIRE_MESSAGE(in.get() != 0,
    "Unit test failure: could not open unit test data file \"" << filepath
      << "\"");


  detail::check_file_compliance(parser,{},records,{},filepath,
    "parse_random_data_file",0);


  fs::remove(filepath);
}

BOOST_AUTO_TEST_SUITE_END()
