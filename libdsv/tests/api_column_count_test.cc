#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"

#include <errno.h>
#include <stdio.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstdio>

/** \file
 *  \brief Unit tests to check handling of column count restrictions
 *
 *  Tests are against a RFC4180-strict. As of this writing, the design should not cause any reason to test the full permutation with all other settings. This may need to be revisited if the design changes significantly
 */




namespace dsv {
namespace test {


namespace fs=boost::filesystem;
namespace d=detail;

BOOST_AUTO_TEST_SUITE( api_column_count_suite )


/** CHECK DEFAULT COLUMN COUNT HANDLING **/

/** \test Check for default column behavior for square records.
 *  This should pass
 */
BOOST_AUTO_TEST_CASE( default_column_count_square )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "default_column_count_square_exact",0);
}

/** \test Check for default column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 2 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( default_column_count_less )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","3","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "default_column_count_less",-1);
}

/** \test Check for default column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 2 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( default_column_count_more )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,
      d::rfc4180_charset,d::comma,d::rfc4180_charset,detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","3","4",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "default_column_count_more",-1);
}

/** \test Check for default column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 2 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( default_column_count_zero )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","3","3","0",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "default_column_count_zero",-1);
}













/** CHECK EXACT COLUMN COUNT HANDLING **/

/** \test Check for set column behavior for square records with correct
 *  column count settings. Set to 3 records, input has 3 records. This should
 *  pass.
 */
BOOST_AUTO_TEST_CASE( exact_column_count_square )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,3);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "exact_column_count_square_exact",0);
}

/** \test Check for set column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 2 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( exact_column_count_less )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,3);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","3","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "exact_column_count_less",-1);
}

/** \test Check for set column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 4 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( exact_column_count_more )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,3);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,
      d::rfc4180_charset,d::comma,d::rfc4180_charset,detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","3","4",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "exact_column_count_more",-1);
}

/** \test Check for set column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 0 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( exact_column_count_zero )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,3);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","3","3","0",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "exact_column_count_zero",-1);
}








/** \test Check for acceptance when column count is set to permissive when
 *  square fields are given. This should pass.
 */
BOOST_AUTO_TEST_CASE( permissive_column_count_square )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,-1);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "permissive_column_count_square",0);
}

/** \test Check for permissive column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 2 records. This should
 *  pass.
 */
BOOST_AUTO_TEST_CASE( permissive_column_count_less )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,-1);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_warning,{"2","2","3","2",""}}
  };

  d::check_compliance(parser,headers,records,logs,file_contents,
    "permissive_column_count_less",0);
}

/** \test Check for permissive column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 4 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( permissive_column_count_more )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,-1);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset,
      d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,
      d::rfc4180_charset,d::comma,d::rfc4180_charset,detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","3","4",""}}
  };

  d::check_compliance(parser,headers,records,logs,file_contents,
    "permissive_column_count_more",0);
}

/** \test Check for permissive column behavior for records with incorrect
 *  column count settings. Set to 3 records, input has 0 records. This should
 *  fail.
 */
BOOST_AUTO_TEST_CASE( permissive_column_count_zero )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_parser_set_field_columns(parser,-1);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {} // valid row with no fields
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,
      detail::crlf,
    detail::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","3","3","0",""}}
  };

  d::check_compliance(parser,headers,records,logs,file_contents,
    "permissive_column_count_zero",0);
}





BOOST_AUTO_TEST_SUITE_END()


}
}
