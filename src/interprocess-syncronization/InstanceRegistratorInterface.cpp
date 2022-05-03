#include <interprocess-syncronization/InstanceRegistratorInterface.h>

namespace Gengine {
namespace InterprocessSynchronization {
InstanceRegistratorInterface::InstanceRegistratorInterface(
    std::wstring&& objectName)
    : m_objectName(std::move(objectName)) {}

const std::wstring& InstanceRegistratorInterface::GetObjectName() const {
  return m_objectName;
}
}  // namespace InterprocessSynchronization
}  // namespace Gengine