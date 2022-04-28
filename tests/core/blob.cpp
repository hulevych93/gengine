#include <tests/gtest-common/gtest.h>

#include <string>

#ifdef BUILD_WINDOWS
#include <tchar.h>
#include <Objbase.h>
#endif

#include <core/Blob.h>

constexpr const char* Data = "data";

using namespace Gengine;

TEST_F(GTest, defaultContructedBlob)
{
    Blob object;
    EXPECT_TRUE(object.GetSize() == 0);
    EXPECT_TRUE(object.GetData() == nullptr);
}

TEST_F(GTest, allocatedBlob)
{
    Blob object(10);
    EXPECT_TRUE(object.GetSize() == 10);
    EXPECT_TRUE(object.GetData() != nullptr);
}

TEST_F(GTest, dataConstructedBlob)
{
    Blob object(Data, strlen(Data));
    EXPECT_TRUE(object.GetSize() == strlen(Data));
    EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), strlen(Data)) == Data);
}

TEST_F(GTest, copyConstructedBlob)
{
    Blob object(Data, strlen(Data));
    Blob object2(object);

    EXPECT_TRUE(object == object2);
}

TEST_F(GTest, assignedBlob)
{
    Blob object(Data, strlen(Data));
    Blob object2;

    object2 = object;
    EXPECT_TRUE(object == object2);
}

TEST_F(GTest, moveConstructedBlob)
{
    Blob object(Data, strlen(Data));
    Blob object2(std::move(object));

    EXPECT_TRUE((object.GetSize() == 0 && object.GetData() == nullptr));
    EXPECT_TRUE(std::string(reinterpret_cast<char*>(object2.GetData()), strlen(Data)) == Data);
}

TEST_F(GTest, moveAssignedBlob)
{
    Blob object(Data, strlen(Data));
    Blob object2;

    object2 = std::move(object);
    EXPECT_TRUE((object.GetSize() == 0 && object.GetData() == nullptr));
    EXPECT_TRUE(std::string(reinterpret_cast<char*>(object2.GetData()), strlen(Data)) == Data);
}

TEST_F(GTest, resizeBlob)
{
    Blob object;
    object.Resize(100);
    EXPECT_TRUE(object.GetSize() == 100);
    EXPECT_TRUE(object.GetData() != nullptr);
}

TEST_F(GTest, resizeWithPreserveBlob)
{
    Blob object;
    object.SetData(Data, strlen(Data));

    EXPECT_TRUE(object.GetSize() == strlen(Data));
    EXPECT_TRUE(object.GetData() != nullptr);

    object.Resize(strlen(Data) * 2, true);
    EXPECT_TRUE(object.GetSize() == strlen(Data) * 2);
    EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), strlen(Data)) == Data);
}

TEST_F(GTest, resizeWithNoPreserveBlob)
{
    Blob object;
    object.SetData(Data, strlen(Data));

    EXPECT_TRUE(object.GetSize() == strlen(Data));
    EXPECT_TRUE(object.GetData() != nullptr);

    object.Resize(strlen(Data) * 2, false);
    EXPECT_TRUE(object.GetSize() == strlen(Data) * 2);
    EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), strlen(Data) * 2) != Data);
}

TEST_F(GTest, clearDataBlob)
{
    Blob object;
    object.SetData(Data, strlen(Data));

    EXPECT_TRUE(object.GetSize() == strlen(Data));
    EXPECT_TRUE(object.GetData() != nullptr);

    object.Clear();
    EXPECT_TRUE(object.GetSize() == 0);
    EXPECT_TRUE(object.GetData() == nullptr);
}

TEST_F(GTest, addDataBlob)
{
    const auto data2 = std::string("bbbbb");

    Blob object;
    object.SetData(Data, strlen(Data));

    EXPECT_TRUE(object.GetSize() == strlen(Data));
    EXPECT_TRUE(object.GetData() != nullptr);

    object.AddData(data2.data(), data2.size());
    EXPECT_TRUE(object.GetSize() == strlen(Data) + data2.size());
    EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()),
                            strlen(Data) + data2.size()) == std::string{Data} + data2);
}
