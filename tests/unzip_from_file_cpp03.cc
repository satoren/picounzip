
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
  EXPECT_EQ(0x3a67f42aU, makefile.CRC);
  EXPECT_EQ(486U, makefile.file_size);
  EXPECT_EQ(190U, makefile.compress_size);
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
    dont_add_dirent.extractall("output/def");
    dont_add_dirent.extractall();

	//check extract file
	std::ifstream fdef("output/def/def.txt", std::ios::binary);
	std::string s;
	fdef >> s;
	EXPECT_EQ("aaaaaa", s);
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


TEST_F(picounzipTest, ExtractDataCheck) {
	using namespace picounzip;
	{
		unzip zip("resource/extract/random.zip");
		zip.extractall("output/random");

		std::ifstream original("resource/extract/random.dat", std::ios::binary);
		std::ifstream extracted("output/random/random.dat", std::ios::binary);

		bool file_equal = std::equal(std::istreambuf_iterator<char>(extracted),
			std::istreambuf_iterator<char>(), std::istreambuf_iterator<char>(original));
		EXPECT_TRUE(file_equal);
	}

	{
		std::ifstream original("resource/extract/random.dat", std::ios::binary);
		unzip zip("resource/extract/random.zip");

		unzip_file_stream unzifs(zip, zip.entrylist().front());

		bool data_equal = std::equal(std::istreambuf_iterator<char>(unzifs),
			std::istreambuf_iterator<char>(), std::istreambuf_iterator<char>(original));
		EXPECT_TRUE(data_equal);
	}
}
TEST_F(picounzipTest, ExtractError) {

  using namespace picounzip;
  { EXPECT_THROW(unzip("resource/extract/random.dat"), unzip_error); }
  { EXPECT_THROW(unzip("resource/extract/def.txt"), unzip_error); }
  { EXPECT_THROW(unzip("resource/extract/def.txt"), unzip_error); }
  {
    unzip crc_error("resource/extract/crc_error.zip");
    EXPECT_THROW(crc_error.extractall("output/crc_error/"), unzip_error);
  }
  {
    unzip crc_error("resource/extract/crc_error.zip");
    error_info error;
    crc_error.extractall("output/crc_error/", error);
    EXPECT_TRUE(error);
    EXPECT_EQ(error.error_code, error.UNZIP_BAD_ZIP_FILE);
  }
  {
    unzip dont_add_dirent("resource/extract/def.zip");
    EXPECT_THROW(dont_add_dirent.extract("notfound"), unzip_error);
  }
  {
    unzip dont_add_dirent("resource/extract/def.zip");
    error_info error;
    EXPECT_FALSE(error);
    dont_add_dirent.extract("notfound", error);
    EXPECT_TRUE(error);
  }
}
