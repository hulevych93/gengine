#include <gtest/gtest.h>

#include <entries/TestEntry.h>
#include <gengine/gengine.h>

class SerializationTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

#include <core/Logger.h>
#include <serialization/ISerializable.h>

using namespace Gengine;
using namespace Entries;
using namespace Serialization;

TEST_F(SerializationTest, NumbersSerialization) {
  Serializer serializer;

  uint8_t inBytes8 = 8;
  uint16_t inBytes16 = 16;
  uint32_t inBytes32 = 32;
  uint64_t inBytes64 = 64;

  int8_t iniBytes8 = 8;
  int16_t iniBytes16 = 16;
  int32_t iniBytes32 = 32;
  int64_t iniBytes64 = 64;

  serializer << inBytes8;
  serializer << inBytes16;
  serializer << inBytes32;
  serializer << inBytes64;

  serializer << iniBytes8;
  serializer << iniBytes16;
  serializer << iniBytes32;
  serializer << iniBytes64;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    uint8_t outBytes8 = 0;
    deserializer >> outBytes8;
    EXPECT_TRUE(inBytes8 == outBytes8);
  }

  {
    uint16_t outBytes16 = 0;
    deserializer >> outBytes16;
    EXPECT_TRUE(inBytes16 == outBytes16);
  }

  {
    uint32_t outBytes32 = 0;
    deserializer >> outBytes32;
    EXPECT_TRUE(inBytes32 == outBytes32);
  }

  {
    uint64_t outBytes64 = 0;
    deserializer >> outBytes64;
    EXPECT_TRUE(inBytes64 == outBytes64);
  }

  {
    int8_t outiBytes8 = 0;
    deserializer >> outiBytes8;
    EXPECT_TRUE(iniBytes8 == outiBytes8);
  }

  {
    int16_t outiBytes16 = 0;
    deserializer >> outiBytes16;
    EXPECT_TRUE(iniBytes16 == outiBytes16);
  }

  {
    int32_t outiBytes32 = 0;
    deserializer >> outiBytes32;
    EXPECT_TRUE(iniBytes32 == outiBytes32);
  }

  {
    int64_t outiBytes64 = 0;
    deserializer >> outiBytes64;
    EXPECT_TRUE(iniBytes64 == outiBytes64);
  }
}

TEST_F(SerializationTest, BooleanSerialization) {
  Serializer serializer;

  bool boolean = true;
  serializer << boolean;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    bool outBoolean = false;
    deserializer >> outBoolean;
    EXPECT_TRUE(boolean == outBoolean);
  }
}

TEST_F(SerializationTest, EnumSerialization) {
  Serializer serializer;

  enum class Types { First, Second, Third } enumType = Types::First;

  serializer << enumType;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(enumType) outEnumType = Types::Third;
    deserializer >> outEnumType;
    EXPECT_TRUE(enumType == outEnumType);
  }
}

TEST_F(SerializationTest, StringSerialization) {
  Serializer serializer;

  std::wstring inString = L"Some readable data";
  serializer << inString;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inString) outString;
    deserializer >> outString;
    EXPECT_TRUE(inString == outString);
  }
}

TEST_F(SerializationTest, VectorsSerialization) {
  Serializer serializer;

  std::vector<int16_t> inVector = {3, 5, 15, 355, 654};
  serializer << inVector;

  std::list<int16_t> inList = {3, 5, 15, 355, 654};
  serializer << inList;

  std::deque<int16_t> inDeque = {3, 5, 15, 355, 654};
  serializer << inDeque;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inVector) outVector;
    deserializer >> outVector;
    EXPECT_TRUE(inVector == outVector);
  }

  {
    decltype(inList) outList;
    deserializer >> outList;
    EXPECT_TRUE(inList == outList);
  }

  {
    decltype(inDeque) outDeque;
    deserializer >> outDeque;
    EXPECT_TRUE(inDeque == outDeque);
  }
}

TEST_F(SerializationTest, SetsSerialization) {
  Serializer serializer;

  std::set<int16_t> inSet = {3, 15, 35, 544, 92};
  serializer << inSet;

  std::unordered_set<int16_t> inUSet = {3, 15, 35, 544, 92};
  serializer << inUSet;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inSet) outSet;
    deserializer >> outSet;
    EXPECT_TRUE(inSet == outSet);
  }

  {
    decltype(inUSet) outSet;
    deserializer >> outSet;
    EXPECT_TRUE(inUSet == outSet);
  }
}

TEST_F(SerializationTest, MapsSerialization) {
  Serializer serializer;

  std::map<int16_t, int8_t> inMap = {{3, 5}, {15, 35}, {544, 92}};
  serializer << inMap;

  std::unordered_map<int16_t, int8_t> inUMap = {{3, 5}, {15, 35}, {544, 92}};
  serializer << inUMap;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inMap) outMap;
    deserializer >> outMap;
    EXPECT_TRUE(inMap == outMap);
  }

  {
    decltype(inUMap) outMap;
    deserializer >> outMap;
    EXPECT_TRUE(inUMap == outMap);
  }
}

TEST_F(SerializationTest, PointersSerialization) {
  Serializer serializer;

  std::shared_ptr<int> inSPointer = std::make_shared<int>(111);
  serializer << inSPointer;

  std::unique_ptr<int> inUPointer = std::make_unique<int>(111);
  serializer << inUPointer;

  boost::optional<int> inOptional(111);
  serializer << inOptional;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inSPointer) outPointer;
    deserializer >> outPointer;
    EXPECT_TRUE(*inSPointer == *outPointer);
  }

  {
    decltype(inUPointer) outPointer;
    deserializer >> outPointer;
    EXPECT_TRUE(*inUPointer == *outPointer);
  }

  {
    decltype(inOptional) outOptional;
    deserializer >> outOptional;
    EXPECT_TRUE(inOptional == outOptional);
  }
}

TEST_F(SerializationTest, PairsSerialization) {
  Serializer serializer;

  std::pair<int, int> inPair = {11, 2};
  serializer << inPair;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inPair) outPair;
    deserializer >> outPair;
    EXPECT_TRUE(inPair == outPair);
  }
}

TEST_F(SerializationTest, VariantSerialization) {
  Serializer serializer;

  boost::variant<std::int32_t, std::string> inVariant = "string variant value";
  serializer << inVariant;

  auto blob = serializer.GetBlob();
  Deserializer deserializer(*blob);

  {
    decltype(inVariant) outVariant;
    deserializer >> outVariant;
    EXPECT_TRUE(inVariant == outVariant);
  }
}

REGISTER_TESTS_ENTRY(GTestModule)
IMPLEMENT_CONSOLE_ENTRY
