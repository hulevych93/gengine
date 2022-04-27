#pragma once

#include <api/entries/IEntry.h>
#include <core/IAbstractCreator.h>
#include <entries/CoreDefs.h>

namespace Gengine {
namespace Entries {
class SharedEntryCreator : public IAbstractCreator<IEntry, std::unique_ptr<IEntryToolsFactory>&&>
{
public:
    SharedEntryCreator(const std::wstring& moduleName);
    std::shared_ptr<IEntry> Create(std::unique_ptr<IEntryToolsFactory>&& factory) const override;

private:
    std::wstring m_moduleName;
};
}
}