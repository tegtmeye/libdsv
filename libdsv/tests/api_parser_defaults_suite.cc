#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"



/** \file
 *  \brief Unit tests for the default settings for the different parsers
 */

namespace dsv {
namespace test {

namespace fs=boost::filesystem;
namespace d=detail;


BOOST_AUTO_TEST_SUITE( api_parser_defaults_suite )

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
  assert(dsv_parser_create(&parser) == 0);
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
BOOST_AUTO_TEST_CASE( parse_named_nonexistent_file_with_zero_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
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
BOOST_AUTO_TEST_CASE( parse_unnamed_empty_file_with_stream )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_operations_t operations;
  assert(dsv_operations_create(&operations) == 0);
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
  assert(dsv_parser_create(&parser) == 0);
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
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  fs::path filepath(detail::random_data_file);
  BOOST_REQUIRE_MESSAGE(fs::exists(filepath) && fs::is_regular_file(filepath),
    detail::missing_random_data_file_msg);

  FILE *rfile = std::fopen(filepath.c_str(),"rb");

  d::field_storage_type field;

  int c;
  while ((c = std::fgetc(rfile)) != EOF)
     field.push_back(c);

  std::vector<std::vector<d::field_storage_type> > records{
    {field}
  };

  BOOST_REQUIRE_MESSAGE(!std::ferror(rfile),
    "I/O error when reading '" << detail::random_data_file << "'");

  std::fclose(rfile);

  detail::check_file_compliance(parser,{},records,{},filepath,
    "parse_random_data_file",0);
}


BOOST_AUTO_TEST_SUITE_END()

}
}
