#include <boost/test/unit_test.hpp>

#include "test_detail.h"

#include <equiv_bytesequence.h>

#include <iostream>

/** \file
 *  \brief Unit tests the scanner
 */


namespace detail {

inline bool operator==(const byte_chunk &lhs, const byte_chunk &rhs)
{
  return (lhs.byte == rhs.byte && lhs.accept == rhs.accept &&
    lhs.pass_skip == rhs.pass_skip && lhs.fail_skip == rhs.fail_skip);
}

inline bool operator!=(const byte_chunk &lhs, const byte_chunk &rhs)
{
  return !(lhs==rhs);
}

}


namespace dsv {
namespace test {

typedef ::detail::basic_equiv_bytesequence<unsigned char> equiv_bytesequence;
typedef equiv_bytesequence::byteseq_desc bytesequence_desc;

namespace d=::detail;




void print_repeats(const std::vector<bytesequence_desc> &delim_desc_vec)
{
  std::cerr << "delim_desc:\n";
  for(std::size_t i=0; i<delim_desc_vec.size(); ++i) {
    std::cerr << "\t" << delim_desc_vec[i].normalized_seq_bytes;
    if(delim_desc_vec[i].repeat)
      std::cerr << " [repeat]";
    std::cerr << "\n";
  }
  std::cerr << "\n";
}

// first is base, second is normalized
std::string
output_byteseq_desc_eval(const std::vector<bytesequence_desc> &desc_vec,
  const std::vector<std::pair<std::string,std::string> > &val_vec)
{
  std::stringstream out;

  std::size_t cur;
  for(cur=0; cur < val_vec.size() && cur < desc_vec.size(); ++cur) {
    out << "\trequired base: [" << val_vec[cur].first  << "] received: "
      << "[" << desc_vec[cur].base_seq_bytes << "]\n";
    out << "\trequired normalized: " << "[" << val_vec[cur].second << "] "
      "received: ["  << desc_vec[cur].normalized_seq_bytes << "]\n";

  }

  out << "Additional received:\n";
  for(;cur < desc_vec.size(); ++cur) {
    out << "\tbase:" << desc_vec[cur].base_seq_bytes << "\n";
    out << "\normalized:" << desc_vec[cur].normalized_seq_bytes << "\n";
  }

  out << "Additional required:\n";
  for(;cur < val_vec.size(); ++cur) {
    out << "\tbase:" << val_vec[cur].first << "\n";
    out << "\normalized:" << val_vec[cur].second << "\n";
  }

  return out.str();
}

std::string
output_byte_chunk_eval(const std::vector<d::byte_chunk> &required,
  const std::vector<d::byte_chunk> &received)
{
  std::stringstream out;

  std::size_t cur;
  out << "Contents:\n";
  for(cur = 0; cur < required.size() && cur < received.size(); ++cur) {
    out << "\t";
    if(required[cur] != received[cur])
      out << "*";
    else
      out << " ";

    out << "required [" << detail::print_byte(required[cur].byte) << ","
      << (required[cur].accept?"    accept,":"not-accept,")
      << required[cur].pass_skip << "," << required[cur].fail_skip << "]";
    out << " received [" << detail::print_byte(received[cur].byte) << ","
      << (received[cur].accept?"    accept,":"not-accept,")
      << received[cur].pass_skip << "," << received[cur].fail_skip << "]\n";
  }

  if(cur < required.size()) {
    out << "Additional required:\n";
    for(std::size_t i=cur; i<required.size(); ++i) {
      out << "\trequired [" << detail::print_byte(required[i].byte) << ","
        << (required[i].accept?"    accept,":"not-accept,")
        << required[i].pass_skip << "," << required[i].fail_skip << "]\n";
    }
  }

  if(cur < received.size()) {
    out << "Additional received:\n";
    for(std::size_t i=cur; i<received.size(); ++i) {
      out << " received [" << detail::print_byte(received[i].byte) << ","
        << (received[i].accept?"    accept,":"not-accept,")
        << received[i].pass_skip << "," << received[i].fail_skip << "]\n";
    }
  }

  return out.str();
}

std::vector<bytesequence_desc>
make_bytesequence_vec(const std::vector<std::pair<std::string,bool> > &contents)
{
  std::vector<bytesequence_desc> result;

  for(std::size_t i=0; i<contents.size(); ++i) {
    result.emplace_back(contents[i].first.begin(),
      contents[i].first.end(),contents[i].second);
  }

  return result;
}

// first is base, second is normalized
bool check_byteseq_contents(const std::vector<bytesequence_desc> &desc_vec,
  const std::vector<std::pair<std::string,std::string> > &val_vec)
{
  if(desc_vec.size() != val_vec.size())
    return false;

  for(std::size_t i=0; i<desc_vec.size(); ++i) {
    if(!(desc_vec[i].base_seq_bytes.size() == val_vec[i].first.size() &&
      desc_vec[i].normalized_seq_bytes.size() == val_vec[i].second.size()))
    {
      return false;
    }

    if(!std::equal(desc_vec[i].base_seq_bytes.begin(),
      desc_vec[i].base_seq_bytes.end(),val_vec[i].first.begin()))
    {
      return false;
    }

    if(!std::equal(desc_vec[i].normalized_seq_bytes.begin(),
      desc_vec[i].normalized_seq_bytes.end(),val_vec[i].second.begin()))
    {
      return false;
    }
  }

  return true;
}



BOOST_AUTO_TEST_SUITE( bytesequence_compare_suite )


// CHECK SEQUENCE NORMALIZATION

/**
    \test Check for normalization of single character nonrepeat
 */
BOOST_AUTO_TEST_CASE( single_char_normalization_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",false}
  };

  std::vector<bytesequence_desc> desc_vec =
    make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","f"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_char_normalization_nonrepeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of trivial case does not do anything
 */
BOOST_AUTO_TEST_CASE( trivial_normalization_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foo"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed trivial_normalization_nonrepeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of single character nonrepeat
 */
BOOST_AUTO_TEST_CASE( single_char_normalization_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","f"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_char_normalization_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of trivial case does not do anything
 */
BOOST_AUTO_TEST_CASE( trivial_normalization_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foo"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed trivial_normalization_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of 2 disjoint single input
 */
BOOST_AUTO_TEST_CASE( single_disjoint2_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",false},
    {"b",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","f"},
      {"b","b"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_disjoint2_nonrepeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of 2 disjoint trivial input
 */
BOOST_AUTO_TEST_CASE( trivial_disjoint2_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",false},
    {"bar",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foo"},
      {"bar","bar"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed trivial_disjoint2_nonrepeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of single shared nonrepeat
 */
BOOST_AUTO_TEST_CASE( single_shared_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",false},
    {"foo",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","f"},
      {"foo","foo"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_shared_nonrepeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of single shared single repeat
 */
BOOST_AUTO_TEST_CASE( single_shared_single_repeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",true},
    {"foo",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","ff"},
      {"foo","foo"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_shared_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of single shared
 */
BOOST_AUTO_TEST_CASE( single_shared_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",true},
    {"foo",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","ff"},
      {"foo","foofoo"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_shared_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of inverted single shared single repeat
 */
BOOST_AUTO_TEST_CASE( inverted_single_shared_single_repeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"f",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foofoo"},
      {"f","f"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed inverted_single_shared_single_repeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of inverted single shared
 */
BOOST_AUTO_TEST_CASE( inverted_single_shared_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"f",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foofoo"},
      {"f","ff"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed inverted_single_shared_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of complex two
 */
BOOST_AUTO_TEST_CASE( complex_two_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"foofy",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foofoofoo"},
      {"foofy","foofyfoofy"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed complex_two_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of many disjoint single nonrepeat
 */
BOOST_AUTO_TEST_CASE( single_disjoint_many_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",false},
    {"b",false},
    {"a",false},
    {"c",false},
    {"d",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","f"},
      {"b","b"},
      {"a","a"},
      {"c","c"},
      {"d","d"},
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_disjoint_many_nonrepeat_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of many disjoint single
 */
BOOST_AUTO_TEST_CASE( single_disjoint_many_single_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"f",true},
    {"b",true},
    {"a",true},
    {"c",true},
    {"d",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"f","ff"},
      {"b","bb"},
      {"a","aa"},
      {"c","cc"},
      {"d","dd"},
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed single_disjoint_many_single_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}


/**
    \test Check for normalization of complex many
 */
BOOST_AUTO_TEST_CASE( complex_many_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"foofy",true},
    {"foofyfoo",true},
    {"foofyfoobar",true},
    {"foofyfoofoobar",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foo","foofoofoo"},
      {"foofy","foofyfoofyfoofy"},
      {"foofyfoo","foofyfoofoofyfoofoofyfoo"},
      {"foofyfoobar","foofyfoobarfoofyfoobar"},
      {"foofyfoofoobar","foofyfoofoobarfoofyfoofoobar"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed complex_many_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}

/**
    \test Check for normalization of inverted complex many
 */
BOOST_AUTO_TEST_CASE( complex_inverted_many_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foofyfoofoobar",true},
    {"foofyfoobar",true},
    {"foofyfoo",true},
    {"foofy",true},
    {"foo",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  d::normalize_seq(desc_vec.begin(),desc_vec.end());

  std::vector<std::pair<std::string,std::string> > req_res{
      {"foofyfoofoobar","foofyfoofoobarfoofyfoofoobar"},
      {"foofyfoobar","foofyfoobarfoofyfoobar"},
      {"foofyfoo","foofyfoofoofyfoofoofyfoo"},
      {"foofy","foofyfoofyfoofy"},
      {"foo","foofoofoo"}
    };

  BOOST_REQUIRE_MESSAGE(check_byteseq_contents(desc_vec,req_res),
    "Failed complex_inverted_many_test:\n"
      << output_byteseq_desc_eval(desc_vec,req_res));
}



// CHECK ASSIGN BYTES

/**
    \test Check byte sequence assignment for single char input nonrepeat
 */
BOOST_AUTO_TEST_CASE( byte_assign_single_nonrepeat_test )
{
  std::vector<unsigned char> in{
    'f'
  };

  std::vector<d::byte_chunk> comp_bytes = d::assign_bytes(in.begin(),
    in.end(),in.size(),false);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',1,0,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed byte_assign_single_nonrepeat_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check byte sequence assignment for single char input
 */
BOOST_AUTO_TEST_CASE( byte_assign_single_test )
{
  std::vector<unsigned char> in{
    'f'
  };

  std::vector<d::byte_chunk> comp_bytes = d::assign_bytes(in.begin(),
    in.end(),in.size(),true);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',1,0,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed byte_assign_single_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check byte sequence assignment for input nonrepeat
 */
BOOST_AUTO_TEST_CASE( byte_assign_nonrepeat_test )
{
  std::vector<unsigned char> in{
    'f','o','o'
  };

  std::vector<d::byte_chunk> comp_bytes = d::assign_bytes(in.begin(),
    in.end(),in.size(),false);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,0,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed byte_assign_nonrepeat_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check byte sequence assignment for input
 */
BOOST_AUTO_TEST_CASE( byte_assign_test )
{
  std::vector<unsigned char> in{
    'f','o','o'
  };

  std::vector<d::byte_chunk> comp_bytes = d::assign_bytes(in.begin(),
    in.end(),in.size(),true);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-std::ptrdiff_t(in.size()-1),0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed byte_assign_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check byte sequence assignment for repeated input
 */
BOOST_AUTO_TEST_CASE( byte_assign_repeat_input_test )
{
  std::vector<unsigned char> in{
    'f','o','o','f','o','o','f','o','o'
  };

  std::vector<d::byte_chunk> comp_bytes = d::assign_bytes(in.begin(),
    in.end(),3,true);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed byte_assign_repeat_input_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}


// CHECK COMPILE_BYTES

/**
    \test Check compile_bytes with single base byte_sequence
 */
BOOST_AUTO_TEST_CASE( compile_bytes_single_disjoint_nonrepeat_test )
{
  std::vector<d::byte_chunk> comp_bytes{
    {'f',1,0,0}
  };

  std::string base_bytes = "b";
  std::string in_bytes = base_bytes;

  d::compile_bytes(in_bytes.begin(),in_bytes.end(),base_bytes.size(),false,
    comp_bytes);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',1,0,1},
    {'b',1,0,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_bytes_single_disjoint_nonrepeat_test:\n"
       << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check compile_bytes with single base byte_sequence
 */
BOOST_AUTO_TEST_CASE( compile_bytes_disjoint_nonrepeat_test )
{
  std::vector<d::byte_chunk> comp_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,0,0}
  };

  std::string base_bytes = "bar";
  std::string in_bytes = base_bytes;

  d::compile_bytes(in_bytes.begin(),in_bytes.end(),base_bytes.size(),false,
    comp_bytes);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,3},
    {'o',0,1,0},
    {'o',1,0,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,0,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_bytes_disjoint_nonrepeat_test:\n"
       << output_byte_chunk_eval(expected_bytes,comp_bytes));
}




/**
    \test Check compile_bytes with single base byte_sequence nonrepeat
 */
BOOST_AUTO_TEST_CASE( compile_bytes_single_nonrepeat_nonrepeat_test )
{
  std::vector<d::byte_chunk> comp_bytes{
    {'f',1,0,0},
  };

  std::string base_bytes = "foo";
  std::string in_bytes = base_bytes;

  d::compile_bytes(in_bytes.begin(),in_bytes.end(),base_bytes.size(),false,
    comp_bytes);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',1,1,0},
    {'o',0,1,0},
    {'o',1,0,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_bytes_single_nonrepeat_nonrepeat_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check compile_bytes with single base byte_sequence
 */
BOOST_AUTO_TEST_CASE( compile_bytes_single_repeat_nonrepeat_test )
{
  std::vector<d::byte_chunk> comp_bytes{
    {'f',1,1,0},
    {'f',1,1,0},
    {'f',1,-1,0},
  };

  std::string base_bytes = "foof";
  std::string in_bytes = base_bytes;

  d::compile_bytes(in_bytes.begin(),in_bytes.end(),base_bytes.size(),false,
    comp_bytes);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',1,1,0},
    {'f',1,1,2},
    {'f',1,-1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'f',1,0,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_bytes_single_repeat_nonrepeat_test:\n"
       << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check compile_bytes with single base byte_sequence
 */
BOOST_AUTO_TEST_CASE( compile_bytes_single_repeat_repeat_test )
{
  std::vector<d::byte_chunk> comp_bytes{
    {'f',1,1,0},
    {'f',1,1,0},
    {'f',1,-1,0},
  };

  std::string base_bytes = "foof";
  std::string in_bytes = base_bytes + base_bytes;

  d::compile_bytes(in_bytes.begin(),in_bytes.end(),base_bytes.size(),true,
    comp_bytes);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',1,1,0},
    {'f',1,1,2},
    {'f',1,-1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'f',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'f',1,-3,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_bytes_single_repeat_repeat_test:\n"
       << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check compile_bytes with single base byte_sequence
 */
BOOST_AUTO_TEST_CASE( compile_bytes_shorter_nonrepeat_input_nonrepeat_test )
{
  std::vector<d::byte_chunk> comp_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'f',0,1,0},
    {'y',1,0,0},
  };

  std::string base_bytes = "foof";
  std::string in_bytes = base_bytes;

  d::compile_bytes(in_bytes.begin(),in_bytes.end(),base_bytes.size(),false,
    comp_bytes);

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'f',1,1,0},
    {'y',1,0,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_bytes_shorter_nonrepeat_input_nonrepeat_test:\n"
       << output_byte_chunk_eval(expected_bytes,comp_bytes));
}



// CHECK COMPILE_SEQ

/**
    \test Check single compiled sequence
 */
BOOST_AUTO_TEST_CASE( compile_single_byteseq_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_single_byteseq_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check multiple nonrepeating disjoint sequences
 */
BOOST_AUTO_TEST_CASE( compile_disjoint_byteseq_nonrepeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",false},
    {"bar",false},
    {"hello",false},
    {"world",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,3},
    {'o',0,1,0},
    {'o',1,0,0},
    {'b',0,1,3},
    {'a',0,1,0},
    {'r',1,0,0},
    {'h',0,1,5},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,0,0},
    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,0,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_disjoint_byteseq_nonrepeat_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check multiple repeating disjoint sequences
 */
BOOST_AUTO_TEST_CASE( compile_disjoint_byteseq_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"bar",true},
    {"hello",true},
    {"world",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,6},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},

    {'b',0,1,6},
    {'a',0,1,0},
    {'r',1,1,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,-2,0},

    {'h',0,1,10},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,1,0},
    {'h',0,1,0},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,-4,0},

    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,1,0},
    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,-4,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_disjoint_byteseq_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check overlapping sequences with no repeat
 */
BOOST_AUTO_TEST_CASE( compile_byteseq_overlapping_no_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",false},
    {"foobar",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,1,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,0,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_byteseq_overlapping_no_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check overlapping sequences with part repeat
 */
BOOST_AUTO_TEST_CASE( compile_byteseq_overlapping_part_repeat_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"foobar",false}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,6},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,0,0},
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_byteseq_overlapping_part_repeat_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check overlapping sequences with nonoverlapping repeat
 */
BOOST_AUTO_TEST_CASE( compile_byteseq_overlapping_nonoverlapping_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"foobar",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,3},
    {'o',0,1,0},
    {'o',1,-2,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,-5,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_byteseq_overlapping_nonoverlapping_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}

/**
    \test Check overlapping sequences with overlapping repeat
 */
BOOST_AUTO_TEST_CASE( compile_byteseq_overlapping_overlapping_test )
{
  std::vector<std::pair<std::string,bool> > input{
    {"foo",true},
    {"foofy",true}
  };

  std::vector<bytesequence_desc> desc_vec = make_bytesequence_vec(input);

  std::vector<d::byte_chunk> comp_bytes = d::compile_seq(desc_vec.begin(),
    desc_vec.end());

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,5},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},
    {'y',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',0,1,0},
    {'f',0,1,0},
    {'y',1,-4,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed compile_byteseq_overlapping_overlapping_test:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));
}



/**
    \test Basic object checks
 */
BOOST_AUTO_TEST_CASE( basic_equiv_bytesequence_object_creation_check )
{
  // check default object creation
  equiv_bytesequence ebyteseq;

  BOOST_REQUIRE(ebyteseq.byteseq_desc_vec().empty());
  BOOST_REQUIRE(ebyteseq.repeatflag() == false);
  BOOST_REQUIRE(ebyteseq.exclusiveflag() == false);
  BOOST_REQUIRE(ebyteseq.compiled_seq_vec().empty());
  BOOST_REQUIRE(!ebyteseq.effective_byteseq());
}

BOOST_AUTO_TEST_CASE( basic_equiv_bytesequence_object_check )
{
  const unsigned char bytes0[] = {'f','o','o'};
  const unsigned char bytes1[] = {'b','a','r'};
  const unsigned char bytes2[] = {'h','e','l','l','o'};
  const unsigned char bytes3[] = {'w','o','r','l','d'};

  const unsigned char *bytes[5] = {bytes0,bytes1,bytes2,bytes3};

  std::size_t bytelen[] = {3,3,5,5};
  int seq_repeat[] = {true,true,true,true};

  // check default object creation
  equiv_bytesequence ebyteseq(bytes,bytelen,seq_repeat,4,false,true);

  BOOST_REQUIRE(ebyteseq.byteseq_desc_vec().size() == 4);

  for(std::size_t i=0; i<4; ++i) {
    // lengths may be different so use this form instead of std::equal
    equiv_bytesequence::bytesequence_type byteseq(bytes[i],bytes[i]+bytelen[i]);

    const bytesequence_desc &desc = ebyteseq.byteseq_desc_vec().at(i);
    BOOST_REQUIRE(desc.base_seq_bytes == byteseq);
    BOOST_REQUIRE(desc.repeat == seq_repeat[i]);
  }

  BOOST_REQUIRE(ebyteseq.repeatflag() == false);
  BOOST_REQUIRE(ebyteseq.exclusiveflag() == true);

  const equiv_bytesequence::byte_chunk_vec_type &comp_bytes =
    ebyteseq.compiled_seq_vec();

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,6},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},

    {'b',0,1,6},
    {'a',0,1,0},
    {'r',1,1,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,-2,0},

    {'h',0,1,10},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,1,0},
    {'h',0,1,0},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,-4,0},

    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,1,0},
    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,-4,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed to produce expected compiled bytesequence:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));

  std::shared_ptr<const equiv_bytesequence::bytesequence_type> eff_byteseq =
    ebyteseq.effective_byteseq();

  BOOST_REQUIRE_MESSAGE(!(eff_byteseq),
    "Nonempty effective bytesequence for unset equiv_byteseq");
}

// schedule after basic object creation check
BOOST_AUTO_TEST_CASE( basic_equiv_bytesequence_object_copy_check )
{
  const unsigned char bytes0[] = {'f','o','o'};
  const unsigned char bytes1[] = {'b','a','r'};
  const unsigned char bytes2[] = {'h','e','l','l','o'};
  const unsigned char bytes3[] = {'w','o','r','l','d'};

  const unsigned char *bytes[5] = {bytes0,bytes1,bytes2,bytes3};

  std::size_t bytelen[] = {3,3,5,5};
  int seq_repeat[] = {true,true,true,true};

  equiv_bytesequence tmp(bytes,bytelen,seq_repeat,4,false,true);

  // check for copy constructor
  equiv_bytesequence ebyteseq(tmp);

  BOOST_REQUIRE(ebyteseq.byteseq_desc_vec().size() == 4);

  for(std::size_t i=0; i<4; ++i) {
    // lengths may be different so use this form instead of std::equal
    equiv_bytesequence::bytesequence_type byteseq(bytes[i],bytes[i]+bytelen[i]);

    const bytesequence_desc &desc = ebyteseq.byteseq_desc_vec().at(i);
    BOOST_REQUIRE(desc.base_seq_bytes == byteseq);
    BOOST_REQUIRE(desc.repeat == seq_repeat[i]);
  }

  BOOST_REQUIRE(ebyteseq.repeatflag() == false);
  BOOST_REQUIRE(ebyteseq.exclusiveflag() == true);

  const equiv_bytesequence::byte_chunk_vec_type &comp_bytes =
    ebyteseq.compiled_seq_vec();

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,6},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},

    {'b',0,1,6},
    {'a',0,1,0},
    {'r',1,1,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,-2,0},

    {'h',0,1,10},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,1,0},
    {'h',0,1,0},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,-4,0},

    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,1,0},
    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,-4,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed to produce expected compiled bytesequence:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));

  std::shared_ptr<const equiv_bytesequence::bytesequence_type> eff_byteseq =
    ebyteseq.effective_byteseq();

  BOOST_REQUIRE_MESSAGE(!(eff_byteseq),
    "Nonempty effective bytesequence for unset equiv_byteseq");
}

// schedule after basic object creation check
BOOST_AUTO_TEST_CASE( basic_equiv_bytesequence_object_assignment_check )
{
  const unsigned char bytes0[] = {'f','o','o'};
  const unsigned char bytes1[] = {'b','a','r'};
  const unsigned char bytes2[] = {'h','e','l','l','o'};
  const unsigned char bytes3[] = {'w','o','r','l','d'};

  const unsigned char *bytes[5] = {bytes0,bytes1,bytes2,bytes3};

  std::size_t bytelen[] = {3,3,5,5};
  int seq_repeat[] = {true,true,true,true};

  equiv_bytesequence tmp(bytes,bytelen,seq_repeat,4,false,true);

  // check for copy constructor
  equiv_bytesequence ebyteseq;
  ebyteseq = tmp;

  BOOST_REQUIRE(ebyteseq.byteseq_desc_vec().size() == 4);

  for(std::size_t i=0; i<4; ++i) {
    // lengths may be different so use this form instead of std::equal
    equiv_bytesequence::bytesequence_type byteseq(bytes[i],bytes[i]+bytelen[i]);

    const bytesequence_desc &desc = ebyteseq.byteseq_desc_vec().at(i);
    BOOST_REQUIRE(desc.base_seq_bytes == byteseq);
    BOOST_REQUIRE(desc.repeat == seq_repeat[i]);
  }

  BOOST_REQUIRE(ebyteseq.repeatflag() == false);
  BOOST_REQUIRE(ebyteseq.exclusiveflag() == true);

  const equiv_bytesequence::byte_chunk_vec_type &comp_bytes =
    ebyteseq.compiled_seq_vec();

  std::vector<d::byte_chunk> expected_bytes{
    {'f',0,1,6},
    {'o',0,1,0},
    {'o',1,1,0},
    {'f',0,1,0},
    {'o',0,1,0},
    {'o',1,-2,0},

    {'b',0,1,6},
    {'a',0,1,0},
    {'r',1,1,0},
    {'b',0,1,0},
    {'a',0,1,0},
    {'r',1,-2,0},

    {'h',0,1,10},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,1,0},
    {'h',0,1,0},
    {'e',0,1,0},
    {'l',0,1,0},
    {'l',0,1,0},
    {'o',1,-4,0},

    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,1,0},
    {'w',0,1,0},
    {'o',0,1,0},
    {'r',0,1,0},
    {'l',0,1,0},
    {'d',1,-4,0}
  };

  BOOST_REQUIRE_MESSAGE(expected_bytes==comp_bytes,
    "Failed to produce expected compiled bytesequence:\n"
      << output_byte_chunk_eval(expected_bytes,comp_bytes));

  std::shared_ptr<const equiv_bytesequence::bytesequence_type> eff_byteseq =
    ebyteseq.effective_byteseq();

  BOOST_REQUIRE_MESSAGE(!(eff_byteseq),
    "Nonempty effective bytesequence for unset equiv_byteseq");
}



BOOST_AUTO_TEST_SUITE_END()

}
}
