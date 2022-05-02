#include <brokers/ServiceBroker.h>

#include <boost/signals2.hpp>

#include <shared-services/SharedServiceImport.h>

namespace Gengine {
using namespace SharedServices;

namespace Services {

namespace {
std::shared_ptr<IServiceBroker> GetServiceBroker()
{
    static std::shared_ptr<IServiceBroker> ServiceBroker;
    if (!ServiceBroker)
    {
        SharedConnection data;
        data.path = "service-broker";
        data.symbol = "ServiceBroker_service";
        ServiceBroker = import_symbol<IServiceBroker>(data);
    }
    assert(ServiceBroker);
    return ServiceBroker;
}
}

using service_signal = boost::signals2::signal<void(const TService&)>;

const TService& GetRouter()
{
    return GetServiceBroker()->GetRouter();
}

void InitializeServices(const std::unordered_map<std::string, ServiceConfig>& inServices, const std::unordered_map<std::string, ServiceConfig>& outServices)
{
    GetServiceBroker()->Configure(inServices, outServices);
}

void DeInitializeServices()
{
    GetServiceBroker()->Deconfigure();
}

StaticServiceRegistrator::StaticServiceRegistrator(const std::string& key, TServiceCreator&& creator, ServiceType preffered)
   : m_key(key)
   , m_creator(std::move(creator))
   , m_preffered(preffered)
{}

void StaticServiceRegistrator::Do()
{
    GetServiceBroker()->Register(m_key, std::move(m_creator), m_preffered);
}

void StaticServiceRegistrator::Undo()
{
    GetServiceBroker()->Unregister(m_key, m_preffered);
}

struct ServiceClient::ServiceClientImpl
{
   connection endpoint;
   std::string key;
};

ServiceClient::ServiceClient()
   : m_impl(std::make_unique<ServiceClientImpl>())
{}

ServiceClient::~ServiceClient()
{
    Disconnect();
}

void ServiceClient::Connect(const std::string& key)
{
    auto broker = GetServiceBroker();
    if (m_impl->key.empty())
    {
        m_impl->endpoint = broker->Subscribe(key, *this);
        m_impl->key = key;
    }
    broker->Touch(key, *this);
}

void ServiceClient::Disconnect()
{
    if (!m_impl->key.empty())
    {
        m_impl->endpoint.clear();
        GetServiceBroker()->Kill(m_impl->key);
        m_impl->key.clear();
    }
}

ServiceClientStub::ServiceClientStub(const std::string& key)
    : Worker(222)
    , m_key(key)
{}

void ServiceClientStub::Execute(TServiceCall call)
{
    if (InitService())
    {
        auto handler = [this, call, service = m_service]() {
            call(service);
        };
        GENGINE_POST_TASK(handler);
    }
    else
    {
        std::lock_guard<std::mutex> locker(m_serviceMtx);
        m_calls.push_back(call);
    }
}

void ServiceClientStub::OnServiceAvailable(const TService& service)
{
    std::lock_guard<std::mutex> locker(m_serviceMtx);
    m_service = service;
    Run();
}

void ServiceClientStub::OnServiceUnavailable(const TService& service)
{
    std::lock_guard<std::mutex> locker(m_serviceMtx);
    m_service.reset();
}

void ServiceClientStub::Run()
{
    if (m_service)
    {
        for (auto& call : m_calls)
        {
            auto handler = [this, call]() {
                call(m_service);
            };
            GENGINE_POST_TASK(handler);
        }

        m_calls.clear();
    }
}

bool ServiceClientStub::InitService()
{
    std::lock_guard<std::mutex> locker(m_serviceMtx);
    if (!m_service)
    {
        Connect(m_key);
        return false;
    }
    return true;
}

}
}
