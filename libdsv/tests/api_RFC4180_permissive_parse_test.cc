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
 *  \brief Unit tests for RFC4180-permissive parsing
 */




namespace dsv {
namespace test {


namespace fs=boost::filesystem;
namespace d=detail;

BOOST_AUTO_TEST_SUITE( RFC4180_permissive_parse_suite )


/** \test Check for default RFC4180 permissive parser object settings
 */
BOOST_AUTO_TEST_CASE( parser_default_RFC4180_permissive_object_settings )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_newline_behavior nl_behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(nl_behavior == dsv_newline_permissive,
    "Default parser newline behavior was not dsv_newline_permissive. Expected "
    << dsv_newline_permissive << " received " << nl_behavior);

  ssize_t field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == 0,
    "Default parser field columns was not '0' but rather '" << field_cols << "'");

  unsigned char delim = dsv_parser_get_field_delimiter(parser);
  BOOST_REQUIRE_MESSAGE(delim == ',',
    "Default parser delimiter was not ',' but rather '" << delim << "'");
}

/** \test Attempt to parse a unnamed file with a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_unnamed_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  int result = dsv_parse(0,0,parser,operations);
  if(result < 0)
    std::cerr << output_logs(log_context.recd_logs) << "\n";

  /**
   *  It appears that the underlying fopen call can return error codes in platform
   *  dependent ways. For example, in POSIX it seems that fopen(0,...) would return 0
   *  and set errno to ENOENT but on MAC it return EFAULT which is equally justifiable
   *  so just make sure it doesn't return 0 here.
   */
  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse attempted to open a nonexistent file and did not return nonzero");
}


/** \test Attempt to parse a named nonexistent file using a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_named_nonexistent_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

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
BOOST_AUTO_TEST_CASE( parse_rfc4180p_unnamed_empty_file_with_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  std::vector<d::field_storage_type> file_contents{
  };

  fs::path filepath = detail::gen_testfile(file_contents,
    "parse_unnamed_empty_file_with_stream");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  BOOST_REQUIRE_MESSAGE(in.get() != 0,
    "Unit test failure: could not open unit test data file \"" << filepath << "\"");

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
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty_file )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::check_compliance(parser,{},{},{},{},"parse_empty_file",0);
}


/** CHECK BASIC RECOGNITION OF THE CHARACTERSET **/

/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,{},{},file_contents,
    "parse_single_rfc4180p_charset_crlf",0);
}

/** \test Attempt to parse an named file with a single field consisting of the
 *    quoted rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_single_quoted_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(parser,headers,{},{},file_contents,
  "parse_single_quoted_rfc4180p_charset_crlf",0);
}


/** CHECK BASIC NEWLINE HANDLING **/


/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set. This file is not line terminated. If this test seems to
 *    be failing for unknown reasons, make sure viewing the test data file does not append
 *    an auto-generated newline as many text editors tend to do (as well as git if
 *    it isn't set up correctly to handle this case).
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180p_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,{},{},file_contents,
  "parse_single_rfc4180p_charset",0);
}


/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set terminated with just a linefeed. When the parser is
 *    set to be RFC4180 permissive, this should succeed.
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,detail::lf
  };

  d::check_compliance(parser,headers,{},{},file_contents,
  "parse_single_rfc4180_charset_lf",0);
}

/** CHECK INVALID FIELD HANDLING **/

/** \test Attempt to parse an named file with a single field consisting of the
 *    the quoted rfc4180 character set. The trailing quote is missing. This should
 *    case a syntax error
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_missing_trailing_quoted_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","2","98","1",""}}
  };

  // drop the null character and the trailing quote
  d::field_storage_type unterminated_quote(d::rfc4180_raw_quoted_charset.begin(),
    d::rfc4180_raw_quoted_charset.end()-1);

  std::vector<d::field_storage_type> file_contents{
    unterminated_quote
  };

  d::check_compliance(parser,{},{},logs,file_contents,
  "parse_rfc4180p_missing_trailing_quoted_charset",-1);
}


/** CHECK BASIC MULTILINE HANDLING **/

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set terminated with CRLF. This test should pass under permissive
 *    settings
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multiline_single_rfc4180p_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The lines are LF terminated and under permissive settings,
 *    this test should pass.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multiline_single_rfc4180p_charset_lf",0);
}


/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated by crlf and the second is
 *    terminated by a lf. Under newline permissive settings, this should fail.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180p_charset_crlf_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","94","95",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,logs,file_contents,
    "parse_multiline_single_rfc4180p_charset_crlf_lf",-1);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated by lf and the second is
 *    terminated by a crlf. Under newline permissive settings, this should fail.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180p_charset_lf_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","94","95",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,logs,file_contents,
    "parse_multiline_single_rfc4180p_charset_lf_crlf",-1);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated by lf and the second is
 *    unterminated. This should pass.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180p_charset_lf_unterminated )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multiline_single_rfc4180p_charset_lf_unterminated",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated by crlf and the second is
 *    unterminated. This should pass.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180p_charset_crlf_unterminated )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multiline_single_rfc4180p_charset_crlf_unterminated",0);
}















/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted rfc4180 character set terminated with CRLF. This test should pass under
 *    permissive settings
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_quoted_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multiline_single_quoted_rfc4180p_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. Although newline permissive is set, this test should fail
 *    because the newline in the quoted field sets the overall file newline.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_quoted_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","2","3",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::lf,
    d::rfc4180_raw_quoted_charset,d::lf
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multiline_single_quoted_rfc4180p_charset_lf",-1);
}


/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated by crlf and the second is
 *    terminated by a lf. Under newline permissive settings, this should fail.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_quoted_rfc4180p_charset_crlf_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_quoted_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"4","4","2","3",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,d::lf
  };

  d::check_compliance(parser,headers,records,logs,file_contents,
    "parse_multiline_single_quoted_rfc4180p_charset_crlf_lf",-1);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated by lf and the second is
 *    terminated by a crlf. Under newline permissive settings, this should fail.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_quoted_rfc4180p_charset_lf_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","2","3",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::lf,
    d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multiline_single_quoted_rfc4180p_charset_lf_crlf",-1);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted rfc4180 character set. The first line is terminated with a crlf and the
 *    second is unterminated. This should pass.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_quoted_rfc4180p_charset_crlf_unterminated )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multiline_single_quoted_rfc4180p_charset_crlf_unterminated",0);
}



/** CHECK BASIC DELIMITER HANDLING **/


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
  "parse_rfc4180p_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. This file is lifefeed terminated
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
  "parse_rfc4180p_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset,d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_quoted_charset,d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::comma,d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,d::comma,d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_quoted_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set. This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_quoted_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_lf_quoted_charset,d::rfc4180_lf_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_lf_quoted_charset,d::rfc4180_lf_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_lf_raw_quoted_charset,d::comma,d::rfc4180_lf_raw_quoted_charset,d::lf,
    d::rfc4180_lf_raw_quoted_charset,d::comma,d::rfc4180_lf_raw_quoted_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_quoted_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    mixed quoted and unquoted rfc4180p character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_mixed_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset,d::rfc4180_charset,d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_quoted_charset,d::rfc4180_charset,d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_mixed_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    mixed quoted and unquoted rfc4180p character set. This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_mixed_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_lf_quoted_charset,d::rfc4180_charset,d::rfc4180_lf_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_lf_quoted_charset,d::rfc4180_charset,d::rfc4180_lf_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_lf_raw_quoted_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_lf_raw_quoted_charset,d::lf,
    d::rfc4180_lf_raw_quoted_charset,d::comma,d::rfc4180_charset,d::comma,d::rfc4180_lf_raw_quoted_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_mixed_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. A middle fields is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty2_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::empty,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::empty,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::comma,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::comma,d::comma,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180_empty2_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. A middle fields is empty. This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty2_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::empty,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::empty,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::comma,d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::comma,d::comma,d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180_empty2_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The first field is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty1_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::empty,d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::empty,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf,
    d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty1_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The first field is empty. This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty1_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::empty,d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::empty,d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf,
    d::comma,d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty1_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last field is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty3_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::empty}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset,d::empty}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180_empty3_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last field is empty.  This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty3_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,d::empty}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset,d::empty}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::lf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::comma,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty3_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The first 2 fields are empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty12_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::empty,d::empty,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::empty,d::empty,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::comma,d::comma,d::rfc4180_charset,d::crlf,
    d::comma,d::comma,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty12_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The first 2 fields are empty. This file is linefeed terminated
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty12_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::empty,d::empty,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::empty,d::empty,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::comma,d::comma,d::rfc4180_charset,d::lf,
    d::comma,d::comma,d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty12_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last 2 fields are empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty23_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::empty,d::empty}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::empty,d::empty}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::comma,d::crlf,
    d::rfc4180_charset,d::comma,d::comma,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty23_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last 2 fields are empty. This file is linefeed terminated
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty23_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::empty,d::empty}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::empty,d::empty}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::comma,d::lf,
    d::rfc4180_charset,d::comma,d::comma,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty23_charset_lf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The header is empty. This should fail under this
 *    interpretation of RFC4180. That is, an empty header means no columns which
 *    means no columns in the record. This interpretation may change and this test case
 *    would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty_header_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {}
  };

  std::vector<d::field_storage_type> file_contents{
    d::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","2","0","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180p_empty_header_charset_crlf",-1);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The header is empty. This should fail under this
 *    interpretation of RFC4180. That is, an empty header means no columns which
 *    means no columns in the record. This interpretation may change and this test case
 *    would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty_header_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {}
  };

  std::vector<d::field_storage_type> file_contents{
    d::lf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","2","0","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180p_empty_header_charset_lf",-1);
}

/** \test Attempt to parse an named file where the header and the record is empty. This
 *    should pass under the current interpretation of RFC4180. That is, an empty header
 *    means no columns which allows for no columns in the record. This interpretation may
 *    change and this test case would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty_header_record_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {}
  };

  std::vector<d::field_storage_type> file_contents{
    d::crlf,
    d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty_header_record_charset_crlf",0);
}

/** \test Attempt to parse an named file where the header and the record is empty. This
 *    should pass under the current interpretation of RFC4180. That is, an empty header
 *    means no columns which allows for no columns in the record. This interpretation may
 *    change and this test case would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_empty_header_record_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {}
  };

  std::vector<d::field_storage_type> file_contents{
    d::lf,
    d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_rfc4180p_empty_header_record_charset_lf",0);
}

/** CHECK ADVANCED INVALID FIELD HANDLING **/


/** \test Attempt to parse a named file where the a RFC4180 charset and quoted charset
 *    are back to back. That is, no delimiter. This should cause a syntax error.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_charset_nodelimiter_rfc4180_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    //syntax error should be triggered before we see the second field
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::rfc4180_raw_quoted_charset,d::crlf,
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","1","94","95",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180p_charset_nodelimiter_rfc4180_quoted_charset_crlf",-1);
}

/** \test Attempt to parse a named file where the a RFC4180 charset and quoted charset
 *    are back to back. That is, no delimiter. This should cause a syntax error.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_charset_nodelimiter_rfc4180_quoted_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    //syntax error should be triggered before we see the second field
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::rfc4180_lf_raw_quoted_charset,d::lf,
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","1","94","95",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180p_charset_nodelimiter_rfc4180_quoted_charset_lf",-1);
}

/** \test Attempt to parse a named file where the a RFC4180 quoted charset and the charset
 *    are back to back. That is, no delimiter. This should cause a syntax error.
 *    This is the inverse of parse_rfc4180_charset_nodelimiter_rfc4180_quoted_charset_crlf
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_quoted_charset_nodelimiter_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    //syntax error should be triggered before we see the second field
    {d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::rfc4180_charset,d::crlf,
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","2","95",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180p_quoted_charset_nodelimiter_rfc4180_charset_crlf",-1);
}

/** \test Attempt to parse a named file where the a RFC4180 quoted charset and the charset
 *    are back to back. That is, no delimiter. This should cause a syntax error.
 *    This is the inverse of parse_rfc4180_charset_nodelimiter_rfc4180_quoted_charset_lf
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180p_quoted_charset_nodelimiter_rfc4180_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    //syntax error should be triggered before we see the second field
    {d::rfc4180_lf_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_lf_raw_quoted_charset,d::rfc4180_charset,d::lf,
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","2","95",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180p_quoted_charset_nodelimiter_rfc4180_charset_lf",-1);
}



/** CHECK ADVANCED MULTILINE HANDLING **/


/** \test Attempt to parse an named file with a single header and multiple records
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multirecord_rfc4180p_charset_crlf",0);
}

/** \test Attempt to parse an named file with a single header and multiple records.
 *  This file is linefeed terminated
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multirecord_rfc4180p_charset_lf",0);
}

/** \test Attempt to parse an named file with a single header and multiple records.
 *    The last line is not terminated
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180p_charset_crlf_missing )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multirecord_rfc4180p_charset_crlf_missing",0);
}

/** \test Attempt to parse an named file with a single header and multiple records.
 *    The last line is not terminated. This file is linefeed terminated
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180p_charset_lf_missing )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "parse_multirecord_rfc4180p_charset_lf_missing",0);
}



/** CHECK COLUMN COUNT ERRORS **/


/** \test Attempt to parse an named file with a single header field and multi record
 *    fields. This should cause a dsv_column_count_message under RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_single_header_multirecord_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","2","1","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_single_header_multirecord_rfc4180p_charset_crlf",-1);
}

/** \test Attempt to parse an named file with a single header field and multi record
 *    fields. This should cause a dsv_column_count_message under RFC4180 -strict.
 *    This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_single_header_multirecord_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","2","1","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_single_header_multirecord_rfc4180p_charset_lf",-1);
}

/** \test Attempt to parse an named file with a multi header field and single record
 *    fields. This should cause a dsv_column_count_message under RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_multi_header_singlerecord_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","2","2","1",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multi_header_singlerecord_rfc4180p_charset_crlf",-1);
}

/** \test Attempt to parse an named file with a multi header field and single record
 *    fields. This should cause a dsv_column_count_message under RFC4180 -strict.
 *    This file is linefeed terminated.
 */
BOOST_AUTO_TEST_CASE( parse_multi_header_singlerecord_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset,d::lf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","2","2","1",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multi_header_singlerecord_rfc4180p_charset_lf",-1);
}

/** \test Attempt to parse an named file with a multi header field and empty record
 *    fields. This should cause a dsv_column_count_message under RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_multi_header_emptyrecord_rfc4180p_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf,
    d::crlf,
    d::rfc4180_charset,d::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","3","2","0",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multi_header_emptyrecord_rfc4180p_charset_crlf",-1);
}

/** \test Attempt to parse an named file with a multi header field and empty record
 *    fields. This should cause a dsv_column_count_message under RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_multi_header_emptyrecord_rfc4180p_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_permissive(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {},
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::lf,
    d::lf,
    d::rfc4180_charset,d::lf
  };

  std::vector<detail::log_msg> logs{
    {dsv_column_count_message,dsv_log_error,{"2","3","2","0",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multi_header_emptyrecord_rfc4180p_charset_lf",-1);
}


BOOST_AUTO_TEST_SUITE_END()


}
}
