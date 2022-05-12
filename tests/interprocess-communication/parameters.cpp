#include <entries/TestEntry.h>
#include <gengine/gengine.h>
#include <gtest/gtest.h>

#include <interprocess-communication/param/InputParameters.h>
#include <interprocess-communication/param/OutputParameters.h>

using namespace Gengine;
using namespace Entries;
using namespace InterprocessCommunication;

namespace {
class InputOutputParamTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

struct Serializable final : Serialization::ISerializable {
  Serializable() = default;
  Serializable(bool first, int second, std::string hello)
      : first(first), second(second), hello(hello) {}

  bool Serialize(Serialization::Serializer& serializer) const override {
    serializer << first;
    serializer << second;
    serializer << hello;
    return true;
  }
  bool Deserialize(const Serialization::Deserializer& deserializer) override {
    deserializer >> first;
    deserializer >> second;
    deserializer >> hello;
    return true;
  }

  bool operator==(const Serializable& that) const {
    return std::tie(first, second, hello) ==
           std::tie(that.first, that.second, that.hello);
  }

  bool first = false;
  int second = 24;
  std::string hello = "hi";
};

struct JsonSerializable final : JSON::IJsonSerializable {
  JsonSerializable() = default;
  JsonSerializable(bool first, int second, std::string hello)
      : first(first), second(second), hello(hello) {}

  bool Serialize(JSON::Object& object) const override {
    object["first"] << first;
    object["second"] << second;
    object["hello"] << hello;
    return true;
  }
  bool Deserialize(const JSON::Object& object) override {
    object["first"] >> first;
    object["second"] >> second;
    object["hello"] >> hello;
    return true;
  }

  bool operator==(const JsonSerializable& that) const {
    return std::tie(first, second, hello) ==
           std::tie(that.first, that.second, that.hello);
  }

  bool first = false;
  int second = 24;
  std::string hello = "hi";
};

template <typename ParamType>
void testInputOutput(const ParamType param) {
  OutputParameters outParams;
  outParams.Append(param);

  auto inParams = InputParameters::makeParameters((void*)outParams.GetData(),
                                                  outParams.GetSize());

  ParamType out{};
  EXPECT_TRUE(inParams.Get(0, out));
  EXPECT_EQ(param, out);
}

}  // namespace

TEST_F(InputOutputParamTest, simple) {
  testInputOutput(true);
  testInputOutput((void*)nullptr);
  testInputOutput(std::int8_t{23});
  testInputOutput(std::int16_t{-245});
  testInputOutput(std::int32_t{2344});
  testInputOutput(std::int64_t{22222222222});
  testInputOutput(std::uint8_t{23});
  testInputOutput(std::uint16_t{245});
  testInputOutput(std::uint32_t{2344});
  testInputOutput(std::uint64_t{22222222222});
  testInputOutput(std::string{"true hello"});
  testInputOutput(std::wstring{L"hells"});
}

TEST_F(InputOutputParamTest, blob) {
  Blob blob;
  blob.AddData("abcde", 5);
  testInputOutput(blob);
}

TEST_F(InputOutputParamTest, serializable) {
  Serializable data{false, 245, std::string{"waka waka"}};
  testInputOutput(data);
}

TEST_F(InputOutputParamTest, jsonSerializable) {
  JsonSerializable data{false, 245, std::string{"waka waka"}};
  testInputOutput(data);
}

TEST_F(InputOutputParamTest, ptrsNulls) {
  OutputParameters params;

  EXPECT_THROW(params.Append(std::unique_ptr<int>{}), std::runtime_error);
  EXPECT_THROW(params.Append(std::shared_ptr<int>{}), std::runtime_error);
}

TEST_F(InputOutputParamTest, sharedPtr) {
  OutputParameters outParams;
  outParams.Append(std::make_shared<int>(22));

  InputParameters inParams = InputParameters::makeParameters(
      (void*)outParams.GetData(), outParams.GetSize());

  std::shared_ptr<int> out;
  EXPECT_TRUE(inParams.Get(0, out));
  EXPECT_NE(nullptr, out);
  EXPECT_EQ(22, *out);
}

TEST_F(InputOutputParamTest, uniquePtr) {
  OutputParameters outParams;
  outParams.Append(std::make_unique<bool>(false));

  InputParameters inParams = InputParameters::makeParameters(
      (void*)outParams.GetData(), outParams.GetSize());

  std::unique_ptr<bool> out;
  EXPECT_TRUE(inParams.Get(0, out));
  EXPECT_NE(nullptr, out);
  EXPECT_EQ(false, *out);
}

TEST_F(InputOutputParamTest, vector) {
  testInputOutput(std::vector<int>{24, 55, 921});
}

TEST_F(InputOutputParamTest, list) {
  testInputOutput(std::list<std::int8_t>{'b', '2', 'c'});
}

TEST_F(InputOutputParamTest, deque) {
  testInputOutput(std::deque<int>{1, 543, -8});
}

TEST_F(InputOutputParamTest, set) {
  testInputOutput(std::set<int>{1, 543, -8});
}

TEST_F(InputOutputParamTest, unorderedSet) {
  testInputOutput(std::unordered_set<int>{1, 543, -8});
}

TEST_F(InputOutputParamTest, map) {
  testInputOutput(std::map<int, int>{{1, 2}, {543, -543}, {8, -8}});
}

TEST_F(InputOutputParamTest, unorderedMap) {
  testInputOutput(std::unordered_map<int, int>{{1, 2}, {543, -543}, {8, -8}});
}

REGISTER_TESTS_ENTRY(GTestModule)
IMPLEMENT_CONSOLE_ENTRY
