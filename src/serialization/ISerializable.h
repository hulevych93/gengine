#pragma once

#include <serialization/Deserializer.h>
#include <serialization/Serializer.h>

namespace Gengine {
namespace Serialization {

/**
 * @brief The ISerializable abstract class
 *
 * The ISerializable is the interface for all binary serializable types
 * that can be transfered over the network, for example.
 */
class ISerializable {
 public:
  /**
   * @brief Serialization method
   * @param serializer object accepts class fields serializable to binary data.
   * @return true on success.
   */
  virtual bool Serialize(Serializer& serializer) const = 0;

  /**
   * @brief Deserialization method
   * @param deserializer object contains binary data to be deserialized
   * @return true on success.
   */
  virtual bool Deserialize(const Deserializer& deserializer) = 0;

  /**
   * @brief SerializeBlob method
   * @return binary serialized blob containing data of the object.
   */
  std::shared_ptr<Blob> SerializeBlob() const {
    Serializer serializer;
    Serialize(serializer);
    return serializer.GetBlob();
  }

  /**
   * @brief DeserializeBlob method
   * @param blob serialized blob containing data of the object to be
   * deserialized.
   */
  void DeserializeBlob(const std::shared_ptr<Blob>& blob) {
    Deserializer deserializer(*blob);
    Deserialize(deserializer);
  }
};

}  // namespace Serialization
}  // namespace Gengine
