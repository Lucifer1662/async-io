#pragma once
#include <asyncio/async_operation.h>

struct TestWriteOperation
{
  std::iostream &stream;
  TestWriteOperation(std::iostream &stream)
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
    auto end = d + n;
    for (; d != end; d++)
    {
      *d = stream.get();
    }
    
    return n;
  }
};


using AsyncTestWriteOperation = AsyncOperation<TestWriteOperation>;
using AsyncTestReadOperation = AsyncOperation<TestReadOperation>;
