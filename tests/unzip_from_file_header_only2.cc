
#include <fstream>
#include <gtest/gtest.h>

#include "picounzip.hpp"

#include "zlib.h"

#if PICOUNZIP_USE_CPP11

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
  EXPECT_EQ(0x3a67f42a, makefile.CRC);
  EXPECT_EQ(486, makefile.file_size);
  EXPECT_EQ(190, makefile.compress_size);
  EXPECT_EQ(ZIP_DEFLATED, makefile.compress_type);
}

TEST_F(picounzipTest2, ExtractFromMovedStream) {
  using namespace picounzip;
  {
    unzip dont_add_dirent(
        std::ifstream("resource/extract/def.zip", std::ios::binary));
    EXPECT_EQ(true, dont_add_dirent.extractall("output/def"));
  }
  {
    unzip defaultopt(
        std::ifstream("resource/extract/default.zip", std::ios::binary));
    EXPECT_EQ(true, defaultopt.extractall("output/default"));
  }
  {
    unzip storeonly(
        std::ifstream("resource/extract/storeonly.zip", std::ios::binary));
    EXPECT_EQ(true, storeonly.extractall("output/storeonly"));
  }
  {
    unzip addcomment(
        std::ifstream("resource/extract/addcomment.zip", std::ios::binary));
    EXPECT_EQ(true, addcomment.extractall("output/addcomment"));
    EXPECT_EQ("zip comment", addcomment.comment());
  }
  {
    unzip bettercompress(
        std::ifstream("resource/extract/bettercompress.zip", std::ios::binary));
    EXPECT_EQ(true, bettercompress.extractall("output/bettercompress"));
  }
  {
    unzip dont_add_dirent(std::ifstream("resource/extract/dont_add_dirent.zip",
                                        std::ios::binary));
    EXPECT_EQ(true, dont_add_dirent.extractall("output/dont_add_dirent"));
  }
  {
    unzip create_by_macos_finder("resource/extract/create_by_macos_finder.zip");
    EXPECT_EQ(
        true,
        create_by_macos_finder.extractall("output/create_by_macos_finder"));
  }
}

#endif // PICOUNZIP_USE_CPP11