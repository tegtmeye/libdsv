#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"

#include <errno.h>

#include <string>
#include <sstream>
#include <memory>
#include <cstdio>

/** \file
 *  \brief Unit tests the scanner
 */




namespace dsv {
namespace test {


namespace fs=boost::filesystem;
namespace d=detail;


BOOST_AUTO_TEST_SUITE( alt_delimiter_test_suite )


/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set separated by a ASCII multibyte delimiter
 */
BOOST_AUTO_TEST_CASE( check_single_multibyte_ascii_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::field_storage_type multibyte_delimiter = {'<','!','>'};

  int err = dsv_parser_set_field_wdelimiter(parser,multibyte_delimiter.data(),
    multibyte_delimiter.size());
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,multibyte_delimiter,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,multibyte_delimiter,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_multibyte_ascii_delimiter_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set separated by a non-ASCII multibyte delimiter
 *    representing the German ess
 */
BOOST_AUTO_TEST_CASE( check_single_multibyte_nonascii_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::field_storage_type multibyte_delimiter = {0xC3,0x9F};

  int err = dsv_parser_set_field_wdelimiter(parser,multibyte_delimiter.data(),
    multibyte_delimiter.size());
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset,d::rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::rfc4180_charset,d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,multibyte_delimiter,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,multibyte_delimiter,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_multibyte_ascii_delimiter_test",0);
}


// INCOMPLETE DELIMITER TEST

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set separated by a ASCII multibyte delimiter
 */
BOOST_AUTO_TEST_CASE( check_single_multibyte_ascii_partial_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::field_storage_type multibyte_delimiter = {'<','!','>'};

  int err = dsv_parser_set_field_wdelimiter(parser,multibyte_delimiter.data(),
    multibyte_delimiter.size());
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  d::field_storage_type partial_delimiter = {'<','!'};

  d::field_storage_type invalid_field = d::rfc4180_charset;
  invalid_field.insert(invalid_field.end(),partial_delimiter.begin(),
    partial_delimiter.end());
  invalid_field.insert(invalid_field.end(),d::rfc4180_charset.begin(),
    d::rfc4180_charset.end());

  std::vector<std::vector<d::field_storage_type> > headers{
    {invalid_field}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {invalid_field}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,partial_delimiter,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,partial_delimiter,d::rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_multibyte_ascii_partial_delimiter_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of the
 *    rfc4180 character set separated by a non-ASCII multibyte delimiter
 */
BOOST_AUTO_TEST_CASE( check_single_multibyte_nonascii_partial_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  d::field_storage_type multibyte_delimiter = {0xC3,0x9F};

  int err = dsv_parser_set_field_wdelimiter(parser,multibyte_delimiter.data(),
    multibyte_delimiter.size());
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  d::field_storage_type partial_delimiter = {0xC3};

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::rfc4180_charset}
  };

  std::vector<d::field_storage_type> file_contents{
    d::rfc4180_charset,partial_delimiter,d::rfc4180_charset,d::crlf,
    d::rfc4180_charset,partial_delimiter,d::rfc4180_charset,d::crlf
  };

  std::vector<detail::log_msg> logs{
    {dsv_syntax_error,dsv_log_error,{"1","1","94","95",""}}
  };

  d::check_compliance(parser,headers,{},logs,file_contents,
    "check_single_multibyte_nonascii_partial_delimiter_test",-1);
}


// TEST FOR SIMPLE EQUIVALENT DELIMITERS

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    single char separated by the first ASCII whitespace delimiters
 */
BOOST_AUTO_TEST_CASE( check_single_first_ascii_equiv_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,d::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::ws_delimiters,
    d::ws_delimiters_sizes,repeats,d::num_ws_delimiters,0,0);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'a'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'a'},{'b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'a'},{' '},{'b'},d::crlf,
    {'a'},{' '},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_first_ascii_equiv_delimiter_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    single char separated by the second ASCII whitespace delimiters
 */
BOOST_AUTO_TEST_CASE( check_single_second_ascii_equiv_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::ws_delimiters,
    d::ws_delimiters_sizes,repeats,d::num_ws_delimiters,0,0);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'a'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'a'},{'b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'a'},{'\t'},{'b'},d::crlf,
    {'a'},{'\t'},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_second_ascii_equiv_delimiter_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    single char separated by a mix ASCII whitespace delimiters
 */
BOOST_AUTO_TEST_CASE( check_single_mix_ascii_equiv_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::ws_delimiters,
    d::ws_delimiters_sizes,repeats,d::num_ws_delimiters,0,0);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'a'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'a'},{'b'}},
    {{'c'},{'d'}},
    {{'e'},{'f'}},
    {{'g'},{'h'}},
    {{'i'},{'j'}},
    {{'k'},{'l'}},
  };

  std::vector<d::field_storage_type> file_contents{
    {'a'},{' '},{'b'},d::crlf,
    {'a'},{'\t'},{'b'},d::crlf,
    {'c'},{'\t'},{'d'},d::crlf,
    {'e'},{' '},{'f'},d::crlf,
    {'g'},{' '},{'h'},d::crlf,
    {'i'},{'\t'},{'j'},d::crlf,
    {'k'},{' '},{'l'},d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_mix_ascii_equiv_delimiter_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    subset of the RFC4180 charset separated by a mix ASCII whitespace
 *    delimiters
 */
BOOST_AUTO_TEST_CASE( check_sub_RFC4180_mix_ascii_equiv_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::ws_delimiters,
    d::ws_delimiters_sizes,repeats,d::num_ws_delimiters,0,0);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset},
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset},
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset},
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset},
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset},
    {d::sub_rfc4180_charset,d::sub_rfc4180_charset},
  };

  std::vector<d::field_storage_type> file_contents{
    d::sub_rfc4180_charset,{' '},d::sub_rfc4180_charset,d::crlf,
    d::sub_rfc4180_charset,{'\t'},d::sub_rfc4180_charset,d::crlf,
    d::sub_rfc4180_charset,{'\t'},d::sub_rfc4180_charset,d::crlf,
    d::sub_rfc4180_charset,{' '},d::sub_rfc4180_charset,d::crlf,
    d::sub_rfc4180_charset,{' '},d::sub_rfc4180_charset,d::crlf,
    d::sub_rfc4180_charset,{'\t'},d::sub_rfc4180_charset,d::crlf,
    d::sub_rfc4180_charset,{' '},d::sub_rfc4180_charset,d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_sub_RFC4180_mix_ascii_equiv_delimiter_test",0);
}


// TEST PARSE EXCLUSIVITY

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    single char separated with parse exclusivity ignoring second
 */
BOOST_AUTO_TEST_CASE( check_single_exclusivity_ignore_second_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::ascii_delimiters,
    d::ascii_delimiters_sizes,repeats,d::num_ascii_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'f'},{'b','_','b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{' '},{'b'},d::crlf,
    {'f'},{' '},{'b'},{'_'},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_exclusivity_ignore_second_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    single char separated with parse exclusivity ignoring first
 */
BOOST_AUTO_TEST_CASE( check_single_exclusivity_ignore_first_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::ascii_delimiters,
    d::ascii_delimiters_sizes,repeats,d::num_ascii_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'f'},{'b',' ','b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{'_'},{'b'},d::crlf,
    {'f'},{'_'},{'b'},{' '},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_single_exclusivity_ignore_first_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    multibyte char separated with parse exclusivity ignoring second
 */
BOOST_AUTO_TEST_CASE( check_multi_exclusivity_ignore_second_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::seq_delimiters,
    d::seq_delimiters_sizes,repeats,d::num_seq_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'f'},{'b','/','?','\\','b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{'<','!','>'},{'b'},d::crlf,
    {'f'},{'<','!','>'},{'b'},{'/','?','\\'},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_multi_exclusivity_ignore_second_test",0);
}

/** \test Attempt to parse an named file with multiple fields consisting of a
 *    multibyte char separated with parse exclusivity ignoring first
 */
BOOST_AUTO_TEST_CASE( check_multi_exclusivity_ignore_first_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::seq_delimiters,
    d::seq_delimiters_sizes,repeats,d::num_seq_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'f'},{'b','<','!','>','b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{'/','?','\\'},{'b'},d::crlf,
    {'f'},{'/','?','\\'},{'b'},{'<','!','>'},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_multi_exclusivity_ignore_first_test",0);
}

// CHECK FOR MULTIPLE MULTIBYTE DELIMITER HANDLING

/** \test Attempt to parse an named file with multibyte delimiters in series
 */
BOOST_AUTO_TEST_CASE( check_multi_empty_multibyte_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::seq_delimiters,
    d::seq_delimiters_sizes,repeats,d::num_seq_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{},{'f'},{},{'b'},{}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{},{'f'},{},{'b'},{}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'/','?','\\'},{'f'},{'/','?','\\'},{'/','?','\\'},{'b'},{'/','?','\\'},
      d::crlf,
    {'/','?','\\'},{'f'},{'/','?','\\'},{'/','?','\\'},{'b'},{'/','?','\\'},
      d::crlf
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_multi_empty_multibyte_delimiter_test",0);
}

// CHECK FOR PALINDROME PARTIAL DELIMITERS

/** \test Attempt to parse an named file with full and partial multibyte
    delimiters in series
 */
BOOST_AUTO_TEST_CASE( check_multi_partial_full_multibyte_delimiter_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::pal_delimiters,
    d::pal_delimiters_sizes,repeats,d::num_pal_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'*','!','!'},{'b'}}
  };
  std::vector<std::vector<d::field_storage_type> > records{
    {{'f'},{'!','*','*','!','!'},{'b'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{'!','*','!'},{'*','!','!'},{'!','*','!'},{'b'},d::crlf,
    {'f'},{'!','*','!'},{'!','*','*','!','!'},{'!','*','!'},{'b'},d::crlf,
  };

  d::check_compliance(parser,headers,records,{},file_contents,
    "check_multi_partial_full_multibyte_delimiter_test",0);
}


// CHECK FOR MALFORMED FILES CONTAINING MULTIBYTE DELIMITERS

/** \test Attempt to parse an named file with a multibyte delimiter that ends
    unexpectedly
 */
BOOST_AUTO_TEST_CASE( check_multi_malformed_final_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int repeats[] = {0,0};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,d::seq_delimiters,
    d::seq_delimiters_sizes,repeats,d::num_seq_delimiters,0,1);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'b','/','?'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{'/','?','\\'},{'b','/','?'}
  };

  d::check_compliance(parser,headers,{},{},file_contents,
    "check_multi_malformed_final_test",0);
}


// CHECK FOR PER-DELIMITER REPEAT

/** \test Attempt to parse an named file with repeated single delimiter
 */
BOOST_AUTO_TEST_CASE( check_single_delimiter_repeat_test )
{
  dsv_parser_t parser;
  assert(dsv_parser_create_RFC4180_strict(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  const unsigned char *space_delimiters[] = {&d::space};
  const std::size_t space_delimiters_sizes[] = {1};
  const std::size_t num_space_delimiters = 1;
  int repeats[] = {1};

  int err = dsv_parser_set_field_wdelimiter_equiv(parser,space_delimiters,
    space_delimiters_sizes,repeats,num_space_delimiters,0,0);
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
      << " for multibyte delimiter");

  std::vector<std::vector<d::field_storage_type> > headers{
    {{'f'},{'b'}}
  };

  std::vector<std::vector<d::field_storage_type> > records{
    {{'a'},{'c'}}
  };

  std::vector<d::field_storage_type> file_contents{
    {'f'},{' ',' '},{'b'},d::crlf,
//     {'a'},{' ',' ',' ',' ',' ',' ',' '},{'c'},d::crlf
  };

//   d::check_compliance(parser,headers,records,{},file_contents,
  d::check_compliance(parser,headers,{},{},file_contents,
    "check_single_delimiter_repeat_test",0);
}



// need to check for binary case for tab



BOOST_AUTO_TEST_SUITE_END()

}
}
