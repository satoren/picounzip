
#include <fstream>
#include <gtest/gtest.h>

#define PICOUNZIP_HEADER_ONLY 1
#include "picounzip.hpp"

#include "zlib.h"

class picounzipTest : public ::testing::Test {
protected:
  virtual void SetUp() {}
  // virtual void TearDown() {}
};

TEST_F(picounzipTest, FileEntry) {
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

TEST_F(picounzipTest, Extract) {
  using namespace picounzip;
  {
    std::ifstream ifs("resource/extract/def.zip", std::ios::binary);
    EXPECT_TRUE(ifs.is_open());
  }
  {
    unzip dont_add_dirent("resource/extract/def.zip");
    dont_add_dirent.extractall("output/def");
  }
  {
    unzip defaultopt("resource/extract/default.zip");
    defaultopt.extractall("output/default");
  }
  {
    unzip storeonly("resource/extract/storeonly.zip");
    storeonly.extractall("output/storeonly");
  }
  {
    unzip addcomment("resource/extract/addcomment.zip");
    addcomment.extractall("output/addcomment");
    EXPECT_EQ("zip comment", addcomment.comment());
  }
  {
    unzip bettercompress("resource/extract/bettercompress.zip");
    bettercompress.extractall("output/bettercompress");
  }
  {
    unzip dont_add_dirent("resource/extract/dont_add_dirent.zip");
    dont_add_dirent.extractall("output/dont_add_dirent");
  }
  {
    unzip create_by_macos_finder("resource/extract/create_by_macos_finder.zip");
    create_by_macos_finder.extractall("output/create_by_macos_finder");
  }
}
