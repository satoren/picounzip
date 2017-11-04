
#include <fstream>
#include <gtest/gtest.h>

#define PICOUNZIP_HEADER_ONLY 1
#include "picounzip.hpp"

#include "zlib.h"

class picounzipTest2 : public ::testing::Test {
protected:
  virtual void SetUp() {}
  // virtual void TearDown() {}
};

TEST_F(picounzipTest2, FileEntry) {
  using namespace picounzip;

  unzip file1("resource/file_entry/file1.zip");

  zip_entry root_dir = file1.getentry("string_view-master/");
  EXPECT_EQ("string_view-master/", root_dir.filename);

  zip_entry makefile = file1.getentry("string_view-master/Makefile");
  EXPECT_EQ("string_view-master/Makefile", makefile.filename);
  EXPECT_EQ(0x3a67f42aUL, makefile.CRC);
  EXPECT_EQ(486UL, makefile.file_size);
  EXPECT_EQ(190UL, makefile.compress_size);
  EXPECT_EQ(ZIP_DEFLATED, makefile.compress_type);
}
