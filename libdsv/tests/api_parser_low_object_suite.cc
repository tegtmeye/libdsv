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
BOOST_AUTO_TEST_CASE( set_equiv_record_delimiters_check )
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

  iresult = dsv_parser_get_record_delimiter_exclusiveflag(parser);

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

  iresult = dsv_parser_get_record_delimiter_exclusiveflag(parser);

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

  iresult = dsv_parser_get_record_delimiter_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 1,
    "delimiters exclusiveflag " << iresult << " != 1");


}

#if 0


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

  // check for default record delimiter
  sresult = dsv_parser_num_equiv_record_delimiters(parser);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "default number of record delimiters " << sresult << " != 0");

  iresult = dsv_parser_get_equiv_record_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "default record delimiters repeatflag " << iresult << " != 0");

  iresult = dsv_parser_get_equiv_record_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "default record delimiters exclusiveflag " << iresult << " != 0");

  // get bytesequence size
  sresult = dsv_parser_get_equiv_record_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "default bytesequence size " << sresult << " != 0");
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

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "default number of field delimiters " << sresult << " != 0");

  iresult = dsv_parser_get_equiv_field_delimiters_repeatflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "default field delimiters repeatflag " << iresult << " != 0");

  iresult = dsv_parser_get_equiv_field_delimiters_exclusiveflag(parser);

  BOOST_REQUIRE_MESSAGE(iresult == 0,
    "default field delimiters exclusiveflag " << iresult << " != 0");

  // get bytesequence size
  sresult = dsv_parser_get_equiv_field_delimiter(parser,0,0,0,0);

  BOOST_REQUIRE_MESSAGE(sresult == 0,
    "default bytesequence size " << sresult << " != 0");
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

  size_t default_size = 0;
  int default_repeatflag = -1;
  int default_exclusiveflag = -1;

  // check for defaults first since we are appending
  size_t sresult = dsv_parser_num_field_escape_pairs(parser);

  BOOST_REQUIRE_MESSAGE(sresult == default_size,
    "number of field delimiters " << sresult << " != " << default_size);

  int iresult = dsv_parser_get_field_escape_pair_open_repeatflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_repeatflag,
    "default open field escape repeat: " << iresult << " != "
      << default_repeatflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_open_repeatflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional open field escape repeats: " << iresult << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_close_repeatflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_repeatflag,
    "default close field escape repeat[0]: " << iresult << " != "
      << default_repeatflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_close_repeatflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional close field escape repeats: " << iresult << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "default open exclusiveflag[0]: " << iresult << " != "
      << default_exclusiveflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_open_exclusiveflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional open field escape exclusiveflags: " << iresult
      << " !< 0");


  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "default close exclusiveflag[0]: " << sresult << " != "
      << default_exclusiveflag);

  // check for invalid arguments
  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,1);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "invalid additional close field escape exclusiveflags: " << iresult
      << " !< 0");
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
    "default open exclusiveflag[0]: " << iresult << " !< 0");

  iresult = dsv_parser_get_field_escape_pair_close_exclusiveflag(parser,0);

  BOOST_REQUIRE_MESSAGE(iresult < 0,
    "default close exclusiveflag[0]: " << iresult << " !< 0");


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

  int default_exclusiveflag = 0;
  BOOST_REQUIRE_MESSAGE(iresult == default_exclusiveflag,
    "unexpected field escape exclusiveflag " << iresult << " != "
      << default_exclusiveflag);
}



/*
  Check multiple open and closing field escapes sequence pairs and
  escaped field escapes. Do this iteratively and build it up. That is,
   - clear and insert the first field escape pair and escaped field escape
      and then check it
   - clear and insert the first and second field escape pair and escaped field
      escape , then check each
   - clear and insert the first, second, and third field escape pair and escaped
      field escape  then check each
   - etc etc
   we do this to ensure appending doesn't invalidate any prior pair and clearing
   truly clears the set

   This is a less than ideal monolithic check but doing it piecewise opens up
   too many unchecked aspects of each interface
*/
BOOST_AUTO_TEST_CASE( field_escape_pair_complex_check )
{
  dsv_parser_t parser;
  BOOST_REQUIRE(dsv_parser_create(&parser) == 0);
  std::shared_ptr<dsv_parser_t> parser_sentry(&parser,detail::parser_destroy);

  int iresult;
  std::size_t sresult;

  std::size_t num_field_escape_pairs = 0;

  /*
      There are 3 pairs of opening and closing equiv butesequences and equiv
      replacements. The pairs are:
        - 1 open, 1 close, 2 escapes w replacement
        - 2 open, 3 close, 1 escapes w replacement
        - 3 open, 2 close, 4 escape w no replacement
  */
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

  // escaped field escapes
  unsigned char ebytesequence0_0_0[] = {'\\','"'};
  unsigned char ebytesequence0_0_1[] = {'0','x','2','2'};
  unsigned char rbytesequence0_0[]   = {0x22};

  unsigned char ebytesequence0_1_0[] = {'\\','\''};
  unsigned char ebytesequence0_1_1[] = {'0','x','2','7'};
  unsigned char rbytesequence0_1[] = {0x27};


  unsigned char ebytesequence1_0_0[] = {'e','s','c','a','p','e'};
  unsigned char rbytesequence1_0[] = {'s', 'e', 'q'};


  unsigned char ebytesequence2_0_0[] = {'b','l','a','h'};
  unsigned char ebytesequence2_0_1[] = {'h','a','l','b'};
  //zero
  //unsigned char rbytesequence2_0[] = {};

  unsigned char ebytesequence2_1_0[] = {'a','s','d','f'};
  unsigned char ebytesequence2_1_1[] = {'q','w','e','r','t','y'};
  unsigned char ebytesequence2_1_2[] = {'z','x','c','v','b'};
  unsigned char rbytesequence2_1[]   = {'r','e','p','l','a','c','e','m','e'};


  const unsigned char *escaped_seq[][2][3] = {
    {
      {ebytesequence0_0_0,ebytesequence0_0_1},
      {ebytesequence0_1_0,ebytesequence0_1_1}
    },
    {
      {ebytesequence1_0_0}
    },
    {
      {ebytesequence2_0_0,ebytesequence2_0_1},
      {ebytesequence2_1_0,ebytesequence2_1_1,ebytesequence2_1_2}
    }
  };

  std::size_t num_equiv_sequences[][2] = {
    {2,2},
    {1},
    {2,3}
  };

  std::size_t escapes_per_pair[] = {
    2,1,2
  };

  std::size_t escaped_seq_size[][2][3] = {
    {
      {2,4},
      {2,4}
    },
    {
      {6}
    },
    {
      {4,4},
      {4,6,5},
    }
  };

  int escaped_repeat[][2][3] = {
    {
      {1,0},
      {0,1}
    },
    {
      {0}
    },
    {
      {1,1},
      {0,1,0},
    }
  };

  const unsigned char *replacement_seq[][2] = {
    {rbytesequence0_0,rbytesequence0_1},
    {rbytesequence1_0},
    {0,rbytesequence2_1}
  };

  std::size_t replacement_seq_size[][2] = {
    {1,1},
    {3},
    {0,9}
  };


  int escaped_repeatflag[][2] = {
    {0,1},
    {1},
    {1,0}
  };

  int escaped_exclusiveflag[][2] = {
    {1,0},
    {0},
    {0,1}
  };



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
          << " (ENOMEM = " << ENOMEM << ", EINVAL = " << EINVAL << ")");

      num_field_escape_pairs = dsv_parser_num_field_escape_pairs(parser);

      BOOST_REQUIRE_MESSAGE(num_field_escape_pairs == i+1,
        "number of field delimiters " << num_field_escape_pairs << " != "
          << i+1);

      sresult = dsv_parser_num_equiv_escaped_field_escapes(parser,i);

      BOOST_REQUIRE_MESSAGE(sresult == 0,
          "for newly inserted pair '" << i
            << "' number of equivalent escape field escapes '"
            << sresult << " != 0");

      for(std::size_t j=0; j<escapes_per_pair[i]; ++j) {
        iresult = dsv_parser_append_equiv_escaped_field_escapes(parser,i,
          escaped_seq[i][j],escaped_seq_size[i][j],escaped_repeat[i][j],
          num_equiv_sequences[i][j],escaped_repeatflag[i][j],
          escaped_exclusiveflag[i][j],replacement_seq[i][j],
          replacement_seq_size[i][j]);

        BOOST_REQUIRE_MESSAGE(iresult == 0,
          "unexpected exit code from " << i << "th escaped field escapes "
          "insertion: " << iresult << " (ENOMEM = " << ENOMEM << ", EINVAL = "
          << EINVAL << ")");

        sresult = dsv_parser_num_equiv_escaped_field_escapes(parser,i);

        BOOST_REQUIRE_MESSAGE(sresult == j+1,
          "for pair '" << i << "' number of equivalent escape field escapes '"
            << sresult << " != " << j+1);
      }

      sresult = dsv_parser_num_equiv_escaped_field_escapes(parser,i);

      BOOST_REQUIRE_MESSAGE(sresult == escapes_per_pair[i],
        "for pair '" << i << "' number of equivalent escape field escapes '"
          << sresult << " != " << escapes_per_pair[i]);
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

      // check for escaped field escapes correctness
      sresult = dsv_parser_num_equiv_escaped_field_escapes(parser,pair_index);

      BOOST_REQUIRE_MESSAGE(sresult == escapes_per_pair[pair_index],
        "for pair '" << pair_index
          << "' number of equivalent escape field escapes '"
          << sresult << " != " << escapes_per_pair[pair_index]);


      for(std::size_t i=0; i<escapes_per_pair[pair_index]; ++i) {
        sresult = dsv_parser_num_equiv_escaped_field_escapes_sequences(
          parser,pair_index,i);

        BOOST_REQUIRE_MESSAGE(sresult == num_equiv_sequences[pair_index][i],
          "for pair '" << i << "' number of equivalent escaped field escape "
          "sequences '" << sresult << " != "
            << num_equiv_sequences[pair_index][i]);

        for(std::size_t j=0; j<num_equiv_sequences[pair_index][i]; ++j) {
          std::size_t cbuffsize =
            escaped_seq_size[pair_index][i][j]+(3*padsize);
          std::unique_ptr<unsigned char[]> cbuf(new unsigned char[cbuffsize]);
          std::unique_ptr<unsigned char[]> ccheck_buf(
            new unsigned char[cbuffsize]);
          std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);
          std::fill(ccheck_buf.get(),ccheck_buf.get()+cbuffsize,0xFF);
          // checkbuf laid out like:
          // padsize byteseq padsize padsize
          std::copy(escaped_seq[pair_index][i][j],
            escaped_seq[pair_index][i][j]+
              escaped_seq_size[pair_index][i][j],
            ccheck_buf.get()+padsize);

          // CHECK FOR BYTESEQUENCE OF EXACT BUFF SIZE

          int flag = 42;
          sresult = dsv_parser_get_equiv_escaped_field_escapes_sequence(
            parser,pair_index,i,j,cbuf.get()+padsize,
            escaped_seq_size[pair_index][i][j],&flag);

          BOOST_REQUIRE_MESSAGE(sresult == escaped_seq_size[pair_index][i][j],
            "incorrect return size of pair " << pair_index
              << " replacement set " << i << " equiv bytesequence " << j
              << ": " << sresult << " != "
              << escaped_seq_size[pair_index][i][j]);

          BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
            ccheck_buf.get()));

          BOOST_REQUIRE_MESSAGE(flag == escaped_repeat[pair_index][i][j],
            "incorrect repeatflag for pair " << pair_index
              << " replacement set " << i << " equiv bytesequence " << j
              << " replacement set: " << i << ": " << flag << " != "
              << escaped_repeat[pair_index][i][j]);

          // CHECK FOR BYTESEQUENCE OF LARGER BUFF SIZE

          //reset buffers
          std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);

          flag = 42;
          sresult = dsv_parser_get_equiv_escaped_field_escapes_sequence(
            parser,pair_index,i,j,cbuf.get()+padsize,
            escaped_seq_size[pair_index][i][j]+padsize,&flag);

          BOOST_REQUIRE_MESSAGE(sresult == escaped_seq_size[pair_index][i][j],
            "incorrect return size of pair " << pair_index
              << " replacement set " << i << " equiv bytesequence " << j
              << ": " << sresult << " != "
              << escaped_seq_size[pair_index][i][j]);

          BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
            ccheck_buf.get()));

          BOOST_REQUIRE_MESSAGE(flag == escaped_repeat[pair_index][i][j],
            "incorrect repeatflag for pair " << pair_index
              << " replacement set " << i << " equiv bytesequence " << j
              << " replacement set: " << i << ": " << flag << " != "
              << escaped_repeat[pair_index][i][j]);


          // CHECK FOR BYTESEQUENCE OF SMALLER BUFF SIZE BUT
          // NO SMALLER THAN SIZE 1

          //reset buffers
          std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);

          std::size_t half_seq_size =
            std::max(std::size_t(1),(escaped_seq_size[pair_index][i][j])/2);

          //setup check buffer for partial contents
          std::fill(ccheck_buf.get(),ccheck_buf.get()+cbuffsize,0xFF);
          // checkbuf laid out like:
          // padsize byteseq/2 byteseq/2 byteseq/2 padsize padsize
          std::copy(escaped_seq[pair_index][i][j],
            escaped_seq[pair_index][i][j]+half_seq_size,
            ccheck_buf.get()+padsize);


          flag = 42;
          sresult = dsv_parser_get_equiv_escaped_field_escapes_sequence(
            parser,pair_index,i,j,cbuf.get()+padsize,half_seq_size,&flag);

          BOOST_REQUIRE_MESSAGE(sresult == half_seq_size,
            "incorrect return size of pair " << pair_index
              << " replacement set " << i << " equiv bytesequence " << j
              << ": " << sresult << " != " << half_seq_size);

          BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
            ccheck_buf.get()));

          BOOST_REQUIRE_MESSAGE(flag == escaped_repeat[pair_index][i][j],
            "incorrect repeatflag for pair " << pair_index
              << " replacement set " << i << " equiv bytesequence " << j
              << " replacement set: " << i << ": " << flag << " != "
              << escaped_repeat[pair_index][i][j]);
        }

        // CHECK REPLACEMENTS
        std::size_t cbuffsize =
          replacement_seq_size[pair_index][i]+(3*padsize);
        std::unique_ptr<unsigned char[]> cbuf(new unsigned char[cbuffsize]);
        std::unique_ptr<unsigned char[]> ccheck_buf(
          new unsigned char[cbuffsize]);
        std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);
        std::fill(ccheck_buf.get(),ccheck_buf.get()+cbuffsize,0xFF);
        // checkbuf laid out like:
        // padsize byteseq padsize padsize
        std::copy(replacement_seq[pair_index][i],
          replacement_seq[pair_index][i]+replacement_seq_size[pair_index][i],
          ccheck_buf.get()+padsize);

        // CHECK FOR BYTESEQUENCE OF EXACT BUFF SIZE
        sresult = dsv_parser_get_equiv_escaped_field_escapes_replacement(parser,
          pair_index,i,cbuf.get()+padsize,replacement_seq_size[pair_index][i]);

        BOOST_REQUIRE_MESSAGE(sresult == replacement_seq_size[pair_index][i],
          "incorrect return size of pair " << pair_index
            << " replacement sequence " << i << ": "
            << sresult << " != " << replacement_seq_size[pair_index][i]);

        BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
          ccheck_buf.get()));

        // CHECK FOR BYTESEQUENCE OF LARGER BUFF SIZE
        //reset buffers
        std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);

        sresult = dsv_parser_get_equiv_escaped_field_escapes_replacement(parser,
          pair_index,i,cbuf.get()+padsize,
          replacement_seq_size[pair_index][i]+padsize);

        BOOST_REQUIRE_MESSAGE(sresult == replacement_seq_size[pair_index][i],
          "incorrect return size of pair " << pair_index
            << " replacement sequence " << i << ": "
            << sresult << " != " << replacement_seq_size[pair_index][i]);

        BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
          ccheck_buf.get()));


        // CHECK FOR BYTESEQUENCE OF SMALLER BUFF SIZE BUT
        // NO SMALLER THAN SIZE 1

        //reset buffers
        std::fill(cbuf.get(),cbuf.get()+cbuffsize,0xFF);

        std::size_t half_seq_size =
          std::max(std::size_t(1),(replacement_seq_size[pair_index][i])/2);

        //setup check buffer for partial contents
        std::fill(ccheck_buf.get(),ccheck_buf.get()+cbuffsize,0xFF);
        // checkbuf laid out like:
        // padsize byteseq/2 byteseq/2 byteseq/2 padsize padsize
        std::copy(replacement_seq[pair_index][i],
          replacement_seq[pair_index][i]+half_seq_size,
          ccheck_buf.get()+padsize);

        sresult = dsv_parser_get_equiv_escaped_field_escapes_replacement(parser,
          pair_index,i,cbuf.get()+padsize,half_seq_size);

        BOOST_REQUIRE_MESSAGE(sresult == half_seq_size,
          "incorrect return size of pair " << pair_index
            << " replacement sequence " << i << ": "
            << sresult << " != " << half_seq_size);

        BOOST_REQUIRE(std::equal(cbuf.get(),cbuf.get()+cbuffsize,
          ccheck_buf.get()));


        // CHECK REPLACEMENT REPEATS
        iresult =
          dsv_parser_get_escaped_field_escapes_repeatflag(parser,pair_index,i);

        BOOST_REQUIRE_MESSAGE(iresult == escaped_repeatflag[pair_index][i],
          "incorrect repeatflag of pair " << pair_index
            << " replacement sequence " << i << ": "
            << iresult << " != " << escaped_repeatflag[pair_index][i]);

        int inverse = !(static_cast<bool>(escaped_repeatflag[pair_index][i]));

        iresult =
          dsv_parser_set_escaped_field_escapes_repeatflag(parser,pair_index,i,
            inverse);

        iresult =
          dsv_parser_get_escaped_field_escapes_repeatflag(parser,pair_index,i);

        BOOST_REQUIRE_MESSAGE(iresult == inverse,
          "incorrect repeatflag of pair " << pair_index
            << " replacement sequence " << i << ": "
            << iresult << " != " << inverse);

        // CHECK REPLACEMENT EXCLUSIVES
        iresult =
          dsv_parser_get_escaped_field_escapes_exclusiveflag(parser,pair_index,
            i);

        BOOST_REQUIRE_MESSAGE(iresult == escaped_exclusiveflag[pair_index][i],
          "incorrect exclusivflag of pair " << pair_index
            << " replacement sequence " << i << ": "
            << iresult << " != " << escaped_exclusiveflag[pair_index][i]);

        inverse = !(static_cast<bool>(escaped_exclusiveflag[pair_index][i]));

        iresult =
          dsv_parser_set_escaped_field_escapes_exclusiveflag(parser,pair_index,
            i,inverse);

        iresult =
          dsv_parser_get_escaped_field_escapes_exclusiveflag(parser,pair_index,
            i);

        BOOST_REQUIRE_MESSAGE(iresult == inverse,
          "incorrect exclusiveflag of pair " << pair_index
            << " replacement sequence " << i << ": "
            << iresult << " != " << inverse);

      }

    }

    iresult = dsv_parser_get_field_escape_exclusiveflag(parser);

    BOOST_REQUIRE_MESSAGE(iresult == 0,
     "unexpected field escape exclusiveflag " << iresult << " != 0");
  }

}

#endif

BOOST_AUTO_TEST_SUITE_END()

