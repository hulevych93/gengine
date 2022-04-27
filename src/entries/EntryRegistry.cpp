#include "EntryRegistry.h"

namespace Gengine {
namespace Entries {

EntryRegistry::EntryRegistry(const std::wstring& key, const TEntryCreator& creator)
    : m_key(key)
    , m_creator(creator)
{}

void EntryRegistry::Do()
{
    Factory.RegisterCreator(m_key, m_creator);
}

void EntryRegistry::Undo()
{
    Factory.RemoveCreator(m_key);
}

std::shared_ptr<IEntry> EntryRegistry::Create(std::unique_ptr<IEntryToolsFactory>&& factory, std::wstring key)
{
    return Factory.Create(key, std::move(factory));
}

EntryRegistry::TEntryFactory EntryRegistry::Factory;

EntryLock::EntryLock()
{
    INITIALIZE_ENTRIES;
}

EntryLock::~EntryLock()
{
    UNINITIALIZE_ENTRIES;
}

}
}