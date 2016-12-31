/*
 Copyright (c) 2014-2016, Michael B. Tegtmeyer. All rights reserved.

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

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>

#include <basic_scanner.h>

#include <boost/filesystem.hpp>

#include <cstdlib>
#include <iostream>
#include <chrono>

namespace d=detail;

namespace fs=boost::filesystem;

typedef d::basic_scanner<unsigned char> scanner_type;

int main(int argc, char *argv[])
{
  // make a file of random data
  std::time_t now = std::time(0);

  fs::path tmpdir = fs::temp_directory_path();
  fs::path filename = std::string("libdsv_perf_") + std::to_string(now)
    + "_scanner_pref_test.data";
  fs::path filepath = tmpdir/filename;


  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    out(std::fopen(filepath.c_str(),"wb"),&std::fclose);

  assert(out);

  std::srand(std::time(0));
  std::size_t chunk = 256;
  std::size_t total_bytes = (2<<24);
  std::vector<unsigned char> buff(chunk);
  for(std::size_t bytes=0; bytes<total_bytes; bytes+=chunk) {
    for(std::size_t i=0; i<chunk; ++i)
      buff[i] = std::rand()%256;

    std::size_t len = std::fwrite(buff.data(),1,chunk,out.get());
    assert(len == chunk);
  }

  out.reset();

  std::unique_ptr<std::FILE,int(*)(std::FILE *)>
    in(std::fopen(filepath.c_str(),"rb"),&std::fclose);

  assert(in);

  volatile int val;

  std::chrono::time_point<std::chrono::high_resolution_clock> start;
  std::chrono::time_point<std::chrono::high_resolution_clock> end;
  std::chrono::duration<double> total_seconds;

  // check baseline
  std::cout << "Warming up the cache and setting baseline\n";

  total_seconds = std::chrono::duration<double>(0);

  for(int i = 0; i<3; ++i) {
    start = std::chrono::high_resolution_clock::now();

    do {
      val = getc(in.get());
    } while(val != EOF);

    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    total_seconds += elapsed_seconds;

    std::cout << "Warm up " << i << " elapsed time: "
      << elapsed_seconds.count()
      << "s\n";

    rewind(in.get());
  }
  std::cout << "Warm up elapsed " << total_seconds.count()/3 << "s\n";



  std::cout << "\nTesting read of entire contents (likely best case)\n";

  buff = std::vector<unsigned char>(total_bytes);

  total_seconds = std::chrono::duration<double>(0);
  for(int i = 0; i<3; ++i) {
    start = std::chrono::high_resolution_clock::now();

    std::size_t len = std::fread(buff.data(),1,total_bytes,in.get());
    assert(len == total_bytes);

    std::vector<unsigned char>::iterator cur = buff.begin();
    while(cur != buff.end())
      val = *cur++;

    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    total_seconds += elapsed_seconds;

    std::cout << "Full buffer read " << i << " elapsed time: "
      << elapsed_seconds.count()
      << "s\n";

    rewind(in.get());
  }
  std::cout << "Full buffer read elapsed " << total_seconds.count()/3 << "s\n";


  std::cout << "\nTesting scanner getc with full buffer & keep cache\n";

  total_seconds = std::chrono::duration<double>(0);
  for(int i = 0; i<3; ++i) {
    scanner_type scanner(0,in.get(),total_bytes+1);

    start = std::chrono::high_resolution_clock::now();

    while((val = scanner.getc()) != EOF)
      ;

    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    total_seconds += elapsed_seconds;

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    rewind(in.get());
  }
  std::cout << "Scanner full buffer getc elapsed "
    << total_seconds.count()/3 << "s\n";



  std::cout << "\nTesting scanner getc with default buffer\n";

  total_seconds = std::chrono::duration<double>(0);
  for(int i = 0; i<3; ++i) {
    scanner_type scanner(0,in.get());

    start = std::chrono::high_resolution_clock::now();

    std::size_t count;
    val = scanner.getc();
    for(count=0; val != EOF; ++count) {
      scanner.clear_cache(1);
      val = scanner.getc();
    }

    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    total_seconds += elapsed_seconds;

    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    rewind(in.get());
  }
  std::cout << "Scanner getc elapsed " << total_seconds.count()/3 << "s\n";


  for(std::size_t read_factor = 2; read_factor < 24; ++read_factor) {

    std::size_t read_size = (2 << read_factor);
    std::cout << "\nTesting scanner getc with read_size " << read_size
      << " bytes\n";

    total_seconds = std::chrono::duration<double>(0);
    for(int i = 0; i<3; ++i) {
      scanner_type scanner(0,in.get(),read_size);

      start = std::chrono::high_resolution_clock::now();

      std::size_t count;
      val = scanner.getc();
      for(count=0; val != EOF; ++count) {
        scanner.clear_cache(1);
        val = scanner.getc();
      }

      end = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double> elapsed_seconds = end-start;
      total_seconds += elapsed_seconds;

      std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

      rewind(in.get());
    }
    std::cout
      << "------------------------------------------------------------\n"
      << "Scanner getc with read_size " << read_size
      << ", average time: "
      << total_seconds.count()/3 << "s\n";
  }


  fs::remove(filepath);

  return 0;
}
