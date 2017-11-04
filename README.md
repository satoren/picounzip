# picounzip
extract ZIP archive library for c++
[![Build Status](https://travis-ci.org/satoren/picounzip.svg?branch=master)](https://travis-ci.org/satoren/picounzip)
[![Build status](https://ci.appveyor.com/api/projects/status/qqwfqbltcky42um6/branch/master?svg=true)](https://ci.appveyor.com/project/satoren/picounzip/branch/master)


Licensed under [Boost Software License](http://www.boost.org/LICENSE_1_0.txt)

## Requirements
- zlib
- C++03 compiler (some limitation) + (stdint.h header) or later.

## Introduction
Extract zip archive for C++

## Usage
* add zlib to link library.
* add "src" directory to "header search path" of your project.

* compile and link for picounzip.cc or header only define `PICOUNZIP_HEADER_ONLY` to 1

### small example

* code
```c++
#include <stdio.h>
#define PICOUNZIP_HEADER_ONLY 1
#include "picounzip.hpp"
int main(int argc,char* argv[]){
  using namespace picounzip;
  if(argc != 3){
	  printf("Usage: %s zipfile destdir",argv[0]);
	  std::cout << "Usage:" << argv[0]  << " zipfile destdir" << std::endl;
  }
  unzip zip(argv[1]);

  //extract all file to destdir
  zip.extractall(argv[2]);
}
```
* compile
```bash
g++ -I/path/to/picounzip/src picounzip_example.cc -lz -o picounzip_example
```

### get zip entry
```
  using namespace picounzip;
  unzip zipfile("myzipfile.zip");
  zip_entry file_entry = zipfile.getentry("file.txt");
  std::cout << "file.txt size is " << file_entry.file_size << std::endl;
```

### iterate zip entry
```
  using namespace picounzip;
  unzip zipfile("myzipfile.zip");
  
  std::cout << "archived file list" << std::endl;
  for(const zip_entry& entry : zipfile.entrylist()){
    std::cout << entry.filename << std::endl;
  }
```


### extract with stream
```
  using namespace picounzip;
  unzip zipfile("myzipfile.zip");
  unzip_file_stream zipfilestream(zipfile,"file.txt");

  std::string s;
  // copy to string
  zipfilestream >> s;
```
