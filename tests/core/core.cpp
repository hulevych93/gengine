#include <tests/gtest-common/gtest.h>

#include <string>

#ifdef BUILD_WINDOWS
#include <tchar.h>
#include <Objbase.h>
#endif

#include <core/Blob.h>
#include <filesystem/Filesystem.h>
#include <core/Logger.h>

using namespace Gengine;

TEST_F(GTest, BlobTest)
{
    auto data = std::string("abc");
    auto data2 = std::string("bbbbb");

    {
        //SECTION("Blob size is 0 when default constructed") 
        {
            Blob object;
            EXPECT_TRUE(object.GetSize() == 0);
            EXPECT_TRUE(object.GetData() == nullptr);
        }

        //SECTION("Blob size is not 0 when size constructed") 
        {
            Blob object(10);
            EXPECT_TRUE(object.GetSize() == 10);
            EXPECT_TRUE(object.GetData() != nullptr);
        }

        //SECTION("Blob is correctly constructed with data") 
        {
            Blob object(data.data(), data.size());
            EXPECT_TRUE(object.GetSize() == data.size());
            EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), data.size()) == data);
        }

        //SECTION("Blob data is correctly coppied") 
        {
            Blob object(data.data(), data.size());
            Blob object2(object);

            EXPECT_TRUE(object == object2);
        }

        //SECTION("Blob data is correctly assigned") 
        {
            Blob object(data.data(), data.size());
            Blob object2;

            object2 = object;
            EXPECT_TRUE(object == object2);
        }

        //SECTION("Blob data is correctly move costructed") 
        {
            Blob object(data.data(), data.size());
            Blob object2(std::move(object));

            EXPECT_TRUE((object.GetSize() == 0 && object.GetData() == nullptr));
            EXPECT_TRUE(std::string(reinterpret_cast<char*>(object2.GetData()), data.size()) == data);
        }

        //SECTION("Blob data is correctly move assigned") 
        {
            Blob object(data.data(), data.size());
            Blob object2;

            object2 = std::move(object);
            EXPECT_TRUE((object.GetSize() == 0 && object.GetData() == nullptr));
            EXPECT_TRUE(std::string(reinterpret_cast<char*>(object2.GetData()), data.size()) == data);
        }
    }

    //SECTION("Blob size is not null when resized") 
    {
        Blob object;
        object.Resize(100);
        EXPECT_TRUE(object.GetSize() == 100);
        EXPECT_TRUE(object.GetData() != nullptr);
    }

    {
        //SECTION("Blob data is saved, if resize with preserve") 
        {
            Blob object;
            object.SetData(data.data(), data.size());

            EXPECT_TRUE(object.GetSize() == data.size());
            EXPECT_TRUE(object.GetData() != nullptr);

            object.Resize(data.size() * 2, true);
            EXPECT_TRUE(object.GetSize() == data.size() * 2);
            EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), data.size()) == data);
        }

        //SECTION("Blob data is now saved, if resize with no preserve") 
        {
            Blob object;
            object.SetData(data.data(), data.size());

            EXPECT_TRUE(object.GetSize() == data.size());
            EXPECT_TRUE(object.GetData() != nullptr);

            object.Resize(data.size() * 2, false);
            EXPECT_TRUE(object.GetSize() == data.size() * 2);
            EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), data.size() * 2) != data);
        }

        //SECTION("Blob data is cleared fine") 
        {
            Blob object;
            object.SetData(data.data(), data.size());

            EXPECT_TRUE(object.GetSize() == data.size());
            EXPECT_TRUE(object.GetData() != nullptr);

            object.Clear();
            EXPECT_TRUE(object.GetSize() == 0);
            EXPECT_TRUE(object.GetData() == nullptr);
        }

        //SECTION("Blob data is correctly added") 
        {
            Blob object;
            object.SetData(data.data(), data.size());

            EXPECT_TRUE(object.GetSize() == data.size());
            EXPECT_TRUE(object.GetData() != nullptr);

            object.AddData(data2.data(), data2.size());
            EXPECT_TRUE(object.GetSize() == data.size() + data2.size());
            EXPECT_TRUE(std::string(reinterpret_cast<char*>(object.GetData()), data.size() + data2.size()) == data + data2);
        }
    }
}
