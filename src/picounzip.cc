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

#include "picounzip.hpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <time.h>
#include <utility>

#include "zlib.h"

#ifdef _WIN32
#include <direct.h> //for CreateDirectory
#else
#include <sys/stat.h> //for mkdir on posix
#endif

#if PICOUNZIP_HEADER_ONLY
#define PICOUNZIP_INLINE inline
#else
#define PICOUNZIP_INLINE
#endif
namespace picounzip {

static const int ZIP_64_EXTRA_ID = 0x0001;
static const int FILE_HEADER_SIGNATURE = 0x04034b50;
static const int DIRECTORY_HEADER_SIGNATURE = 0x02014b50;
static const int DIRECTORY_END_SIGNATURE = 0x06054b50;
static const int DIRECTORY_64_LOC_SIGNATURE = 0x07064b50;
static const int DIRECTORY_64_END_SIGNATURE = 0x06064b50;
static const int DATA_DESCRIPTOR_SIGNATURE = 0x08074b50;

class noop_reader : public detail::reader {
public:
  noop_reader(std::istream &stream) : source_stream_(stream) {}
  virtual ~noop_reader(void) {}
  virtual read_result read(char *dest, size_t dest_size) {
    source_stream_.read(dest, dest_size);
    std::streamsize readsize = source_stream_.gcount();
    return read_result(size_t(readsize), 0);
  }

private:
  std::istream &source_stream_;
  noop_reader(const noop_reader &);
};

class deflate_reader : public detail::reader {
public:
  deflate_reader(std::istream &stream) : source_stream_(stream) {
    memset(&zs_, 0, sizeof(zs_));
    if (inflateInit2(&zs_, -MAX_WBITS) != Z_OK) {
      throw unzip_error(zs_.msg);
    }
  }
  virtual ~deflate_reader(void) {
    if (inflateEnd(&zs_) != Z_OK) {
      assert(0);
    }
  }
  virtual read_result read(char *dest, size_t dest_size) {
    zs_.next_out = (unsigned char *)dest;
    zs_.avail_out = uInt(dest_size);
    int status = Z_OK;
    while (status == Z_OK && zs_.avail_out != 0) {
      if (zs_.avail_in == 0) {
        source_stream_.read(sbuffer_, SOURCE_BUFFER_SIZE);
        std::streamsize readsize = source_stream_.gcount();
        zs_.next_in = (unsigned char *)sbuffer_;
        zs_.avail_in = uInt(readsize);
      }
      status = inflate(&zs_, Z_NO_FLUSH);

      if (status < 0) {
        return read_result(0, zs_.msg);
      }
    }
    return read_result(dest_size - zs_.avail_out, 0);
  }

  static const size_t SOURCE_BUFFER_SIZE = 256;

private:
  std::istream &source_stream_;
  deflate_reader(const deflate_reader &);
  char sbuffer_[SOURCE_BUFFER_SIZE];
  z_stream zs_;
};

namespace detail {

struct ifstream_holder : stream_holder_base {
  ifstream_holder(const std::string &filepath) {
    ifs.open(filepath.c_str(), std::ifstream::binary);
  }

  virtual std::istream &stream() { return ifs; }
  std::ifstream ifs;
};

#ifdef _WIN32
PICOUNZIP_INLINE void make_directory(const std::string &dir) {
  _mkdir(dir.c_str());
}
#else
PICOUNZIP_INLINE void make_directory(const std::string &dir) {
  mkdir(dir.c_str(), 0755);
}
#endif

PICOUNZIP_INLINE bool is_big_endian() {
  union {
    uint32_t i;
    char c[4];
  } e = {0x01000000};

  return e.c[0];
}

template <typename readtype> readtype read_stream(std::istream &is) {
  char buffer[sizeof(readtype)];
  is.read(buffer, sizeof(readtype));

  if (is_big_endian()) {
    // swap endianness
    for (size_t i = 0; i < sizeof(readtype) / 2; ++i) {
      std::swap(buffer[i], buffer[sizeof(readtype) - i - 1]);
    }
  }
  readtype ret;
  memcpy(&ret, buffer, sizeof(ret));
  return ret;
}
PICOUNZIP_INLINE std::string read_stream(std::istream &is, size_t size) {
  if (size == 0) {
    return std::string();
  }
  std::vector<char> buffer;
  buffer.resize(size + 1);
  is.read(&buffer[0], buffer.size() - 1);

  return std::string(&buffer[0], buffer.size() - 1);
}

PICOUNZIP_INLINE time_t msdos_time_to_ctime(uint32_t dosDate,
                                            uint32_t dosTime) {
  struct tm t = {};
  t.tm_year = (dosDate >> 9) + 80;
  t.tm_mon = (dosDate >> 5) & 0xf;
  t.tm_mday = (dosDate & 0x1f);
  t.tm_hour = (dosTime) >> 11;
  t.tm_min = (dosTime >> 5) & 0x3f;
  t.tm_sec = (dosTime & 0x1f) * 2;
  return mktime(&t);
}

PICOUNZIP_INLINE std::istream::pos_type
end_of_central_dir_pos(std::istream &stream) {
  const int32_t BUFF_SIZE = 64;
  stream.seekg(0, std::istream::end);
  std::istream::pos_type filesize = stream.tellg();
  std::istream::pos_type readpos = filesize;
  char buf[BUFF_SIZE];
  while (readpos > std::istream::pos_type(0)) {
    int readsize = BUFF_SIZE;
    if (readpos < readsize) {
      readsize = int(readpos);
    }
    readpos -= readsize;

    stream.seekg(readpos);
    stream.read(buf, readsize);

    size_t i = readsize - 3;
    while (i-- > 0) {
      if ((buf[i] == 0x50) && (buf[i + 1] == 0x4b) && (buf[i + 2] == 0x05) &&
          (buf[i + 3] == 0x06)) {
        return readpos + std::istream::pos_type(i);
      }
    }
  }
  return 0;
}

PICOUNZIP_INLINE std::istream::pos_type
find_directory64_end(std::istream &stream, std::streampos offset) {
  static const std::streamoff directory64_end_len = 20;
  stream.seekg(offset - directory64_end_len);

  uint32_t signature = read_stream<uint32_t>(stream);
  if (signature != DIRECTORY_64_LOC_SIGNATURE) {
    return -1;
  }

  if (read_stream<uint32_t>(stream) != 0) {
    return -1;
  }

  uint64_t pos = read_stream<uint64_t>(stream);
  if (read_stream<uint32_t>(stream) != 1) {
    return -1;
  }
  return pos;
}
PICOUNZIP_INLINE bool read_directory64_end(std::istream &stream,
                                           std::streampos offset,
                                           unzip::end_of_central_dir &ret) {
  stream.seekg(offset);

  uint32_t signature = read_stream<uint32_t>(stream);
  if (signature != DIRECTORY_64_END_SIGNATURE) {
    throw unzip_error("bad zip file");
  }
  read_stream<uint64_t>(stream); // dir size
  read_stream<uint16_t>(stream); // create version
  read_stream<uint16_t>(stream); // extract version

  ret.disk_no = read_stream<uint32_t>(stream);
  ret.dir_disk_no = read_stream<uint32_t>(stream);
  ret.dir_record_this_disk = read_stream<uint64_t>(stream);
  ret.directory_records = read_stream<uint64_t>(stream);
  ret.directory_size = read_stream<uint64_t>(stream);
  ret.directory_offset = read_stream<uint64_t>(stream);
  return true;
}
PICOUNZIP_INLINE unzip::end_of_central_dir
read_directory_end(std::istream &is) {
  std::istream::pos_type eocd_pos = end_of_central_dir_pos(is);
  if (!eocd_pos) {
    throw unzip_error("bad zip file");
  }
  is.seekg(eocd_pos);

  uint32_t signature = read_stream<uint32_t>(is);
  if (signature != DIRECTORY_END_SIGNATURE) {
    throw unzip_error("bad zip file");
  }
  unzip::end_of_central_dir ret;
  ret.disk_no = read_stream<uint16_t>(is);
  ret.dir_disk_no = read_stream<uint16_t>(is);
  ret.dir_record_this_disk = read_stream<uint16_t>(is);
  ret.directory_records = read_stream<uint16_t>(is);
  ret.directory_size = read_stream<uint32_t>(is);
  ret.directory_offset = read_stream<uint32_t>(is);
  uint16_t comment_size = read_stream<uint16_t>(is);

  ret.comment = read_stream(is, comment_size);

  // zip64 file
  if (ret.directory_records == 0xffff || ret.directory_size == 0xffff ||
      ret.directory_offset == 0xffffffff) {
    std::istream::pos_type dir64_pos = find_directory64_end(is, eocd_pos);
    if (dir64_pos != std::istream::pos_type(-1)) {
      read_directory64_end(is, dir64_pos, ret);
    }
  }

  return ret;
}

PICOUNZIP_INLINE void search_zip64_size_from_extra(zip_entry &info) {

  bool need_file_size = info.file_size == 0xFFFFFFFF;
  bool need_compress_size = info.compress_size == 0xFFFFFFFF;
  bool need_header_offset = info.local_file_header_offset == 0xFFFFFFFF;
  if (info.extra.size() > 0) {
    for (std::stringstream ss(info.extra); ss.good();) {
      uint16_t tag = read_stream<uint16_t>(ss);
      int32_t size = read_stream<uint16_t>(ss);
      if (ZIP_64_EXTRA_ID == tag) {
        if (need_file_size) {
          info.file_size = read_stream<uint64_t>(ss);
          size -= 8;
          need_file_size = false;
        }
        if (need_compress_size) {
          info.compress_size = read_stream<uint64_t>(ss);
          size -= 8;
          need_compress_size = false;
        }
        if (need_header_offset) {
          info.local_file_header_offset = read_stream<uint64_t>(ss);
          size -= 8;
          need_header_offset = false;
        }
        if (size < 0) {
          throw unzip_error("bad zip file");
        }
      }
      read_stream(ss, size);
    }
  }
}

PICOUNZIP_INLINE std::vector<zip_entry>
build_entry_list(std::istream &is, const unzip::end_of_central_dir &eocd) {

  std::istream::pos_type central_dir_start = eocd.directory_offset;

  std::vector<zip_entry> ret;
  ret.reserve(eocd.directory_records);

  is.seekg(central_dir_start);
  for (size_t entry = 0; entry < eocd.directory_records; ++entry) {
    uint32_t central_dir_signature = read_stream<uint32_t>(is);
    if (central_dir_signature != DIRECTORY_HEADER_SIGNATURE) {
      throw unzip_error("bad zip file");
    }

    zip_entry info;
    info.create_version = read_stream<uint16_t>(is);
    info.extract_version = read_stream<uint16_t>(is);
    info.flag_bits = read_stream<uint16_t>(is);
    info.compress_type = read_stream<uint16_t>(is);

    uint16_t modified_time = read_stream<uint16_t>(is);
    uint16_t modified_date = read_stream<uint16_t>(is);
    info.date_time = msdos_time_to_ctime(modified_date, modified_time);

    info.CRC = read_stream<uint32_t>(is);
    info.compress_size = read_stream<uint32_t>(is);
    info.file_size = read_stream<uint32_t>(is);
    uint16_t filename_size = read_stream<uint16_t>(is);
    uint16_t ext_field_size = read_stream<uint16_t>(is);
    uint16_t filecomment_size = read_stream<uint16_t>(is);
    read_stream<uint16_t>(is);//diskno
    info.internal_attr = read_stream<uint16_t>(is);
    info.external_attr = read_stream<uint32_t>(is);
    info.local_file_header_offset = read_stream<uint32_t>(is);
    info.filename = read_stream(is, filename_size);
    info.extra = read_stream(is, ext_field_size);
    info.comment = read_stream(is, filecomment_size);

    search_zip64_size_from_extra(info);

	ret.push_back(info);
  }

  return ret;
}

PICOUNZIP_INLINE bool read_local_file_header(std::istream &is,
                                             const zip_entry &info,
                                             std::string *localextra = 0) {
  is.seekg(info.local_file_header_offset);

  uint32_t signature = detail::read_stream<uint32_t>(is);
  if (signature != 0x04034b50) {
    throw unzip_error("bad zip file");
  }
  uint16_t extract_version = detail::read_stream<uint16_t>(is);
  if (info.extract_version != extract_version) {
    // throw unzip_error("bad zip file");
  }
  uint16_t flag_bits = detail::read_stream<uint16_t>(is);
  if (info.flag_bits != flag_bits) {
    // throw unzip_error("bad zip file");
  }
  uint16_t compress_type = detail::read_stream<uint16_t>(is);
  if (info.compress_type != compress_type) {
    //   throw unzip_error("bad zip file");
  }

  uint16_t modified_time = detail::read_stream<uint16_t>(is);
  uint16_t modified_date = detail::read_stream<uint16_t>(is);
  if (info.date_time !=
      detail::msdos_time_to_ctime(modified_date, modified_time)) {
    // throw unzip_error("bad zip file");
  }
  uint32_t CRC = detail::read_stream<uint32_t>(is);
  if (info.CRC != CRC) {
    //  throw unzip_error("bad zip file");
  }
  uint32_t compress_size = detail::read_stream<uint32_t>(is);
  uint32_t file_size = detail::read_stream<uint32_t>(is);
  uint16_t file_name_size = detail::read_stream<uint16_t>(is);
  uint16_t extra_size = detail::read_stream<uint16_t>(is);
  std::string filename = detail::read_stream(is, file_name_size);
  if (info.filename != filename) {
    //   throw unzip_error("bad zip file");
  }
  std::string extra = detail::read_stream(is, extra_size);

  if (info.compress_size != compress_size) {
    //  throw unzip_error("bad zip file");
  }
  if (info.file_size != file_size) {
    //  throw unzip_error("bad zip file");
  }

  if (localextra) {
    *localextra = extra;
  }

  return true;
}
}

PICOUNZIP_INLINE unzip::unzip(std::istream &stream)
    : stream_holder_(0), stream_(stream) {
  read_header();
}

PICOUNZIP_INLINE unzip::unzip(const std::string &filepath)
    : stream_holder_(new detail::ifstream_holder(filepath)),
      stream_(stream_holder_->stream()) {
  read_header();
}

PICOUNZIP_INLINE const std::vector<zip_entry>& unzip::entrylist() {

    if (entry_list_.empty()) {
    entry_list_ = detail::build_entry_list(stream_, header_);
  }
  return entry_list_;
}
PICOUNZIP_INLINE std::vector<std::string> unzip::namelist() {
  std::vector<std::string> ret;
  const std::vector<zip_entry>&zipentry = entrylist();
  for (std::vector<zip_entry>::const_iterator it = zipentry.begin();
       it != zipentry.end(); ++it) {
    ret.push_back(it->filename);
  }
  return ret;
}
PICOUNZIP_INLINE zip_entry unzip::getentry(const std::string &name) {

	const std::vector<zip_entry>&zipentry = entrylist();
	for (std::vector<zip_entry>::const_iterator it = zipentry.begin();
		it != zipentry.end(); ++it) {
		if (it->filename == name) {
			return *it;
		}
	}
	return zip_entry();

}

PICOUNZIP_INLINE bool unzip::extract(const zip_entry &info,
                                     const std::string &path) {

  std::string outpath = path;
  if (!outpath.empty() && outpath[outpath.size() - 1] != '/' &&
      outpath[outpath.size() - 1] != '\\') {
    outpath += "/";
  }

  // do not allow absolute path by filename
  if (outpath.empty()) {
    outpath = "./";
  }
  outpath += info.filename;
  for (size_t i = 1; i < outpath.size(); ++i) {
    if (outpath[i] == '/') {
      detail::make_directory(outpath.substr(0, i));
    }
  }

  if (info.is_dir()) {
    detail::make_directory(outpath);
    return true;
  }

  std::ofstream ofs(outpath.c_str(), std::ios::binary);

  if (!ofs.is_open()) {
    throw unzip_error("can not open file:" + outpath);
  }

  unzip_file_stream ifs(*this, info);

  std::copy(std::istreambuf_iterator<char>(ifs),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(ofs));

  if (!ifs.error_message().empty()) {
    throw unzip_error(ifs.error_message());
  }
  return true;
}
PICOUNZIP_INLINE bool unzip::extract(const std::string &filename,
                                     const std::string &path) {
  return extract(getentry(filename), path);
}
PICOUNZIP_INLINE bool unzip::extractall(const std::string &path) {
  const std::vector<zip_entry>&zipentry = entrylist();

  bool ret = true;
  for (std::vector<zip_entry>::const_iterator it = zipentry.begin();
       it != zipentry.end(); ++it) {
    ret &= extract(*it, path);
  }
  return ret;
}

PICOUNZIP_INLINE bool unzip::read_header() {
  header_ = detail::read_directory_end(stream_);
  return true;
}
PICOUNZIP_INLINE bool unzip::read_entry() {


  return true;
}
PICOUNZIP_INLINE unzip_file_stream::unzip_file_stream(unzip &unzip,
                                                      const zip_entry &entry)
    : std::istream(&steam_buf_), steam_buf_(unzip.stream(), entry) {}
PICOUNZIP_INLINE
unzip_file_stream::unzip_file_stream(unzip &unzip, const std::string &filename)
    : std::istream(&steam_buf_),
      steam_buf_(unzip.stream(), unzip.getentry(filename)) {}

PICOUNZIP_INLINE unzip_file_stream::unzip_file_streambuf::unzip_file_streambuf(
    std::istream &is, const zip_entry &entry)
    : decompresser_(0), source_(is), zipentry_(entry) {
  source_.clear();
  detail::read_local_file_header(is, entry);

  remain_read_size_ = zipentry_.file_size;
  current_pos_ = source_.tellg();

  crc_ = crc32(0L, Z_NULL, 0);
}

PICOUNZIP_INLINE
    unzip_file_stream::unzip_file_streambuf::~unzip_file_streambuf() {
  delete decompresser_;
  decompresser_ = 0;
}

PICOUNZIP_INLINE void
unzip_file_stream::unzip_file_streambuf::raise_error(const std::string &what) {
  error_message_ = what;
  throw unzip_error(error_message_);
}

PICOUNZIP_INLINE unzip_file_stream::unzip_file_streambuf::int_type
unzip_file_stream::unzip_file_streambuf::underflow(void) {
  if (!decompresser_) {
    if (zipentry_.compress_type == ZIP_DEFLATED) {
      decompresser_ = new deflate_reader(source_);
    } else if (zipentry_.compress_type == ZIP_STORED) {
      decompresser_ = new noop_reader(source_);
    } else {
      raise_error("not supported compress_type");
    }
  }
  if (!error_message_.empty()) {
    throw unzip_error(error_message_);
  }
  if (egptr() <= gptr()) {
    source_.clear();
    source_.seekg(current_pos_);

    size_t readsize = BUFFER_SIZE;
    if (std::streamsize(readsize) > remain_read_size_) {
      readsize = size_t(remain_read_size_);
    }

    deflate_reader::read_result ret = decompresser_->read(dbuffer_, readsize);

    remain_read_size_ -= ret.size;
    crc_ = crc32(crc_, reinterpret_cast<unsigned char *>(dbuffer_),
                 uInt(ret.size));
    current_pos_ = source_.tellg();

    if (remain_read_size_ == 0) {
      if (crc_ != zipentry_.CRC) {
        raise_error("crc mismatch");
      }
    }

    if (ret.error) {
      raise_error(ret.error);
    } else {
      setg(dbuffer_, dbuffer_, dbuffer_ + ret.size);
    }
    if (egptr() <= gptr()) {
      return traits_type::eof();
    }
  }
  return traits_type::not_eof(*gptr());
}
}
