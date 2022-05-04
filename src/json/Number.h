#pragma once

#include <json/Common.h>

namespace Gengine {
namespace JSON {

class Number final {
 private:
  enum class number_type : std::uint8_t {
    signed_type,
    unsigned_type,
    real_type
  };

 public:
  Number(real_t value);
  Number(uint64_t value);
  Number(int64_t value);
  Number(uint32_t value);
  Number(int32_t value);
  Number(uint16_t value);
  Number(int16_t value);
  Number(uint8_t value);
  Number(int8_t value);
  ~Number();

  bool IsReal() const;
  bool IsUint64() const;

  real_t ToReal() const;
  uint64_t ToUint64() const;
  int64_t ToInt64() const;
  uint32_t ToUint32() const;
  int32_t ToInt32() const;
  bool operator==(const Number& that) const;
  void Serialize(stream_t& stream) const;

 private:
  union {
    real_t m_realValue;
    int64_t m_intValue;
    uint64_t m_uintValue;
  };

  number_type m_type;
};

}  // namespace JSON
}  // namespace Gengine
