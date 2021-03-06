#pragma once

#include <map>
#include <memory>

namespace Gengine {
namespace AbstractFactory {

template <class Interface, class... Args>
class IAbstractCreator {
 public:
  virtual ~IAbstractCreator() = default;
  virtual std::shared_ptr<Interface> Create(Args&&... args) const = 0;
};

template <class Interface, class ObjectType, class... Args>
class AbstractFactory final {
 private:
  using Creators =
      std::map<ObjectType,
               std::shared_ptr<IAbstractCreator<Interface, Args...>>>;

 public:
  AbstractFactory() = default;
  AbstractFactory(
      const std::initializer_list<typename Creators::value_type>& initList) {
    for (const auto& listValue : initList) {
      m_creators.insert(listValue);
    }
  }

  void RegisterCreator(
      ObjectType type,
      const std::shared_ptr<IAbstractCreator<Interface, Args...>>& creator) {
    m_creators[type] = creator;
  }

  void RemoveCreator(ObjectType type) { m_creators.erase(type); }

  std::shared_ptr<Interface> Create(ObjectType type, Args&&... args) const {
    auto creator = FindCreator(type);
    if (creator) {
      return creator->Create(std::forward<Args>(args)...);
    }

    return std::shared_ptr<Interface>();
  }

 private:
  std::shared_ptr<IAbstractCreator<Interface, Args...>> FindCreator(
      ObjectType type) const {
    auto creatorIter = m_creators.find(type);
    if (creatorIter != m_creators.end()) {
      return creatorIter->second;
    }

    return std::shared_ptr<IAbstractCreator<Interface, Args...>>();
  }

 private:
  Creators m_creators;
};

template <class Interface, class Implementation, class... Args>
class ConcreteCreator final : public IAbstractCreator<Interface, Args...> {
 public:
  std::shared_ptr<Interface> Create(Args&&... args) const override {
    return std::make_shared<Implementation>(std::forward<Args>(args)...);
  }
};

template <class Interface,
          class Implementation,
          class CustingType,
          class... Args>
class ConcreteCustingCreator final
    : public IAbstractCreator<Interface, Args...> {
 public:
  std::shared_ptr<Interface> Create(Args&&... args) const override {
    auto custedArg = std::dynamic_pointer_cast<CustingType>(args...);
    if (custedArg) {
      return std::make_shared<Implementation>(custedArg);
    }

    return std::shared_ptr<Interface>();
  }
};

}  // namespace AbstractFactory
}  // namespace Gengine
