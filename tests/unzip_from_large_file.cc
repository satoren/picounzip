
#include <fstream>
#include <gtest/gtest.h>

#define PICOUNZIP_HEADER_ONLY 1
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
  if (std::ifstream("resource/large_data/large_entry.zip").is_open()) {
    unzip zip("resource/large_data/large_entry.zip");
    EXPECT_EQ(true, zip.extractall("output/large_entry"));
    EXPECT_EQ(100000, zip.entrylist().size());
  }
  if (std::ifstream("resource/large_data/zero_largedata.zip").is_open()) {
    unzip zip("resource/large_data/zero_largedata.zip");
    EXPECT_EQ(true, zip.extractall("output/zero_largedata"));
    EXPECT_EQ(uint64_t(1024) * 1024 * 1024 * 6,
              zip.entrylist().front().file_size);
  }
}