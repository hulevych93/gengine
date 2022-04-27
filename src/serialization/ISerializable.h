#pragma once

#include <serialization/Serializer.h>
#include <serialization/Deserializer.h>

namespace Gengine {
namespace Serialization {

class ISerializable
{
public:
    virtual bool Serialize(Serializer& serializer) const = 0;
    virtual bool Deserialize(const Deserializer& deserializer) = 0;

    std::shared_ptr<Blob> SerializeBlob() const
    {
        Serializer serializer;
        Serialize(serializer);
        return serializer.GetBlob();
    }

    void DeserializeBlob(const std::shared_ptr<Blob>& blob)
    {
        Deserializer deserializer(blob);
        Deserialize(deserializer);
    }
};

}
}