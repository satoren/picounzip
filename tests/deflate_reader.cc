

#include <fstream>
#include <gtest/gtest.h>

#include "picounzip.hpp"

class deflate_readerTest : public ::testing::Test {
protected:
  virtual void SetUp() {}

  // virtual void TearDown() {}
};

TEST_F(deflate_readerTest, read_defrate_stream) {
  using namespace picounzip;
  // one pass
  {
    std::ifstream ifs("resource/deflate/text.txt.z", std::ios::binary);

    deflate_reader reader(ifs);

    char buffer[512] = {};
    deflate_reader::read_result ret = reader.read(buffer, 512);

    EXPECT_STREQ(0, ret.error);
    EXPECT_EQ(size_t(52), ret.size);
    EXPECT_STREQ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
                 buffer);
  }
  {
    std::ifstream ifs("resource/deflate/text.txt.z", std::ios::binary);

    deflate_reader reader(ifs);

    char buffer[512] = {};
    int readpos = 0;
    {
      deflate_reader::read_result ret;
      ret = reader.read(buffer, 30);
      readpos += ret.size;
      while (ret.size != 0 && !ret.error) {
        ret = reader.read(&buffer[readpos], 30);
        readpos += ret.size;
      }
      EXPECT_EQ(0, ret.error);
      EXPECT_EQ(52, readpos);
      EXPECT_STREQ(buffer,
                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
  }
  {
    std::ifstream ifs("resource/deflate/text.txt.z", std::ios::binary);

    deflate_reader reader(ifs);

    char buffer[512] = {};
    int readpos = 0;
    {
      deflate_reader::read_result ret;
      ret = reader.read(buffer, 1);
      readpos += ret.size;
      while (ret.size != 0 && !ret.error) {
        ret = reader.read(&buffer[readpos], 1);
        readpos += ret.size;
      }
      EXPECT_EQ(0, ret.error);
      EXPECT_EQ(52, readpos);
      EXPECT_STREQ(buffer,
                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
  }
}
