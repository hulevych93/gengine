#include <brokers/PersistencyBroker.h>

#include <shared-services/SharedServiceImport.h>

namespace Gengine {
using namespace SharedServices;

namespace Services {

namespace {
std::shared_ptr<IPersistencyBroker> GetBroker()
{
    static std::shared_ptr<IPersistencyBroker> PersistencyBroker;
    if (!PersistencyBroker)
    {
        SharedConnection data;
        data.path = "persistency-broker";
        data.symbol = "PersistencyBroker_service";
        PersistencyBroker = import_symbol<IPersistencyBroker>(data);
    }
    assert(PersistencyBroker);
    return PersistencyBroker;
}
}

void InitializePersistency(const std::string& directory)
{
    GetBroker()->Configure(directory);
}

void DeinitializePersistency()
{
    GetBroker()->Deconfigure();
}

PersistableBase::PersistableBase(const std::string& id)
    : m_id(id)
{
    m_endpoint = GetBroker()->Subscribe(m_id, *this);
}

PersistableBase::~PersistableBase()
{
    m_endpoint.clear();
}

void PersistableBase::Load()
{
    GetBroker()->Load(m_id);
}

void PersistableBase::Store()
{
    GetBroker()->Store(m_id);
}

void PersistableBase::Delete()
{
    GetBroker()->Delete(m_id);
}

}
}
