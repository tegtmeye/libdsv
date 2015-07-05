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




/** \test Attempt to parse a unnamed file with a zero stream
 */
BOOST_AUTO_TEST_CASE( parse_unnamed_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;
  int result;
  if((result = dsv_parse(0,0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

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
BOOST_AUTO_TEST_CASE( parse_named_nonexistent_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  detail::field_context context;

  int result;
  if((result = dsv_parse("nonexistant_file.dsv",0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == ENOENT,
    "dsv_parse attempted to open a nonexistent file and did not return ENOENT");
}

/** \test Attempt to parse an unnamed empty file opened with a stream
 */
BOOST_AUTO_TEST_CASE( parse_unnamed_empty_file_with_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::vector<std::string> file_contents{
  };

  fs::path filepath = detail::gen_testfile(file_contents,"parse_unnamed_empty_file_with_stream");

  std::unique_ptr<std::FILE,int(*)(std::FILE *)> in(std::fopen(filepath.c_str(),"rb"),&std::fclose);
  BOOST_REQUIRE_MESSAGE(in.get() != 0,
    "Unit test failure: could not open unit test data file \"" << filepath << "\"");

  int result;
  if((result = dsv_parse(0,in.get(),parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result == 0,
    "dsv_parse failed for a valid but empty file");

  in.reset(0);
  fs::remove(filepath);
}

/** \test Attempt to parse an empty file
 */
BOOST_AUTO_TEST_CASE( parse_empty_file )
{
  std::vector<std::vector<std::string> > headers{
  };
  std::vector<std::vector<std::string> > records{
  };

  std::vector<std::string> file_contents{
  };

  d::check_compliance(headers,records,file_contents,"parse_empty_file");
}



/** CHECK BASIC RECOGNITION OF THE CHARACTERSET **/

/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_single_rfc4180_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_single_rfc4180_charset_crlf");
}

/** \test Attempt to parse an named file with a single field consisting of the
 *    quoted rfc4180 character set
 */
BOOST_AUTO_TEST_CASE( parse_single_quoted_rfc4180_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<std::string> > records{
  };

  std::vector<std::string> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_single_quoted_rfc4180_charset_crlf");
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
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset
  };

  d::check_compliance(headers,records,file_contents,"parse_single_rfc4180_charset");
}

#if 0
// todo, update this when logging gets worked out
/** \test Attempt to parse an named file with a single field consisting of the
 *    the rfc4180 character set terminated with just a linefeed. When the parser is
 *    set to be RFC4180 strict, this is an error.
 */
BOOST_AUTO_TEST_CASE( parse_named_single_field_rfc4180_charset_lf )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // set RFC4180 strict
  dsv_parser_set_newline_handling(parser,dsv_newline_RFC4180_strict);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
  std::shared_ptr<dsv_operations_t> operations_sentry(&operations,detail::operations_destroy);

  std::vector<std::vector<std::string> > header_matrix;
  dsv_set_header_callback(detail::fill_matrix,&header_matrix,operations);

  detail::field_context record_context;
  dsv_set_record_callback(detail::field_callback,&record_context,operations);

  std::string filename(detail::testdatadir+"/single_field_rfc4180_charset_lf.dsv");

  int result;
  if((result = dsv_parse(filename.c_str(),0,parser,operations)))
    std::cerr << detail::msg_log(parser,dsv_log_all) << "\n";

  BOOST_REQUIRE_MESSAGE(result != 0,
    "dsv_parse incorrectly accepted a non-quoted linefeed when set to be RFC4180-strict: " 
      << filename << ". Received HEADER\n" << detail::print_matrix(header_matrix));

  dsv_log_code codes[] = {
    dsv_syntax_error
  };
  
  BOOST_REQUIRE_MESSAGE(detail::msg_log_check(parser,
    std::vector<dsv_log_code>(codes,codes+sizeof(codes)/sizeof(dsv_log_code))),
    "dsv_parse did not log the correct number or type of msgs");
}
#endif



/** CHECK BASIC MULTILINE HANDLING **/

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_multiline_single_rfc4180_charset_crlf");
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted_rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_quoted_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_quoted_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_multiline_single_rfc4180_quoted_charset_crlf");
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    rfc4180 character set. The last line is not terminated.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_charset )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset
  };

  d::check_compliance(headers,records,file_contents,"parse_multiline_single_rfc4180_charset");
}

/** \test Attempt to parse an named file with multiple lines consisting of the
 *    quoted_rfc4180 character set. The last line is not terminated.
 */
BOOST_AUTO_TEST_CASE( parse_multiline_single_rfc4180_quoted_charset )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_quoted_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset
  };

  d::check_compliance(headers,records,file_contents,"parse_multiline_single_rfc4180_quoted_charset");
}



/** CHECK BASIC DELIMITER HANDLING **/


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,",",d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,",",d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    quoted rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_quoted_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_quoted_charset,d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_quoted_charset,d::rfc4180_quoted_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_raw_quoted_charset,",",d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,",",d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_quoted_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    mixed quoted and unquoted rfc4180 character set.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_mixed_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_quoted_charset,d::rfc4180_charset,d::rfc4180_quoted_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_quoted_charset,d::rfc4180_charset,d::rfc4180_quoted_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_raw_quoted_charset,",",d::rfc4180_charset,",",d::rfc4180_raw_quoted_charset,d::crlf,
    d::rfc4180_raw_quoted_charset,",",d::rfc4180_charset,",",d::rfc4180_raw_quoted_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_mixed_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. A middle fields is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty2_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset,"",d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset,"",d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,",",",",d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,",",",",d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty2_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The first field is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty1_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {"",d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {"",d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    ",",d::rfc4180_charset,",",d::rfc4180_charset,d::crlf,
    ",",d::rfc4180_charset,",",d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty1_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last field is empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty3_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset,d::rfc4180_charset,""}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset,d::rfc4180_charset,""}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,",",d::rfc4180_charset,",",d::crlf,
    d::rfc4180_charset,",",d::rfc4180_charset,",",d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty3_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The first 2 fields are empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty12_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {"","",d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {"","",d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    ",,",d::rfc4180_charset,d::crlf,
    ",,",d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty12_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The last 2 fields are empty
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty23_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset,"",""}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset,"",""}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,",,",d::crlf,
    d::rfc4180_charset,",,",d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty23_charset_crlf");
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set. The header is empty. This should pass under the current
 *    interpretation of RFC4180. That is, an empty header means no columns which doesn't
 *    mean no columns in the record. This interpretation may change and this test case
 *    would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty_header_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::crlf,
    d::rfc4180_charset,",",d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty_header_charset_crlf");
}

// TODO need empty record test case. add when new test framework for noncompliance gets
// built


/** \test Attempt to parse an named file where the header and the record is empty. This
 *    should pass under the current interpretation of RFC4180. That is, an empty header
 *    means no columns which allows for no columns in the record. This interpretation may
 *    change and this test case would then need to be updated accordingly.
 */
BOOST_AUTO_TEST_CASE( parse_rfc4180_empty_header_record_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {}
  };
  std::vector<std::vector<std::string> > records{
    {}
  };

  std::vector<std::string> file_contents{
    d::crlf,
    d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_rfc4180_empty_header_record_charset_crlf");
}



/** CHECK ADVANCED MULTILINE HANDLING **/


/** \test Attempt to parse an named file with a single header and multiple records
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180_charset_crlf )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf
  };

  d::check_compliance(headers,records,file_contents,"parse_multirecord_rfc4180_charset_crlf");
}

/** \test Attempt to parse an named file with a single header and multiple records.
 *    The last line is not terminated
 */
BOOST_AUTO_TEST_CASE( parse_multirecord_rfc4180_charset )
{
  std::vector<std::vector<std::string> > headers{
    {d::rfc4180_charset}
  };
  std::vector<std::vector<std::string> > records{
    {d::rfc4180_charset},
    {d::rfc4180_charset},
    {d::rfc4180_charset}
  };

  std::vector<std::string> file_contents{
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,d::crlf,
    d::rfc4180_charset
  };

  d::check_compliance(headers,records,file_contents,"parse_multirecord_rfc4180_charset");
}










































BOOST_AUTO_TEST_SUITE_END()

}
}
