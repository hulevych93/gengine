#include <tests/gtest-common/gtest.h>

#include <core/Logger.h>
#include <api/services/ServiceType.h>
#include <appconfig/AppConfig.h>
#include <appconfig/BufferConfigReader.h>
#include <appconfig/SelExtractedBufferedConfigReader.h>
#include <appconfig/SelfExtractedFileConfigReader.h>

using namespace Gengine;
using namespace AppConfig;
using namespace JSON;

TEST_F(GTest, AppConfig)
{
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

    auto testEngine = [config](EntryConfig& test, IConfigReader& reader) {
        EXPECT_TRUE(reader.Load());
        EXPECT_TRUE(config == test);
    };

    {
        BufferConfigReader engine(config);
        engine.Save();

        EntryConfig test;
        BufferConfigReader engine2(engine.GetBuffer(), test);
        testEngine(test, engine2);
    }

    //{
    //    EntryConfig test;
    //    SelfExtractedFileConfigReader engine(test);
    //    testEngine(test, engine);
    //}

    //{
    //    EntryConfig test;
    //    SelExtractedBufferedConfigReader engine(test);
    //    testEngine(test, engine);
    //}
}
