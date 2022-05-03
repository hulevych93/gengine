#pragma once

#include <core/AbstractFactory.h>
#include <core/StaticList.h>

#include <entries/IEntryToolsFactory.h>
#include <entries/SharedEntryCreator.h>

#include <api/entries/IEntry.h>

namespace Gengine {
namespace Entries {
class EntryRegistry {
 public:
  using TEntryCreator = std::shared_ptr<
      IAbstractCreator<IEntry, std::unique_ptr<IEntryToolsFactory>&&>>;
  using TEntryFactory = AbstractFactory<IEntry,
                                        std::wstring,
                                        std::unique_ptr<IEntryToolsFactory>&&>;

 public:
  EntryRegistry(const std::wstring& key, const TEntryCreator& creator);

  void Do();
  void Undo();

  static std::shared_ptr<IEntry> Create(
      std::unique_ptr<IEntryToolsFactory>&& factory,
      std::wstring key);

 private:
  std::wstring m_key;
  TEntryCreator m_creator;
  static TEntryFactory Factory;
};

#define REGISTER_ENTRY(key, name)                                            \
  static FactoryItem<EntryRegistry> const entryRegistry##name(EntryRegistry( \
      key, std::make_shared<ConcreteCreator<                                 \
               IEntry, name, std::unique_ptr<IEntryToolsFactory>&&>>()));
#define REGISTER_MAIN_ENTRY(name) REGISTER_ENTRY(L"default", name)
#define REGISTER_TESTS_ENTRY(name) REGISTER_ENTRY(L"", name)

#define REGISTER_SHARED_ENTRY(key, name, moduleName)           \
  static FactoryItem<EntryRegistry> const entryRegistry##name( \
      EntryRegistry(key, std::make_shared<SharedEntryCreator>(moduleName)));

#define INITIALIZE_ENTRIES        \
  do {                            \
    Runtime<EntryRegistry>::Do(); \
  } while (false)
#define UNINITIALIZE_ENTRIES        \
  do {                              \
    Runtime<EntryRegistry>::Undo(); \
  } while (false)

struct EntryLock {
  EntryLock();
  ~EntryLock();
};
}  // namespace Entries
}  // namespace Gengine
