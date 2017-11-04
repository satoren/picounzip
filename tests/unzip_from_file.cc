
#include <fstream>
#include <gtest/gtest.h>

#include "picounzip.hpp"

#include "zlib.h"

#if PICOUNZIP_USE_CPP11

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
  EXPECT_EQ(0x3a67f42aU, makefile.CRC);
  EXPECT_EQ(486U, makefile.file_size);
  EXPECT_EQ(190U, makefile.compress_size);
  EXPECT_EQ(ZIP_DEFLATED, makefile.compress_type);
}

TEST_F(picounzipTest, ExtractFromMovedStream) {
  using namespace picounzip;
  {
    unzip dont_add_dirent(
        std::ifstream("resource/extract/def.zip", std::ios::binary));
    dont_add_dirent.extractall("output/def");
	//check extracted file
	std::ifstream fdef("output/def/def.txt", std::ios::binary);
	std::string s;
	fdef >> s;
	EXPECT_EQ("aaaaaa", s);
  }
  {
    unzip defaultopt(
        std::ifstream("resource/extract/default.zip", std::ios::binary));
    defaultopt.extractall("output/default");
  }
  {
    unzip storeonly(
        std::ifstream("resource/extract/storeonly.zip", std::ios::binary));
    storeonly.extractall("output/storeonly");
  }
  {
    unzip addcomment(
        std::ifstream("resource/extract/addcomment.zip", std::ios::binary));
    addcomment.extractall("output/addcomment");
    EXPECT_EQ("zip comment", addcomment.comment());
  }
  {
    unzip bettercompress(
        std::ifstream("resource/extract/bettercompress.zip", std::ios::binary));
    bettercompress.extractall("output/bettercompress");
  }
  {
    unzip dont_add_dirent(std::ifstream("resource/extract/dont_add_dirent.zip",
                                        std::ios::binary));
    dont_add_dirent.extractall("output/dont_add_dirent");
  }
}

TEST_F(picounzipTest, ExtractByStream) {
  using namespace picounzip;

  unzip file1("resource/extract/def.zip");

  zip_entry deftext = file1.getentry("def.txt");

  unzip_file_stream unzifs(file1, deftext);
  std::string s;
  unzifs >> s;
  EXPECT_EQ("aaaaaa", s);

  unzip_file_stream unzifs2(file1, deftext);

  std::copy(std::istreambuf_iterator<char>(unzifs2),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(std::cout));
}
#endif // PICOUNZIP_USE_CPP11
