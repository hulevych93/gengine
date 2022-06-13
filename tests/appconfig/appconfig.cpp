#include <entries/TestEntry.h>
#include <gtest/gtest.h>

#include <gengine/gengine.h>

#include <api/services/ServiceType.h>

#include <appconfig/AppConfig.h>
#include <appconfig/BufferConfigReader.h>
#include <appconfig/SelfExtractedBufferedConfigReader.h>
#include <appconfig/SelfExtractedFileConfigReader.h>

using namespace Gengine;
using namespace Entries;
using namespace AppConfig;
using namespace JSON;

class AppconfigTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

EntryConfig makeEntryConfig() {
  EntryConfig config;
  config.name = "test1";
  config.singleObjectName = "test2";
  config.aliveObjectName = "test3";

  {
    ServiceConfig in;
    in.key = "ITest1";
    in.type = ServiceType::Remote;
    PipeConnection connection;
    connection.pipe = L"abcdef";
    in.connection = connection;
    in.processRef.emplace(123);
    config.inServices.emplace(std::make_pair("1", in));
  }

  {
    ServiceConfig in2;
    in2.key = "ITest4";
    in2.type = ServiceType::Shared;
    SharedConnection connection;
    connection.path = "wreqr.dll";
    in2.connection = connection;
    in2.serviceRefs.emplace("23");
    config.inServices.emplace(std::make_pair("3", in2));
  }

  {
    ServiceConfig out;
    out.key = "ITest2";
    out.type = ServiceType::Remote;
    TcpConnection connection;
    connection.ip = "127.0.0.1";
    connection.port = 1234;
    out.connection = connection;
    out.threadRef.emplace(123);
    out.required = false;
    config.outServices.emplace(std::make_pair("2", out));
  }

  return config;
}

TEST_F(AppconfigTest, bufferConfigReader) {
  auto config = makeEntryConfig();

  auto engine = makeBinaryConfigReader<BufferConfigReader>(config);
  engine.Save();

  EntryConfig test;
  auto engine2 = makeBinaryConfigReader<BufferConfigReader>(test, engine.GetBuffer());
  EXPECT_TRUE(engine2.Load());
  EXPECT_TRUE(config == test);
}

REGISTER_TESTS_ENTRY(GTestModule)
IMPLEMENT_CONSOLE_ENTRY
