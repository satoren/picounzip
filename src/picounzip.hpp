#pragma once
/*
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <istream>
#include <map>
#include <string>
#include <vector>

#include <stdexcept>
#include <stdint.h>

#ifndef PICOUNZIP_USE_CPP11
#if defined(__cpp_decltype) || __cplusplus >= 201103L ||                       \
    (defined(_MSC_VER) && _MSC_VER >= 1800)
#define PICOUNZIP_USE_CPP11 1
#else
#define PICOUNZIP_USE_CPP11 0
#endif
#endif

#ifndef PICOUNZIP_HEADER_ONLY
#define PICOUNZIP_HEADER_ONLY 0
#endif

namespace picounzip {

struct stream_holder_base {
  virtual std::istream &stream() = 0;
  virtual ~stream_holder_base() {}
};

enum compress_type {
  ZIP_STORED = 0,
  ZIP_DEFLATED = 8,
};

struct unzip_error : std::runtime_error {
  explicit unzip_error(const std::string &what_arg)
      : std::runtime_error(what_arg) {}
};
struct zip_entry {
  zip_entry()
      : flag_bits(0), create_version(0), extract_version(0),
        compress_type(ZIP_STORED), date_time(0), CRC(0), compress_size(0),
        file_size(0), internal_attr(0), external_attr(0),
        local_file_header_offset(0) {}

  uint32_t flag_bits;
  uint16_t create_version;
  uint16_t extract_version;
  uint16_t compress_type;
  time_t date_time;
  uint32_t CRC;
  uint64_t compress_size;
  uint64_t file_size;
  uint32_t internal_attr;
  uint32_t external_attr;
  int64_t local_file_header_offset;

  std::string filename;
  std::string extra;
  std::string comment;

  bool is_dir() const {
    return !filename.empty() && filename[filename.size() - 1] == '/';
  }
  bool invalid() const { return filename.empty(); }
};

class unzip {
public:
  /// construct with input stream
  /// stream object required feature is read/seekg(include end position)
  explicit unzip(std::istream &stream);

#if PICOUNZIP_USE_CPP11
  template <typename stream_type> struct stream_holder : stream_holder_base {
    stream_holder(stream_type &&stream) : stream_(std::move(stream)) {}
    virtual std::istream &stream() { return stream_; }
    stream_type stream_;
  };
  /// construct with input stream(move)
  /// stream object required feature is read/seekg(include end position)
  template <typename stream_type,
            typename = typename std::enable_if<
                std::is_base_of<std::istream, stream_type>::value>::type>
  explicit unzip(stream_type &&stream)
      : stream_holder_(new stream_holder<stream_type>(std::move(stream))),
        stream_(stream_holder_->stream()) {
    read_header();
  }
#endif

  /// construct with filepath
  explicit unzip(const std::string &filepath);

  ~unzip() {
    delete stream_holder_;
    stream_holder_ = 0;
  }

  std::vector<zip_entry> entrylist();
  std::vector<std::string> namelist();
  zip_entry getentry(const std::string &name);

  bool extract(const zip_entry &info, const std::string &path = "");
  bool extract(const std::string &filename, const std::string &path = "");
  bool extractall(const std::string &path = "");

  struct end_of_central_dir {
    uint32_t disk_no;
    uint32_t dir_disk_no;
    uint64_t dir_record_this_disk;
    uint64_t directory_records;
    uint64_t directory_size;
    uint64_t directory_offset;
    std::string comment;
  };

  std::string comment() { return header_.comment; }

  std::istream &stream() { return stream_; };

private:
  unzip(const unzip &);
  stream_holder_base *stream_holder_;
  std::istream &stream_;
  end_of_central_dir header_;

  // for entry cache
  bool read_header();
  bool read_entry();
  const std::map<std::string, zip_entry> &info_map();
  std::map<std::string, zip_entry> info_map_;
};

namespace detail {

struct reader {
  virtual ~reader(void) {}
  struct read_result {
    read_result() : size(0), error(0) {}
    read_result(size_t s, const char *e) : size(s), error(e) {}
    read_result(const read_result &src) : size(src.size), error(src.error) {}
    size_t size;
    const char *error;
  };
  virtual read_result read(char *dest, size_t dest_size) = 0;
};
}
class unzip_file_stream : public std::istream {
public:
  unzip_file_stream(unzip &unzip, const zip_entry &entry);
  unzip_file_stream(unzip &unzip, const std::string &filename);

  const std::string &error_message() const {
    return steam_buf_.error_message_;
  };

private:
  class unzip_file_streambuf : public std::streambuf {
  public:
    unzip_file_streambuf(std::istream &is, const zip_entry &entry);
    virtual ~unzip_file_streambuf();

    virtual int_type underflow(void);

    void raise_error(const std::string &what);

    detail::reader *decompresser_;
    std::istream &source_;

    std::streampos current_pos_;
    std::streamsize remain_read_size_;

    uint32_t crc_;

    zip_entry zipentry_;

    std::string error_message_;

    static const size_t BUFFER_SIZE = 4096;
    char dbuffer_[BUFFER_SIZE];
  };

  unzip_file_streambuf steam_buf_;
};
}
#if PICOUNZIP_HEADER_ONLY
#include "picounzip.cc"
#endif
