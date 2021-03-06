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
 *  \brief Unit tests for basic parsing
 */




namespace dsv {
namespace test {


namespace fs=boost::filesystem;
namespace d=detail;





BOOST_AUTO_TEST_SUITE( RFC4180_parse_suite )


/** \test Check for default RFC4180 parser object settings
 */
BOOST_AUTO_TEST_CASE( parser_default_RFC4180_object_settings )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_newline_behavior nl_behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(nl_behavior == dsv_newline_RFC4180_strict,
    "Default parser newline behavior was not dsv_newline_permissive. Expected "
    << dsv_newline_permissive << " received " << nl_behavior);

  ssize_t field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == 0,
    "Default parser field columns was not '0' but rather '" << field_cols
      << "'");

  unsigned char delim = dsv_parser_get_field_delimiter(parser);
  BOOST_REQUIRE_MESSAGE(delim == ',',
    "Default parser delimiter was not ',' but rather '" << delim << "'");
}


/** \test Attempt to parse a unnamed file with a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_unnamed_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
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
BOOST_AUTO_TEST_CASE( parse_rfc4180_named_nonexistent_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
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
BOOST_AUTO_TEST_CASE( parse_rfc4180_unnamed_empty_file_with_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t>
    operations_sentry(&operations,detail::operations_destroy);

  detail::logging_context log_context;
  dsv_set_logger_callback(detail::logger,&log_context,dsv_log_all,parser);

  std::vector<d::field_storage_type> file_contents{
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
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::check_compliance(parser,{},{},{},{},"parse_empty_file",0);
}



/** CHECK BASIC RECOGNITION OF THE CHARACTERSET **/

/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,{},{},file_contents,
    "parse_single_rfc4180_charset_crlf",0);
}

/** \test Attempt to parse an named file with a the invalid characters of the
 *    ASCII character set
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180_invalid_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","1","1","2",""}}
  };

  // check the first 32 non-printing characters
  for(std::size_t i=0; i<32; ++i) {
    std::vector<d::field_storage_type> file_contents{
      {static_cast<unsigned char>(i)},d::crlf
    };

    d::check_compliance(parser,{},{},logs,file_contents,
      "parse_single_rfc4180_charset_crlf",-1);
  }

  for(std::size_t i=127; i<256; ++i) {
    std::vector<d::field_storage_type> file_contents{
      {static_cast<unsigned char>(i)},d::crlf
    };

    d::check_compliance(parser,{},{},logs,file_contents,
      "parse_single_rfc4180_charset_crlf",-1);
  }


}


/** \test Attempt to parse an named file with a single field consisting of the
 *    quoted rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_single_quoted_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(parser,headers,{},{},file_contents,
  "parse_single_quoted_rfc4180_charset_crlf",0);
}


/** \test Attempt to parse an named file with a single field consisting of the
 *    quoted rfc4180 character set with a invalid embedded LF.
 *
 *  RFC4180 state the following:
 *    Fields containing line breaks (CRLF), double quotes, and commas
 *    should be enclosed in double-quotes.
 *
 *  Therefore a since LF in a quoted field in RFC4180-strict is considered an error
 */
BOOST_AUTO_TEST_CASE( parse_single_quoted_rfc4180_lf_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<detail::log_msg> logs{
    {dsv_unexpected_binary,dsv_log_error,{"1","1","98","99","0x0a",""}}
  };

  // remove the leading embedded CR in the quoted field
  d::field_storage_type field = d::rfc4180_raw_quoted_charset;
  field.erase(field.end()-3);

  std::vector<d::field_storage_type> file_contents{
    field,d::crlf
  };

  d::check_compliance(parser,{},{},logs,file_contents,
  "parse_single_quoted_rfc4180_lf_charset_crlf",-1);
}






/** CHECK BASIC NEWLINE HANDLING **/


/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set. This file is not line terminated. If this test seems to
 *    be failing for unknown reasons, make sure viewing the test data file does not append
 *    an auto-generated newline as many text editors tend to do (as well as git if
 *    it isn't set up correctly to handle this case).
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,{},{},file_contents,
  "parse_single_rfc4180_charset",0);
}



/** CHECK INVALID FIELD HANDLING **/

/** \test Attempt to parse an named file with a single field consisting of the
 *    the quoted rfc4180 character set. The trailing quote is missing. This should
 *    case a syntax error
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_missing_trailing_quoted_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
  "parse_rfc4180_missing_trailing_quoted_charset",-1);
}

/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set terminated with just a linefeed. When the parser is
 *    set to be RFC4180 strict, this is an error.
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","1","94","95",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,detail::lf
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
  "parse_single_rfc4180_charset_lf",-1);
}




/** CHECK BASIC MULTILINE HANDLING **/

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_multiline_single_rfc4180_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted_rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_multiline_single_rfc4180_quoted_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The last line is not terminated.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_multiline_single_rfc4180_charset",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted_rfc4180 character set. The last line is not terminated.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_quoted_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_multiline_single_rfc4180_quoted_charset",0);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The first line is terminated with a LF. When
 *    set to be RFC4180 strict, this is an error.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset}
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","1","94","95",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,d::lf,
    d::rfc4180_charset
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
  "parse_multiline_single_rfc4180_charset_lf",-1);
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted rfc4180 character set. The first line is terminated with a LF. Then
 *    set to be RFC4180 strict, this is an error.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_quoted_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_quoted_charset}
  };

  // the crlf in the quoted field increases the line count
  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"2","2","2","3",""}}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_raw_quoted_charset,d::lf,
    d::rfc4180_raw_quoted_charset
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
  "parse_multiline_single_rfc4180_quoted_charset_lf",-1);
}






/** CHECK BASIC DELIMITER HANDLING **/


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
  "parse_rfc4180_charset_crlf",0);
}

/** \test Parse a named file with multiple fields in both headers and records
  *   each with different field lengths. This should pass. This address a bug in
  *   v0.1.0 where the records field lengths were not getting reported properly.
 */
BOOST_AUTO_TEST_CASE( parse_mixed_field_length_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::field_storage_type h1 = {'0','1','2','3','4','5','6','7','8','9'};
  d::field_storage_type h2 = {
    '0','1','2','3','4','5','6','7',
    '8','9','8','7','6','5','4','3','2','1','0'
  };
  d::field_storage_type r1 = {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s',
    't','u','v','w','x','y','z'
  };

  std::vector<std::vector<d::field_storage_type> > headers{
    {h1,h2}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {r1,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    h1,d::comma,h2,d::crlf,
    r1,d::comma,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
  "parse_mixed_field_length_crlf",0);
}


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_quoted_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    mixed quoted and unquoted rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_mixed_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_mixed_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. A middle fields is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty2_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
 *    rfc4180 character set. The first field is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty1_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_empty1_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last field is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty3_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
 *    rfc4180 character set. The first 2 fields are empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty12_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_empty12_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last 2 fields are empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty23_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_empty23_charset_crlf",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The header is empty. This should fail under this
 *    interpretation of RFC4180. That is, an empty header means no columns which
 *    means no columns in the record. This interpretation may change and this test case
 *    would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty_header_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<std::vector<d::field_storage_type> > headers{
    {}
  };

  std::vector<d::field_storage_type> file_contents{
    d::crlf,
    d::rfc4180_charset,d::comma,d::rfc4180_charset,d::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","0","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_rfc4180_empty_header_charset_crlf",-1);
}

/** \test Attempt to parse an named file where the header and the record is empty. This
 *    should pass under the current interpretation of RFC4180. That is, an empty header
 *    means no columns which allows for no columns in the record. This interpretation may
 *    change and this test case would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty_header_record_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_empty_header_record_charset_crlf",0);
}



/** CHECK ADVANCED INVALID FIELD HANDLING **/


/** \test Attempt to parse a named file where the a RFC4180 charset and quoted charset
 *    are back to back. That is, no delimiter. This should cause a syntax error.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_charset_nodelimiter_rfc4180_quoted_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_charset_nodelimiter_rfc4180_quoted_charset_crlf",-1);
}

/** \test Attempt to parse a named file where the a RFC4180 quoted charset and the charset
 *    are back to back. That is, no delimiter. This should cause a syntax error.
 *    This is the inverse of parse_rfc4180_charset_nodelimiter_rfc4180_quoted_charset_crlf
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_quoted_charset_nodelimiter_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_rfc4180_quoted_charset_nodelimiter_rfc4180_charset_crlf",-1);
}





/** CHECK ADVANCED MULTILINE HANDLING **/


/** \test Attempt to parse an named file with a single header and multiple records
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_multirecord_rfc4180_charset_crlf",0);
}

/** \test Attempt to parse an named file with a single header and multiple records.
 *    The last line is not terminated
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180_charset )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    "parse_multirecord_rfc4180_charset",0);
}



/** CHECK COLUMN COUNT ERRORS **/


/** \test Attempt to parse an named file with a single header field and
 *    multi record fields. This should cause a dsv_inconsistant_column_count
 *    under RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_single_header_multirecord_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","1","2",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_single_header_multirecord_rfc4180_charset_crlf",-1);
}

/** \test Attempt to parse an named file with a multi header field and single
 *    record fields. This should cause a dsv_inconsistant_column_count under
 *    RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_multi_header_singlerecord_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    {dsv_inconsistant_column_count,dsv_log_error,{"2","2","2","1",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multi_header_singlerecord_rfc4180_charset_crlf",-1);
}

/** \test Attempt to parse an named file with a multi header field and empty
 *    record fields. This should cause a dsv_inconsistant_column_count under
 *    RFC4180 -strict
 */
BOOST_AUTO_TEST_CASE( parse_multi_header_emptyrecord_rfc4180_charset_crlf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
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
    {dsv_inconsistant_column_count,dsv_log_error,{"2","3","2","0",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "parse_multi_header_singlerecord_rfc4180_charset_crlf",-1);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
