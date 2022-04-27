#include <atomic>
#include <unordered_map>
#include <brokers/WorkerBroker.h>

#include <core/AbstractFactory.h>
#include <core/Encoding.h>
#include <multithreading/WorkerThread.h>

#include <shared-services/SharedServiceExport.h>
#include <shared-services/ISharedLibraryClient.h>

namespace Gengine {
using namespace InterprocessSynchronization;
using namespace InterprocessCommunication;
using namespace Multithreading;
using namespace SharedServices;
using namespace Services;

namespace Services {

class WorkerBroker : public IWorkerBroker
{
public:
    WorkerBroker();
   ~WorkerBroker();

public:
   void Configure(const std::set<ThreadConfig>& config) override;
   std::shared_ptr<IWorkerThread> GetThread(std::uint8_t id) override;
   void Shutdown() override;

private:
   struct ThreadContext
   {
       std::uint32_t id;
       std::wstring name;
       std::shared_ptr<IWorkerThread> thread;
   };
   std::unordered_map<std::uint32_t, std::unique_ptr<ThreadContext>> m_contexts;
   std::mutex m_threadMutex;
   std::atomic_bool m_run;
};

WorkerBroker::WorkerBroker()
    : m_run(true)
{}

WorkerBroker::~WorkerBroker()
{
    Shutdown();
}

void WorkerBroker::Configure(const std::set<ThreadConfig>& config)
{
    for (const auto& conf : config)
    {
        auto context = std::make_unique<ThreadContext>();
        context->id = conf.id;
        context->name = utf8toWchar(conf.name);
        m_contexts.emplace(std::make_pair(conf.id, std::move(context)));
    }
}

std::shared_ptr<IWorkerThread> WorkerBroker::GetThread(std::uint8_t id)
{
   auto heartbeatThread = std::shared_ptr<IWorkerThread>();

   if (m_run.load())
   {
       auto it = m_contexts.find(id);
       if (it != m_contexts.end())
       {
           auto& context = it->second;
           if (!context->thread)
           {
               context->thread = std::shared_ptr<WorkerThread>(new WorkerThread(context->name));
           }

           heartbeatThread = context->thread;
       }
       else
       {
           //LOGERROR
       }
   }

   return heartbeatThread;
}

void WorkerBroker::Shutdown()
{
    m_run.store(false);
    for (const auto& contextIter : m_contexts)
    {
        auto thread = contextIter.second->thread;
        if(thread)
            thread->Dispose();
    }
    m_contexts.clear();
}

EXPORT_GLOBAL_SHARED_SERVICE(WorkerBroker)

}
}
