
#include <fstream>
#include <gtest/gtest.h>

#include "picounzip.hpp"

#include "zlib.h"

class PicounzipLargeFileTest : public ::testing::Test {
protected:
  virtual void SetUp() {}
  // virtual void TearDown() {}
};

TEST_F(PicounzipLargeFileTest, LargeFile) {
  using namespace picounzip;
  // create_large_testdata.py
  {
    unzip zip("resource/large_data/large_entry.zip");
    EXPECT_EQ(size_t(100000), zip.entrylist().size());
  }
  {
    unzip zip("resource/large_data/zero_largedata.zip");
    EXPECT_EQ(uint64_t(1024) * 1024 * 1024 * 6, uint64_t(
              zip.entrylist().front().file_size));

	unzip_file_stream unzifs(zip, zip.entrylist().front());

	uint64_t streamdistance = std::distance(std::istreambuf_iterator<char>(unzifs), std::istreambuf_iterator<char>());
	EXPECT_EQ(uint64_t(1024) * 1024 * 1024 * 6, streamdistance);
  }
}