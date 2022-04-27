#pragma once

#include <boost/dll/import.hpp>
#include <boost/function.hpp>
#include <core/IAbstractCreator.h>
#include <filesystem/Filesystem.h>
#include <core/Encoding.h>

#include <shared-services/ISharedLibraryClient.h>

namespace Gengine {
namespace Services {
class IMicroService;
}
namespace SharedServices {

template<class ServiceInterface>
std::shared_ptr<ServiceInterface> to_std(const boost::shared_ptr<ServiceInterface>& p)
{
    return std::shared_ptr<ServiceInterface>(p.get(), [p](...) {});
}

template<class ServiceInterface>
std::shared_ptr<ServiceInterface> import_symbol(const SharedConnection& data)
{
    auto libPath = Filesystem::CombinePath(toUtf8(Filesystem::GetAppFolder()), data.path);
    return to_std(boost::dll::import<ServiceInterface>(
        libPath,
        data.symbol,
        boost::dll::load_mode::append_decorations
        ));
}

template<class ServiceInterface>
std::shared_ptr<ServiceInterface> import_creation_method(const SharedConnection& data)
{
    auto libPath = Filesystem::CombinePath(toUtf8(Filesystem::GetAppFolder()), data.path);
    auto creator = boost::dll::import_alias<ServiceInterface*()>(
        libPath,
        data.symbol,
        boost::dll::load_mode::append_decorations
        );

    auto plugin = creator();
    auto lib = boost::make_shared<boost::dll::shared_library>(libPath, boost::dll::load_mode::append_decorations);
    return std::shared_ptr<ServiceInterface>(plugin, [lib](ServiceInterface* p) { delete p; });
}

template<class ServiceInterface>
class SymbolClient : public Services::IMicroService,
    public ISharedLibraryClient
{
public:
    std::shared_ptr<Services::IMicroService> Connect(const SharedConnection& data) override
    {
        if (!m_interface)
        {
            m_interface = import_symbol<ServiceInterface>(data);
        }
        return m_interface;
    }

private:
    std::shared_ptr<ServiceInterface> m_interface;
};

template<class ServiceInterface>
class CreationMethodClient : public Services::IMicroService,
    public ISharedLibraryClient
{
public:
    std::shared_ptr<Services::IMicroService> Connect(const SharedConnection& data) override
    {
        if (!m_interface)
        {
            m_interface = import_creation_method<ServiceInterface>(data);
        }
        return m_interface;
    }

private:
    std::shared_ptr<ServiceInterface> m_interface;
};

}
}
