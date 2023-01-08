#include <gtest/gtest.h>
#include <asyncio/buffer/CStringBuffer.h>
#include "test.h"



// Demonstrate some basic assertions.
TEST(CStringBufferTests, CrissCrossSenderReceiverTest)
{
  auto sendBuffer = std::make_unique<CStringBufferSender>("hey");
  auto recvBuffer = std::make_unique<CStringBufferReceiver>();


  std::stringstream ss; 
  auto sender = AsyncTestWriteOperation(false, ss);
  auto receiver = AsyncTestReadOperation(false, ss);


  bool send_called = false;
  sender.request(std::move(sendBuffer), [&](auto buffer_ptr, bool error){
    send_called = true;
    //includes zero character at the end of a c string
    ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 4);
    ASSERT_EQ(error, false);
  });


  bool recv_called = false;
  receiver.request(std::move(recvBuffer), [&](std::unique_ptr<CStringBufferReceiver> buffer_ptr, bool error){
    recv_called = true;
    ASSERT_EQ(buffer_ptr->getString(), "hey");
  });

  while(!sender.check_requests()){}

  ss.seekg(0);
  while(!receiver.check_requests()){}

  ASSERT_EQ(send_called, true);
  ASSERT_EQ(recv_called, true);

}
