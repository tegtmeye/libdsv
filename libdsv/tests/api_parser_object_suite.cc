#include <boost/test/unit_test.hpp>

#include <dsv_parser.h>
#include "test_detail.h"


#include <algorithm>
#include <limits>

/** \file
 *  \brief Unit tests for parser creation
 */

namespace dsv {
namespace test {



BOOST_AUTO_TEST_SUITE( api_parser_object_suite )

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
BOOST_AUTO_TEST_CASE( set_equiv_record_delimiters_byte_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char bytesequence[] = {'\n'};
  const unsigned char *equiv_byteseq[] = {bytesequence};
  size_t byteseq_size[] = {1};
  int byteseq_repeat[] = {0};
  size_t size = 1;
  int repeatflag = 0;
  int exclusiveflag = 0;

  int iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  size_t sresult = dsv_parser_num_equiv_record_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == size,
    "number of record delimiters " << sresult << " != " << size);

  iresult = dsv_parser_get_equiv_record_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == repeatflag,
    "record delimiters repeatflag " << iresult << " != " << repeatflag);

  iresult = dsv_parser_get_equiv_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == exclusiveflag,
    "record delimiters exclusiveflag " << iresult << " != " << exclusiveflag);

  // get bytesequence size
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  // get bytesequence size with repeatflag
  int flag = 42;

  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);


  // get invalid bytesequence size
  sresult = dsv_parser_get_equiv_record_delimiter(parser,size,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "unexpected bytesequence size " << sresult << " != " << 0);

  static const size_t bufsize = 5;
  unsigned char buf[bufsize];
  unsigned char sentry_buf[bufsize] = {'X','X','X','X','X'};
  unsigned char check_buf[bufsize] = {bytesequence[0],'X','X','X','X'};

  // check for returned bytesequence of exact buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,1,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of larger buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,bufsize,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of zero buff size and valid repeatflag
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,sentry_buf));

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);
}


BOOST_AUTO_TEST_CASE( set_equiv_record_delimiters_multibyte_check )
{
  // these are declared here for convenience of adding/removing new tests
  // without worrying about the variable declaration
  int iresult;
  size_t sresult;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char bytesequence[] = {'\n',0xFF,'h','e','l','l','o'};
  const unsigned char *equiv_byteseq[] = {bytesequence};
  size_t byteseq_size[] = {7};
  int byteseq_repeat[] = {0};
  size_t size = 1;
  int repeatflag = 0;
  int exclusiveflag = 0;

  // check for pathological equiv_bytesequence size
  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,
      std::numeric_limits<size_t>::max(),0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for 0 equiv_bytesequence size
  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,0,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for pathological individual bytesequece size
  size_t path_byteseq_size[] = {std::numeric_limits<size_t>::max()};
  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,path_byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for 0 individual bytesequece size
  size_t zero_byteseq_size[] = {0};
  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,zero_byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult);

  sresult = dsv_parser_num_equiv_record_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == size,
    "number of record delimiters " << sresult << " != " << size);

  iresult = dsv_parser_get_equiv_record_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == repeatflag,
    "record delimiters repeatflag " << iresult << " != " << repeatflag);

  iresult = dsv_parser_get_equiv_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == exclusiveflag,
    "record delimiters exclusiveflag " << iresult << " != " << exclusiveflag);

  // get bytesequence size
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  // get bytesequence size with repeatflag
  int flag = 42;

  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);


  // get invalid bytesequence size
  sresult = dsv_parser_get_equiv_record_delimiter(parser,size,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "unexpected bytesequence size " << sresult << " != " << 0);

  static const size_t bufsize = 14;
  unsigned char buf[bufsize];
  unsigned char sentry_buf[bufsize];
  std::fill(sentry_buf,sentry_buf+bufsize,'X');

  unsigned char check_buf[bufsize];
  std::fill(std::copy(bytesequence,bytesequence+byteseq_size[0],
      check_buf),check_buf+bufsize,'X');

  // check for returned bytesequence of exact buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,
    byteseq_size[0],0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of larger buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,bufsize,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of zero buff size and valid repeatflag
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,sentry_buf));

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);
}

// schedule this after the getting/setting tests
BOOST_AUTO_TEST_CASE( set_equiv_record_delimiters_default_check )
{
  int iresult;
  size_t sresult;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // check for default record delimiter (CRLF)
  sresult = dsv_parser_num_equiv_record_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == 1,
    "default number of record delimiters " << sresult << " != 1");

  iresult = dsv_parser_get_equiv_record_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "default record delimiters repeatflag " << iresult << " != 0");

  iresult = dsv_parser_get_equiv_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "default record delimiters exclusiveflag " << iresult << " != 1");

  // get bytesequence size
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 2,
    "default bytesequence size " << sresult << " != 2");

  static const size_t default_byteseq_size = 2;
  unsigned char buf[default_byteseq_size];
  unsigned char default_bytesequence[default_byteseq_size] = {0x0D,0x0A};
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,buf,
    default_byteseq_size,0);

  BOOST_REQUIRE_MESSAGE(sresult == default_byteseq_size,
    "unexpected default bytesequence size " << sresult << " != "
      << default_byteseq_size);

  BOOST_REQUIRE(std::equal(buf,buf+default_byteseq_size,default_bytesequence));
}

BOOST_AUTO_TEST_CASE( set_equiv_record_delimiters_multi_multibyte_check )
{
  // these are declared here for convenience of adding/removing new tests
  // without worrying about the variable declaration
  int iresult;
  size_t sresult;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char bytesequence1[] = {'\n',0xFF,'h','e','l','l','o'};
  unsigned char bytesequence2[] = {0x0D};
  unsigned char bytesequence3[] = {'w','o','r','l','d',0x0A};
  const unsigned char *equiv_byteseq[] = {bytesequence1,bytesequence2,
    bytesequence3};
  size_t byteseq_size[] = {7,1,6};
  int byteseq_repeat[] = {0,1,0};
  size_t size = 3;
  int repeatflag = 0;
  int exclusiveflag = 0;

  // check for pathological individual bytesequece size
  size_t path_byteseq_size[] = {7,std::numeric_limits<size_t>::max(),6};
  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,path_byteseq_size,byteseq_repeat,3,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for 0 individual bytesequece size
  size_t zero_byteseq_size[] = {7,1,0};
  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,zero_byteseq_size,byteseq_repeat,3,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_equiv_record_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,size,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult);

  sresult = dsv_parser_num_equiv_record_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == size,
    "number of record delimiters " << sresult << " != " << size);

  iresult = dsv_parser_get_equiv_record_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == repeatflag,
    "record delimiters repeatflag " << iresult << " != " << repeatflag);

  iresult = dsv_parser_get_equiv_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == exclusiveflag,
    "record delimiters exclusiveflag " << iresult << " != " << exclusiveflag);

  for(std::size_t i=0; i<size; ++i) {
    // check bytesequence size
    sresult = dsv_parser_get_equiv_record_delimiter(parser,i,0,0,0);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "bytesequence size " << sresult << " != " << byteseq_size[i]);

    // check bytesequence size with repeatflag
    int flag = 42;
    sresult = dsv_parser_get_equiv_record_delimiter(parser,i,0,0,&flag);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "bytesequence size " << sresult << " != " << byteseq_size[i]);

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);

    std::size_t padsize = 5;
    std::size_t buffsize = byteseq_size[i]+(2*padsize);
    std::unique_ptr<unsigned char[]> buf(new unsigned char[buffsize]);
    std::unique_ptr<unsigned char[]> check_buf(new unsigned char[buffsize]);
    std::fill(buf.get(),buf.get()+buffsize,0xFF);
    std::fill(check_buf.get(),check_buf.get()+buffsize,0xFF);
    std::copy(equiv_byteseq[i],equiv_byteseq[i]+byteseq_size[i],
      check_buf.get()+padsize);

    // check for returned bytesequence of exact buff size
    // check flag again since library is actually filling the buffer
    flag = 42;
    sresult = dsv_parser_get_equiv_record_delimiter(parser,i,buf.get()+padsize,
      byteseq_size[i],&flag);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "unexpected bytesequence size " << sresult << " != " << byteseq_size[i]);

    BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);

    //reset buffer
    std::fill(buf.get(),buf.get()+buffsize,0xFF);

    // check for returned bytesequence of larger buff size
    // check flag again since library is actually filling the buffer
    flag = 42;
    sresult = dsv_parser_get_equiv_record_delimiter(parser,i,buf.get()+padsize,
      byteseq_size[i]+padsize,&flag);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "unexpected bytesequence size " << sresult << " != " << byteseq_size[i]);

    BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);

    //reset buffer
    std::fill(buf.get(),buf.get()+buffsize,0xFF);
    //adjust check buffer
    std::size_t undersize = std::max((byteseq_size[i]/2),1ul);
    std::fill(check_buf.get(),check_buf.get()+buffsize,0xFF);
    std::copy(equiv_byteseq[i],equiv_byteseq[i]+undersize,
      check_buf.get()+padsize);

    // check for returned bytesequence of undersized buff size
    // check flag again since library is actually filling the buffer
    flag = 42;
    sresult = dsv_parser_get_equiv_record_delimiter(parser,i,buf.get()+padsize,
      undersize,&flag);

    BOOST_REQUIRE_MESSAGE(sresult == undersize,
      "unexpected bytesequence size " << sresult << " != " << undersize);

    BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);
  }
}


// FIELD COLUMNS PARAMETER CHECKS

BOOST_AUTO_TEST_CASE( set_field_columns_check )
{
  size_t result;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // check default value
  size_t default_value = 0;
  result = dsv_parser_get_field_columns(parser);

  BOOST_REQUIRE_MESSAGE(result == default_value,
    "unexpected default field column: " << result << " != " << default_value);

  // set to a non-default value
  size_t test_value = 42;
  dsv_parser_set_field_columns(parser,test_value);

  result = dsv_parser_get_field_columns(parser);

  BOOST_REQUIRE_MESSAGE(result == test_value,
    "unexpected field column return: " << result << " != " << test_value);

  // set to zero
  test_value = 0;
  dsv_parser_set_field_columns(parser,test_value);

  result = dsv_parser_get_field_columns(parser);

  BOOST_REQUIRE_MESSAGE(result == test_value,
    "unexpected field column return: " << result << " != " << test_value);

  // set to -1 or the maximum value for size_t
  test_value = -1;
  dsv_parser_set_field_columns(parser,test_value);

  result = dsv_parser_get_field_columns(parser);

  BOOST_REQUIRE_MESSAGE(result == test_value,
    "unexpected field column return: " << result << " != " << test_value);
}




// FIELD DELIMITER PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( set_equiv_field_delimiters_byte_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char bytesequence[] = {'\n'};
  const unsigned char *equiv_byteseq[] = {bytesequence};
  size_t byteseq_size[] = {1};
  int byteseq_repeat[] = {0};
  size_t size = 1;
  int repeatflag = 0;
  int exclusiveflag = 0;

  int iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  size_t sresult = dsv_parser_num_equiv_field_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == size,
    "number of field delimiters " << sresult << " != " << size);

  iresult = dsv_parser_get_equiv_field_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == repeatflag,
    "field delimiters repeatflag " << iresult << " != " << repeatflag);

  iresult = dsv_parser_get_equiv_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == exclusiveflag,
    "field delimiters exclusiveflag " << iresult << " != " << exclusiveflag);

  // get bytesequence size
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  // get bytesequence size with repeatflag
  int flag = 42;

  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);


  // get invalid bytesequence size
  sresult = dsv_parser_get_equiv_field_delimiter(parser,size,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "unexpected bytesequence size " << sresult << " != " << 0);

  static const size_t bufsize = 5;
  unsigned char buf[bufsize];
  unsigned char sentry_buf[bufsize] = {'X','X','X','X','X'};
  unsigned char check_buf[bufsize] = {bytesequence[0],'X','X','X','X'};

  // check for returned bytesequence of exact buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,1,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of larger buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,bufsize,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of zero buff size and valid repeatflag
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,sentry_buf));

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);
}


BOOST_AUTO_TEST_CASE( set_equiv_field_delimiters_multibyte_check )
{
  // these are declared here for convenience of adding/removing new tests
  // without worrying about the variable declaration
  int iresult;
  size_t sresult;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char bytesequence[] = {'\n',0xFF,'h','e','l','l','o'};
  const unsigned char *equiv_byteseq[] = {bytesequence};
  size_t byteseq_size[] = {7};
  int byteseq_repeat[] = {0};
  size_t size = 1;
  int repeatflag = 0;
  int exclusiveflag = 0;

  // check for pathological equiv_bytesequence size
  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,
      std::numeric_limits<size_t>::max(),0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for 0 equiv_bytesequence size
  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,0,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for pathological individual bytesequece size
  size_t path_byteseq_size[] = {std::numeric_limits<size_t>::max()};
  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,path_byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for 0 individual bytesequece size
  size_t zero_byteseq_size[] = {0};
  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,zero_byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,1,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult);

  sresult = dsv_parser_num_equiv_field_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == size,
    "number of field delimiters " << sresult << " != " << size);

  iresult = dsv_parser_get_equiv_field_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == repeatflag,
    "field delimiters repeatflag " << iresult << " != " << repeatflag);

  iresult = dsv_parser_get_equiv_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == exclusiveflag,
    "field delimiters exclusiveflag " << iresult << " != " << exclusiveflag);

  // get bytesequence size
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  // get bytesequence size with repeatflag
  int flag = 42;

  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);


  // get invalid bytesequence size
  sresult = dsv_parser_get_equiv_field_delimiter(parser,size,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "unexpected bytesequence size " << sresult << " != " << 0);

  static const size_t bufsize = 14;
  unsigned char buf[bufsize];
  unsigned char sentry_buf[bufsize];
  std::fill(sentry_buf,sentry_buf+bufsize,'X');

  unsigned char check_buf[bufsize];
  std::fill(std::copy(bytesequence,bytesequence+byteseq_size[0],
      check_buf),check_buf+bufsize,'X');

  // check for returned bytesequence of exact buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,
    byteseq_size[0],0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of larger buff size
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,bufsize,0);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,check_buf));

  // check for returned bytesequence of zero buff size and valid repeatflag
  std::copy(sentry_buf,sentry_buf+bufsize,buf);
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[0],
    "unexpected bytesequence size " << sresult << " != " << byteseq_size[0]);

  BOOST_REQUIRE(std::equal(buf,buf+bufsize,sentry_buf));

  BOOST_REQUIRE_MESSAGE(flag == repeatflag,
    "unexpected bytesequence repetflag " << flag << " != " << repeatflag);
}

// schedule this after the getting/setting tests
BOOST_AUTO_TEST_CASE( set_equiv_field_delimiters_default_check )
{
  int iresult;
  size_t sresult;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // check for default field delimiter (comma)
  sresult = dsv_parser_num_equiv_field_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == 1,
    "default number of field delimiters " << sresult << " != 1");

  iresult = dsv_parser_get_equiv_field_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "default field delimiters repeatflag " << iresult << " != 0");

  iresult = dsv_parser_get_equiv_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "default field delimiters exclusiveflag " << iresult << " != 1");

  // get bytesequence size
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 1,
    "default bytesequence size " << sresult << " != 1");

  static const size_t default_byteseq_size = 1;
  unsigned char buf[default_byteseq_size];
  unsigned char default_bytesequence[default_byteseq_size] = {','};
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,buf,
    default_byteseq_size,0);

  BOOST_REQUIRE_MESSAGE(sresult == default_byteseq_size,
    "unexpected default bytesequence size " << sresult << " != "
      << default_byteseq_size);

  BOOST_REQUIRE(std::equal(buf,buf+default_byteseq_size,default_bytesequence));
}

BOOST_AUTO_TEST_CASE( set_equiv_field_delimiters_multi_multibyte_check )
{
  // these are declared here for convenience of adding/removing new tests
  // without worrying about the variable declaration
  int iresult;
  size_t sresult;

  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char bytesequence1[] = {'\n',0xFF,'h','e','l','l','o'};
  unsigned char bytesequence2[] = {0x0D};
  unsigned char bytesequence3[] = {'w','o','r','l','d',0x0A};
  const unsigned char *equiv_byteseq[] = {bytesequence1,bytesequence2,
    bytesequence3};
  size_t byteseq_size[] = {7,1,6};
  int byteseq_repeat[] = {0,1,0};
  size_t size = 3;
  int repeatflag = 0;
  int exclusiveflag = 0;

  // check for pathological individual bytesequece size
  size_t path_byteseq_size[] = {7,std::numeric_limits<size_t>::max(),6};
  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,path_byteseq_size,byteseq_repeat,3,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  // check for 0 individual bytesequece size
  size_t zero_byteseq_size[] = {7,1,0};
  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,zero_byteseq_size,byteseq_repeat,3,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == EINVAL,
    "unexpected exit code: " << iresult
      << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

  iresult = dsv_parser_set_equiv_field_delimiters(parser,
    equiv_byteseq,byteseq_size,byteseq_repeat,size,0,0);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "unexpected exit code: " << iresult);

  sresult = dsv_parser_num_equiv_field_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == size,
    "number of field delimiters " << sresult << " != " << size);

  iresult = dsv_parser_get_equiv_field_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == repeatflag,
    "field delimiters repeatflag " << iresult << " != " << repeatflag);

  iresult = dsv_parser_get_equiv_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == exclusiveflag,
    "field delimiters exclusiveflag " << iresult << " != " << exclusiveflag);

  for(std::size_t i=0; i<size; ++i) {
    // check bytesequence size
    sresult = dsv_parser_get_equiv_field_delimiter(parser,i,0,0,0);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "bytesequence size " << sresult << " != " << byteseq_size[i]);

    // check bytesequence size with repeatflag
    int flag = 42;
    sresult = dsv_parser_get_equiv_field_delimiter(parser,i,0,0,&flag);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "bytesequence size " << sresult << " != " << byteseq_size[i]);

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);

    std::size_t padsize = 5;
    std::size_t buffsize = byteseq_size[i]+(2*padsize);
    std::unique_ptr<unsigned char[]> buf(new unsigned char[buffsize]);
    std::unique_ptr<unsigned char[]> check_buf(new unsigned char[buffsize]);
    std::fill(buf.get(),buf.get()+buffsize,0xFF);
    std::fill(check_buf.get(),check_buf.get()+buffsize,0xFF);
    std::copy(equiv_byteseq[i],equiv_byteseq[i]+byteseq_size[i],
      check_buf.get()+padsize);

    // check for returned bytesequence of exact buff size
    // check flag again since library is actually filling the buffer
    flag = 42;
    sresult = dsv_parser_get_equiv_field_delimiter(parser,i,buf.get()+padsize,
      byteseq_size[i],&flag);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "unexpected bytesequence size " << sresult << " != " << byteseq_size[i]);

    BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);

    //reset buffer
    std::fill(buf.get(),buf.get()+buffsize,0xFF);

    // check for returned bytesequence of larger buff size
    // check flag again since library is actually filling the buffer
    flag = 42;
    sresult = dsv_parser_get_equiv_field_delimiter(parser,i,buf.get()+padsize,
      byteseq_size[i]+padsize,&flag);

    BOOST_REQUIRE_MESSAGE(sresult == byteseq_size[i],
      "unexpected bytesequence size " << sresult << " != " << byteseq_size[i]);

    BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);

    //reset buffer
    std::fill(buf.get(),buf.get()+buffsize,0xFF);
    //adjust check buffer
    std::size_t undersize = std::max((byteseq_size[i]/2),1ul);
    std::fill(check_buf.get(),check_buf.get()+buffsize,0xFF);
    std::copy(equiv_byteseq[i],equiv_byteseq[i]+undersize,
      check_buf.get()+padsize);

    // check for returned bytesequence of undersized buff size
    // check flag again since library is actually filling the buffer
    flag = 42;
    sresult = dsv_parser_get_equiv_field_delimiter(parser,i,buf.get()+padsize,
      undersize,&flag);

    BOOST_REQUIRE_MESSAGE(sresult == undersize,
      "unexpected bytesequence size " << sresult << " != " << undersize);

    BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

    BOOST_REQUIRE_MESSAGE(flag == byteseq_repeat[i],
      "unexpected bytesequence repetflag " << flag << " != "
        << byteseq_repeat[i]);
  }
}









// FIELD ESCAPE PAIR PARAMETER CHECKS
BOOST_AUTO_TEST_CASE( field_escape_pair_default_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  unsigned char default_bytesequence[] = {'"'};
  const unsigned char *default_field_escape_seq[] = {default_bytesequence};
  size_t default_escape_seq_size[] = {1};
  int default_escape_repeat[] = {0};
  size_t default_size = 1;
  int default_repeatflag = 0;
  int default_exclusiveflag = 1;

  // check for defaults first since we are appending
  size_t sresult = dsv_parser_num_field_escape_pairs(parser);

  BOOST_REQUIRE_MESSAGE(sresult == default_size,
    "number of field delimiters " << sresult << " != " << default_size);

  int iresult = dsv_parser_get_field_escape_pair_open_repeatflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_repeatflag,
    "default open field escape repeat: " << sresult << " != "
      << default_repeatflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_open_repeatflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional open field escape repeats: " << sresult << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_close_repeatflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_repeatflag,
    "default close field escape repeat[0]: " << sresult << " != "
      << default_repeatflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_close_repeatflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional close field escape repeats: " << sresult << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "default open exclusiveflag[0]: " << sresult << " != "
      << default_exclusiveflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional open field escape exclusiveflags: " << sresult
      << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "default close exclusiveflag[0]: " << sresult << " != "
      << default_exclusiveflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional close field escape exclusiveflags: " << sresult
      << " !< 0");


  sresult = dsv_parser_num_field_escape_pair_open_sequences(parser,0);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "number of open field delimiters sequences " << sresult << " != "
      << default_escape_seq_size[0]);

  // check for invalid arguments
  sresult = dsv_parser_num_field_escape_pair_open_sequences(parser,1);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "nonzero number of open field delimiters sequences for invalid pair"
      << sresult << " != " << 0);



  sresult = dsv_parser_num_field_escape_pair_close_sequences(parser,0);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "number of close field delimiters sequences " << sresult << " != "
      << default_escape_seq_size[0]);

  // check for invalid arguments
  sresult = dsv_parser_num_field_escape_pair_close_sequences(parser,1);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "nonzero number of close field delimiters sequences for invalid pair"
      << sresult << " != " << 0);




  // check field escape open size with repeatflag
  int flag = 42;
  sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,
    0,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "pair open sequence size " << sresult << " != "
      << default_escape_seq_size[0]);

  BOOST_REQUIRE_MESSAGE(flag == default_escape_repeat[0],
    "unexpected pair open sequence repetflag " << flag << " != "
      << default_escape_repeat[0]);

  std::size_t padsize = 5;
  std::size_t buffsize = default_escape_seq_size[0]+(2*padsize);
  std::unique_ptr<unsigned char[]> buf(new unsigned char[buffsize]);
  std::unique_ptr<unsigned char[]> check_buf(new unsigned char[buffsize]);
  std::fill(buf.get(),buf.get()+buffsize,0xFF);
  std::fill(check_buf.get(),check_buf.get()+buffsize,0xFF);
  std::copy(default_field_escape_seq[0],
    default_field_escape_seq[0]+default_escape_seq_size[0],
    check_buf.get()+padsize);

  // check for returned bytesequence of exact buff size
  // check flag again since library is actually filling the buffer
  flag = 42;
  sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,0,0,
    buf.get()+padsize,default_escape_seq_size[0],&flag);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "pair open sequence size " << sresult << " != "
      << default_escape_seq_size[0]);

  BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

  BOOST_REQUIRE_MESSAGE(flag == default_escape_repeat[0],
    "unexpected bytesequence repeatflag " << flag << " != "
      << default_escape_repeat[0]);

  //reset buffer
  std::fill(buf.get(),buf.get()+buffsize,0xFF);

  // check for returned bytesequence of larger buff size
  // check flag again since library is actually filling the buffer
  flag = 42;
  sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,0,0,
    buf.get()+padsize,default_escape_seq_size[0]+padsize,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "pair open sequence size " << sresult << " != "
      << default_escape_seq_size[0]);

  BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

  BOOST_REQUIRE_MESSAGE(flag == default_escape_repeat[0],
    "unexpected bytesequence repeatflag " << flag << " != "
      << default_escape_repeat[0]);


  iresult = dsv_parser_get_field_escape_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "unexpected field escape exclusiveflag " << iresult << " != "
      << default_exclusiveflag);


  // check field escape close size with repeatflag
  flag = 42;
  sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,
    0,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "pair close sequence size " << sresult << " != "
      << default_escape_seq_size[0]);

  BOOST_REQUIRE_MESSAGE(flag == default_escape_repeat[0],
    "unexpected pair close sequence repetflag " << flag << " != "
      << default_escape_repeat[0]);

  //reset buffer
  std::fill(buf.get(),buf.get()+buffsize,0xFF);

  // check for returned bytesequence of exact buff size
  // check flag again since library is actually filling the buffer
  flag = 42;
  sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,0,0,
    buf.get()+padsize,default_escape_seq_size[0],&flag);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "pair close sequence size " << sresult << " != "
      << default_escape_seq_size[0]);

  BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

  BOOST_REQUIRE_MESSAGE(flag == default_escape_repeat[0],
    "unexpected bytesequence repeatflag " << flag << " != "
      << default_escape_repeat[0]);

  //reset buffer
  std::fill(buf.get(),buf.get()+buffsize,0xFF);

  // check for returned bytesequence of larger buff size
  // check flag again since library is actually filling the buffer
  flag = 42;
  sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,0,0,
    buf.get()+padsize,default_escape_seq_size[0]+padsize,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == default_escape_seq_size[0],
    "pair close sequence size " << sresult << " != "
      << default_escape_seq_size[0]);

  BOOST_REQUIRE(std::equal(buf.get(),buf.get()+buffsize,check_buf.get()));

  BOOST_REQUIRE_MESSAGE(flag == default_escape_repeat[0],
    "unexpected bytesequence repeatflag " << flag << " != "
      << default_escape_repeat[0]);
}




BOOST_AUTO_TEST_CASE( field_escape_pair_clear_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  // clear out defaults
  dsv_parser_clear_field_escape_pairs(parser);

  // check for empty
  size_t sresult = dsv_parser_num_field_escape_pairs(parser);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "number of field delimiters " << sresult << " != " << 0);

  // check for invalid arguments
  int iresult = dsv_parser_get_field_escape_pair_open_repeatflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "default open field escape repeat: " << sresult << " !< 0");

  iresult = dsv_parser_get_field_escape_pair_close_repeatflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "default close field escape repeat: " << sresult << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "default open exclusiveflag[0]: " << sresult << " !< 0");

  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "default close exclusiveflag[0]: " << sresult << " !< 0");


  sresult = dsv_parser_num_field_escape_pair_open_sequences(parser,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "number of open field delimiters sequences " << sresult << " != 0");

  sresult = dsv_parser_num_field_escape_pair_close_sequences(parser,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "number of close field delimiters sequences " << sresult << " != 0");


  // check field escape open size with repeatflag
  int flag = 42;
  sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,
    0,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "pair open sequence size " << sresult << " != 0");

  BOOST_REQUIRE_MESSAGE(flag == 42,
    "pair open sequence repeatflag unexpected modified provided value "
      << flag << " != 42");

  sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,
    0,0,0,0,&flag);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "pair close sequence size " << sresult << " != 0");

  BOOST_REQUIRE_MESSAGE(flag == 42,
    "pair close sequence repeatflag unexpected modified provided value "
      << flag << " != 42");


  iresult = dsv_parser_get_field_escape_exclusiveflag(parser);

  int default_exclusiveflag = 1;
  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "unexpected field escape exclusiveflag " << iresult << " != "
      << default_exclusiveflag);
}



// check multiple open and closing sequences. Do this iteratively and build it
// up. That is,
// - clear and insert the first and then check it
// - clear and insert the first and second, then check each
// - clear and insert the first, second, and third , then check each
// we do this to ensure appending doesn't invalidate any prior pair
BOOST_AUTO_TEST_CASE( field_escape_pair_complex_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int iresult;
  std::size_t sresult;

  std::size_t num_field_escape_pairs = 0;

  unsigned char obytesequence0[] = {'O'};
  unsigned char obytesequence1_0[] = {'o','p','e','n'};
  unsigned char obytesequence1_1[] = {'f','o','o'};
  unsigned char obytesequence2_0[] = {0xfb, 0x17, 0x2b, 0xf5};
  unsigned char obytesequence2_1[] = {0x5b, 0x5c, 0x98, 0x2c};
  unsigned char obytesequence2_2[] = {0x6c, 0xa1, 0xed, 0x32};
  const unsigned char *open_escape_seq[][3] = {
    {obytesequence0},
    {obytesequence1_0,obytesequence1_1},
    {obytesequence2_0,obytesequence2_1,obytesequence2_2}
  };

  std::size_t open_escape_seq_size[][3] = {{1},{4,3},{4,4,4}};
  int open_escape_repeat[][3] = {{1},{0,1},{0,0,1}};
  std::size_t open_size[] = {1,2,3};
  int open_repeatflag[] = {1,0,1};
  int open_exclusiveflag[] = {0,1,1};

  unsigned char cbytesequence0[] = {'C'};
  unsigned char cbytesequence1_0[] = {'c','l','o','s','e'};
  unsigned char cbytesequence1_1[] = {'b','a','r'};
  unsigned char cbytesequence1_2[] = {'b','a','r','f','o','o'};
  unsigned char cbytesequence2_0[] = {0x12, 0xca, 0xfb, 0x22};
  unsigned char cbytesequence2_1[] = {0x9a, 0x42, 0x72, 0xde};
  const unsigned char *close_escape_seq[][3] = {
    {cbytesequence0},
    {cbytesequence1_0,cbytesequence1_1,cbytesequence1_2},
    {cbytesequence2_0,cbytesequence2_1}
  };

  std::size_t close_escape_seq_size[][3] = {{1},{5,3,6},{4,4}};
  int close_escape_repeat[][3] = {{0},{1,0,0},{1,1}};
  std::size_t close_size[] = {1,3,2};
  int close_repeatflag[] = {0,1,0};
  int close_exclusiveflag[] = {1,0,0};

  std::size_t sets = 3;
  std::size_t padsize = 5;


  for(std::size_t n=0; n<sets; ++n) {
    // clear out defaults
    dsv_parser_clear_field_escape_pairs(parser);

    // add 0 through n sets
    for(std::size_t i=0; i<n; ++i) {
      iresult = dsv_parser_append_field_escape_pair(parser,
        open_escape_seq[i],open_escape_seq_size[i],open_escape_repeat[i],
          open_size[i],open_repeatflag[i],open_exclusiveflag[i],
        close_escape_seq[i],close_escape_seq_size[i],close_escape_repeat[i],
          close_size[i],close_repeatflag[i],close_exclusiveflag[i]);

      BOOST_REQUIRE_MESSAGE(iresult == 0,
        "unexpected exit code from " << i << "th insertion: " << iresult
          << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL);

      num_field_escape_pairs = dsv_parser_num_field_escape_pairs(parser);

      BOOST_REQUIRE_MESSAGE(num_field_escape_pairs == i+1,
        "number of field delimiters " << num_field_escape_pairs << " != "
          << i+1);
    }

    dsv_parser_set_field_escape_exclusiveflag(parser,0);

    //check the validity of each pair up to i
    for(std::size_t pair_index = 0; pair_index<n; ++pair_index) {
      sresult = dsv_parser_num_field_escape_pair_open_sequences(parser,
        pair_index);

      BOOST_REQUIRE_MESSAGE(sresult == open_size[pair_index],
        "number of open sequences for pair " << pair_index << ": " << sresult
          << " != " << open_size[pair_index]);

      // check for open correctness
      for(size_t i=0; i<open_size[pair_index]; ++i) {
        std::size_t obuffsize = open_escape_seq_size[pair_index][i]+(3*padsize);
        std::unique_ptr<unsigned char[]> obuf(new unsigned char[obuffsize]);
        std::unique_ptr<unsigned char[]> ocheck_buf(
          new unsigned char[obuffsize]);
        std::fill(obuf.get(),obuf.get()+obuffsize,0xFF);
        std::fill(ocheck_buf.get(),ocheck_buf.get()+obuffsize,0xFF);
        // checkbuf laid out like:
        // padsize byteseq padsize padsize
        std::copy(open_escape_seq[pair_index][i],
          open_escape_seq[pair_index][i]+open_escape_seq_size[pair_index][i],
          ocheck_buf.get()+padsize);

        // CHECK FOR OPEN BYTESEQUENCE OF EXACT BUFF SIZE
        int flag = 42;
        sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,
          pair_index,i,obuf.get()+padsize,
          open_escape_seq_size[pair_index][i],&flag);

        BOOST_REQUIRE_MESSAGE(sresult == open_escape_seq_size[pair_index][i],
          "incorrect return size of pair " << pair_index
            << " open sequence " << i << ": "
            << sresult << " != " << open_escape_seq_size[pair_index][i]);

        BOOST_REQUIRE(std::equal(obuf.get(),obuf.get()+obuffsize,
          ocheck_buf.get()));

        BOOST_REQUIRE_MESSAGE(flag == open_escape_repeat[pair_index][i],
          "unexpected bytesequence repeatflag for pair " << pair_index
          << ": " << flag << " != " << open_escape_repeat[i]);


        // CHECK FOR OPEN BYTESEQUENCE OF LARGER BUFF SIZE

        //reset buffers
        std::fill(obuf.get(),obuf.get()+obuffsize,0xFF);

        flag = 42;
        sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,
          pair_index,i,obuf.get()+padsize,
          open_escape_seq_size[pair_index][i]+padsize,&flag);

        BOOST_REQUIRE_MESSAGE(sresult == open_escape_seq_size[pair_index][i],
          "incorrect return size of pair " << pair_index
            << " open sequence " << i << ": "
            << sresult << " != " << open_escape_seq_size[pair_index][i]);

        BOOST_REQUIRE(std::equal(obuf.get(),obuf.get()+obuffsize,
          ocheck_buf.get()));

        BOOST_REQUIRE_MESSAGE(flag == open_escape_repeat[pair_index][i],
          "unexpected bytesequence repeatflag for pair " << pair_index
          << ": " << flag << " != " << open_escape_repeat[i]);


        // CHECK FOR OPEN BYTESEQUENCE OF SMALLER BUFF SIZE BUT
        // NO SMALLER THAN SIZE 1

        //reset buffers
        std::fill(obuf.get(),obuf.get()+obuffsize,0xFF);

        std::size_t half_seq_size =
          std::max(std::size_t(1),(open_escape_seq_size[pair_index][i])/2);

        //setup check buffer for partial contents
        std::fill(ocheck_buf.get(),ocheck_buf.get()+obuffsize,0xFF);
        // checkbuf laid out like:
        // padsize byteseq/2 byteseq/2 byteseq/2 padsize padsize
        std::copy(open_escape_seq[pair_index][i],
          open_escape_seq[pair_index][i]+half_seq_size,
          ocheck_buf.get()+padsize);

        flag = 42;
        sresult = dsv_parser_get_field_escape_pair_open_sequence(parser,
          pair_index,i,obuf.get()+padsize,half_seq_size,&flag);

        BOOST_REQUIRE_MESSAGE(sresult == half_seq_size,
          "incorrect return size of pair " << pair_index
            << " open sequence " << i << ": "
            << sresult << " != " << half_seq_size);

        BOOST_REQUIRE(std::equal(obuf.get(),obuf.get()+obuffsize,
          ocheck_buf.get()));

        BOOST_REQUIRE_MESSAGE(flag == open_escape_repeat[pair_index][i],
          "unexpected bytesequence repeatflag for pair " << pair_index
          << ": " << flag << " != " << open_escape_repeat[i]);
      }



      // check for close correctness
      for(size_t i=0; i<close_size[pair_index]; ++i) {
        std::size_t cbuffsize =
          close_escape_seq_size[pair_index][i]+(3*padsize);
        std::unique_ptr<unsigned char[]> cbuf(new unsigned char[cbuffsize]);
        std::unique_ptr<unsigned char[]> ccheck_buf(
          new unsigned char[cbuffsize]);
        std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);
        std::fill(ccheck_buf.get(),ccheck_buf.get()+cbuffsize,0xFF);
        // checkbuf laid out like:
        // padsize byteseq padsize padsize
        std::copy(close_escape_seq[pair_index][i],
          close_escape_seq[pair_index][i]+close_escape_seq_size[pair_index][i],
          ccheck_buf.get()+padsize);

        // CHECK FOR CLOSE BYTESEQUENCE OF EXACT BUFF SIZE

        // close bytesequence
        int flag = 42;
        sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,
          pair_index,i,cbuf.get()+padsize,
          close_escape_seq_size[pair_index][i],&flag);

        BOOST_REQUIRE_MESSAGE(sresult == close_escape_seq_size[pair_index][i],
          "incorrect return size of pair " << pair_index
            << " close sequence " << i << ": "
            << sresult << " != " << close_escape_seq_size[pair_index][i]);

        BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
          ccheck_buf.get()));

        BOOST_REQUIRE_MESSAGE(flag == close_escape_repeat[pair_index][i],
          "unexpected bytesequence repeatflag for pair " << pair_index
          << ": " << flag << " != " << close_escape_repeat[i]);



        // CHECK FOR CLOSE BYTESEQUENCE OF LARGER BUFF SIZE

        //reset buffers
        std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);

        // close bytesequence
        flag = 42;
        sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,
          pair_index,i,cbuf.get()+padsize,
          close_escape_seq_size[pair_index][i]+padsize,&flag);

        BOOST_REQUIRE_MESSAGE(sresult == close_escape_seq_size[pair_index][i],
          "incorrect return size of pair " << pair_index
            << " close sequence " << i << ": "
            << sresult << " != " << close_escape_seq_size[pair_index][i]);

        BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
          ccheck_buf.get()));

        BOOST_REQUIRE_MESSAGE(flag == close_escape_repeat[pair_index][i],
          "unexpected bytesequence repeatflag for pair " << pair_index
          << ": " << flag << " != " << close_escape_repeat[i]);


        // CHECK FOR CLOSE BYTESEQUENCE OF SMALLER BUFF SIZE BUT
        // NO SMALLER THAN SIZE 1

        //reset buffers
        std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);

        std::size_t half_seq_size =
          std::max(std::size_t(1),(close_escape_seq_size[pair_index][i])/2);

        //setup check buffer for partial contents
        std::fill(ccheck_buf.get(),ccheck_buf.get()+cbuffsize,0xFF);
        // checkbuf laid out like:
        // padsize byteseq/2 byteseq/2 byteseq/2 padsize padsize
        std::copy(close_escape_seq[pair_index][i],
          close_escape_seq[pair_index][i]+half_seq_size,
          ccheck_buf.get()+padsize);

        // close bytesequence
        flag = 42;
        sresult = dsv_parser_get_field_escape_pair_close_sequence(parser,
          pair_index,i,cbuf.get()+padsize,half_seq_size,&flag);

        BOOST_REQUIRE_MESSAGE(sresult == half_seq_size,
          "incorrect return size of pair " << pair_index
            << " close sequence " << i << ": "
            << sresult << " != " << half_seq_size);

        BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
          ccheck_buf.get()));

        BOOST_REQUIRE_MESSAGE(flag == close_escape_repeat[pair_index][i],
          "unexpected bytesequence repeatflag for pair " << pair_index
          << ": " << flag << " != " << close_escape_repeat[i]);
      }
    }

    iresult = dsv_parser_get_field_escape_exclusiveflag(parser);

    BOOST_REQUIRE_MESSAGE(iresult == 0,
     "unexpected field escape exclusiveflag " << iresult << " != 0");
  }

}





































# if 0
/** \test Test newline getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_newline_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  dsv_newline_behavior behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_permissive,
    "dsv_parser_get_newline_behavior returned a value other than the default "
    "newline behavior (" << behavior << " != " << dsv_newline_permissive
    << ")");

  int err = dsv_parser_set_newline_behavior(parser,(dsv_newline_behavior)999);
  BOOST_REQUIRE_MESSAGE(err!=0,
    "dsv_parser_set_newline_behavior accepted a invalid value of "
    "dsv_newline_behavior");

  behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_permissive,
    "dsv_parser_get_newline_behavior returned a value other than the default "
    "newline behavior after attempting a invalid value of "
    "dsv_newline_behavior (" << behavior << " != " << dsv_newline_permissive
    << ")");

  err = dsv_parser_set_newline_behavior(parser,dsv_newline_lf_strict);
  BOOST_REQUIRE_MESSAGE(err==0,
    "dsv_parser_set_newline_behavior failed with error value " << err);

  behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(behavior==dsv_newline_lf_strict,
    "dsv_parser_get_newline_behavior returned a value other than the set "
    "newline behavior (" << behavior << " != " << dsv_newline_lf_strict << ")");

  dsv_parser_destroy(parser);
}

/** \test Test field columns getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_field_columns_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  ssize_t field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == 0,
    "dsv_parser_get_field_columns returned a value other than the default 0 "
    " (" << field_cols << ")");

  dsv_parser_set_field_columns(parser,-1);
  field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == -1,
    "dsv_parser_get_field_columns returned a value other than the newly set "
    "value of -1 (" << field_cols << ")");

  dsv_parser_destroy(parser);
}

/** \test Test single delimiter getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_single_delimiter_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  std::vector<unsigned char> delimiter = {'a'};

  // add one for error checking
  std::vector<unsigned char> buf(7,'*');

  // CHECK dsv_parser_set_field_delimiter FUNCTION
  int err;
  BOOST_REQUIRE_MESSAGE((err = dsv_parser_set_field_delimiter(parser,'a')) == 0,
    "dsv_parser_set_field_delimiter failed with code: " << err);

  // CHECK dsv_parser_num_field_delimiters FUNCTION
  size_t size = dsv_parser_num_field_delimiters(parser);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_num_field_delimiters did not return 1 for a "
    "single byte delimiter but instead returned: " << size);

  // CHECK dsv_parser_get_field_delimiter FUNCTION
  size = dsv_parser_get_field_delimiter(parser,0,0,0,0);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a buffer length of 1 for a "
    "single byte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(dsv_parser_get_field_delimiters_repeatflag(parser)==0,
    "dsv_parser_get_field_delimiters_repeatflag indicates repeat for a "
    "call to dsv_parser_set_field_delimiter");

  BOOST_REQUIRE_MESSAGE(
    dsv_parser_get_field_delimiters_exclusiveflag(parser)==1,
    "dsv_parser_get_field_delimiters_repeatflag does not indicate "
    "exclusivity for a call to dsv_parser_set_field_delimiter");

  // check for get with exact buffer size
  int repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),1,&repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a byte length of 1 when "
    "retrieving a single byte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == '*' && buf[2] == '*'
     && buf[3] == '*' && buf[4] == '*' && buf[5] == '*' && buf[6] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  BOOST_REQUIRE_MESSAGE(repeatflag == 0,
    "dsv_parser_get_field_delimiter did not return a nonrepeating flag but "
    "instead returned '" << repeatflag << "'");

  // reset buf
  buf.assign(7,'*');


  // check for get with larger buffer size
  repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),buf.size(),
    &repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a byte length of 1 when "
    "retrieving a single byte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == '*' && buf[2] == '*'
     && buf[3] == '*' && buf[4] == '*' && buf[5] == '*' && buf[6] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  BOOST_REQUIRE_MESSAGE(repeatflag == 0,
    "dsv_parser_get_field_delimiter did not return a nonrepeating flag but "
    "instead returned '" << repeatflag << "'");

  dsv_parser_destroy(parser);
}



/** \test Test single delimiter getting and setting
 */
BOOST_AUTO_TEST_CASE( parser_multibyte_delimiter_getting_and_setting )
{
  dsv_parser_t parser = {};

  assert(dsv_parser_create(&parser) == 0);

  std::vector<unsigned char> multibyte_delimiter = {'a','b','c','d','e','f'};

  // add one for error checking
  std::vector<unsigned char> buf(multibyte_delimiter.size()+4,'*');

  // CHECK dsv_parser_set_field_wdelimiter FUNCTION
  int err = dsv_parser_set_field_wdelimiter(parser,multibyte_delimiter.data(),
    multibyte_delimiter.size());
  BOOST_REQUIRE_MESSAGE(err == 0,
    "dsv_parser_set_field_wdelimiter failed with code: " << err
    << " for multibyte delimiter");

  // CHECK dsv_parser_num_field_delimiters FUNCTION
  size_t size = dsv_parser_num_field_delimiters(parser);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_num_field_delimiters did not return 1 for a "
    "single byte delimiter but instead returned: " << size);

  // CHECK dsv_parser_get_field_delimiter FUNCTION
  size = dsv_parser_get_field_delimiter(parser,0,0,0,0);
  BOOST_REQUIRE_MESSAGE(size == multibyte_delimiter.size(),
    "dsv_parser_get_field_delimiter did not return a buffer length of "
    << multibyte_delimiter.size() << " for a "
    "single byte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(dsv_parser_get_field_delimiters_repeatflag(parser)==0,
    "dsv_parser_get_field_delimiters_repeatflag indicates repeat for a "
    "call to dsv_parser_set_field_wdelimiter");

  BOOST_REQUIRE_MESSAGE(
    dsv_parser_get_field_delimiters_exclusiveflag(parser)==1,
    "dsv_parser_get_field_delimiters_repeatflag does not indicate "
    "exclusivity for a call to dsv_parser_set_field_wdelimiter");

  // check for get with exact buffer size
  int repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),
    multibyte_delimiter.size(),&repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 6,
    "dsv_parser_get_field_delimiter did not return a byte length of 6 when "
    "retrieving the multibyte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c'
     && buf[3] == 'd' && buf[4] == 'e' && buf[5] == 'f' && buf[6] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  // reset buf
  buf.assign(multibyte_delimiter.size()+1,'*');


  // check for get with larger buffer size
  repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),buf.size(),
    &repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 6,
    "dsv_parser_get_field_delimiter did not return a byte length of 6 when "
    "retrieving the multibyte delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c'
     && buf[3] == 'd' && buf[4] == 'e' && buf[5] == 'f' && buf[6] == '*'
     && buf[7] == '*' && buf[8] == '*' && buf[9] == '*',
    "dsv_parser_get_field_delimiter did not copy a sigle byte buffer "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "','" << buf[2]
     << "','" << buf[3] << "','" << buf[4] << "','" << buf[5] << "','" << buf[6]
     << "','" << buf[7] << "','" << buf[8] << "','" << buf[9] << "'");

  dsv_parser_destroy(parser);
}


/** \test Check for default parser object settings
 */
BOOST_AUTO_TEST_CASE( parser_default_object_object_settings )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  dsv_newline_behavior nl_behavior = dsv_parser_get_newline_behavior(parser);
  BOOST_REQUIRE_MESSAGE(nl_behavior == dsv_newline_permissive,
    "Default parser newline behavior was not dsv_newline_permissive. Expected "
    << dsv_newline_permissive << " received " << nl_behavior);

  ssize_t field_cols = dsv_parser_get_field_columns(parser);
  BOOST_REQUIRE_MESSAGE(field_cols == 0,
    "Default parser field columns was not '0' but rather '" << field_cols
      << "'");
}

/** \test Check for default settings
 */
BOOST_AUTO_TEST_CASE( parser_default_object_delimiter_settings )
{
  dsv_parser_t parser;
  assert(dsv_parser_create(&parser) == 0);
  boost::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  std::vector<unsigned char> buf(2,'*');

  // CHECK dsv_parser_num_field_delimiters FUNCTION
  size_t size = dsv_parser_num_field_delimiters(parser);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_num_field_delimiters did not return 1 for the "
    "default delimiter but instead returned: " << size);

  // CHECK dsv_parser_get_field_delimiter FUNCTION
  size = dsv_parser_get_field_delimiter(parser,0,0,0,0);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a buffer length of 1 for "
    "the default delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(dsv_parser_get_field_delimiters_repeatflag(parser)==0,
    "dsv_parser_get_field_delimiters_repeatflag indicates repeat for the "
    "default delimiter");

  BOOST_REQUIRE_MESSAGE(
    dsv_parser_get_field_delimiters_exclusiveflag(parser)==1,
    "dsv_parser_get_field_delimiters_repeatflag does not indicate "
    "exclusivity for the default delimiter");

  // check for get with exact buffer size
  int repeatflag = -1;
  size = dsv_parser_get_field_delimiter(parser,0,buf.data(),1,&repeatflag);
  BOOST_REQUIRE_MESSAGE(size == 1,
    "dsv_parser_get_field_delimiter did not return a byte length of 1 when "
    "retrieving the default delimiter but instead returned: " << size);

  BOOST_REQUIRE_MESSAGE(buf[0] == ',' && buf[1] == '*',
    "dsv_parser_get_field_delimiter did not copy the default buffer of ',' "
    "correctly. Copied: '" << buf[0] << "','" << buf[1] << "'");

  BOOST_REQUIRE_MESSAGE(repeatflag == 0,
    "dsv_parser_get_field_delimiter did not return a nonrepeating flag for "
    "the default delimiter but instead returned '" << repeatflag << "'");
}
#endif

BOOST_AUTO_TEST_SUITE_END()

}
}
