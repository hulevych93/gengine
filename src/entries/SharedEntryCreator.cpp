#include "SharedEntryCreator.h"

#include <boost/dll/import.hpp>
#include <boost/function.hpp>
#include <filesystem/Filesystem.h>
#include <core/Encoding.h>

namespace Gengine {
namespace Entries {

SharedEntryCreator::SharedEntryCreator(const std::wstring& moduleName)
    : m_moduleName(moduleName)
{}

std::shared_ptr<IEntry> SharedEntryCreator::Create(std::unique_ptr<IEntryToolsFactory>&& factory) const
{
    auto creator = boost::dll::import_alias<boost::shared_ptr<IEntry>(std::unique_ptr<IEntryToolsFactory>&& factory)>(
        Filesystem::CombinePath(Filesystem::GetAppFolder(), m_moduleName),
        "create_shared_entry",
        boost::dll::load_mode::append_decorations
        );

    auto lib = boost::make_shared<boost::dll::shared_library>(Filesystem::CombinePath(Filesystem::GetAppFolder(), m_moduleName), boost::dll::load_mode::append_decorations);
    auto ptr = creator(std::move(factory));
    return std::shared_ptr<IEntry>(ptr.get(), [lib, ptr](...) { });
}
}
}
