#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions)
{
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

#include <asyncio/buffer/TypeLengthBuffer.h>
#include <asyncio/async_operation.h>

struct TestWriteOperation
{
  std::ostream &stream;
  TestWriteOperation(std::ostream &stream)
      : stream(stream) {}

  int operator()(char *d, int n)
  {
    for (size_t i = 0; i < n; i++)
    {
      stream.put(d[i]);
    }
    return n;
  }
};

struct TestReadOperation
{
  std::istream &stream;
  TestReadOperation(std::istream &stream)
      : stream(stream) {}

  int operator()(char *d, int n)
  {
    stream.get(d,n);
    return n;
  }
};


using AsyncTestWriteOperation = AsyncOperation<TestWriteOperation>;
using AsyncTestReadOperation = AsyncOperation<TestReadOperation>;

// Demonstrate some basic assertions.
TEST(TypeLengthBufferTests, CrissCrossSenderReceiverTest)
{
  auto sendBuffer = std::make_unique<TypeLengthBufferSender>(5, std::vector<char>({'h', 'e', 'y'}));
  std::stringstream ss(std::ios_base::binary); 
  auto sender = AsyncTestWriteOperation(false, ss);
  auto receiver = AsyncTestReadOperation(false, ss);


  bool send_called = false;
  sender.request(std::move(sendBuffer), [&](Buffer_Ptr buffer_ptr, bool error){
    send_called = true;
    auto& buffer = *(TypeLengthBufferSender*)(buffer_ptr.get());
    ASSERT_EQ(buffer.payload_sent_so_far() , 3);
    ASSERT_EQ(error, false);
  });


  while(!sender.check_requests()){}

  ASSERT_EQ(send_called, true);

}
