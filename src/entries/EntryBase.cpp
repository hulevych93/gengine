#include "EntryBase.h"

#include <appconfig/AppConfig.h>
#include <appconfig/SelfExtractedBufferedConfigReader.h>
#include <diagnostic/IDumper.h>
#include <interprocess-syncronization/IAliveObject.h>
#include <interprocess-syncronization/InstanceRegistratorInterface.h>

#include <core/Encoding.h>
#include <core/Logger.h>

#include <api/entries/IExecutor.h>
#include <entries/IEntryToolsFactory.h>

#include <api/core/ILifetime.h>
#include <brokers/ExecutorBroker.h>

using namespace Gengine::Diagnostic;
using namespace Gengine::InterprocessSynchronization;
using namespace Gengine::AppConfig;

namespace Gengine {
namespace Entries {

EntryBase::EntryBase(std::unique_ptr<IEntryToolsFactory>&& factory)
    : m_factory(std::move(factory)) {}

EntryBase::~EntryBase() = default;

bool EntryBase::Initialize() {
  std::wstring name;
  GetAppName(&name);
  m_dumpers = m_factory->CreateDumpers(name);
  return true;
}

bool EntryBase::Finalize() {
  void* config_;
  GetConfig(&config_);
  const auto config = reinterpret_cast<EntryConfig*>(config_);
  if (config->watchdogTime) {
    auto terminationHandler = [this,
                               watchdogTime = config->watchdogTime.get()]() {
      std::this_thread::sleep_for(std::chrono::seconds(watchdogTime));
      EmergencyCleanUp();
      std::exit(1);
    };
    std::thread(terminationHandler).detach();
  }
  return true;
}

bool EntryBase::EmergencyCleanUp() {
  for (const auto& dumper : m_dumpers) {
    dumper->WriteDump();
  }
  return true;
}

bool EntryBase::GetAppName(std::wstring* name) {
  assert(name);

  void* config_;
  GetConfig(&config_);
  const auto config = reinterpret_cast<EntryConfig*>(config_);
  if (config->name) {
    *name = utf8toWchar(config->name.get());
    return true;
  }

  return false;
}

bool EntryBase::GetConfig(void** config) {
  assert(config);

  if (!m_config) {
    m_config = std::make_unique<EntryConfig>();
    if (!SelfExtractedBufferedConfigReader(*m_config).Load())
      throw std::runtime_error("no service config loaded...");
  }

  *config = m_config.get();

  return true;
}

bool EntryBase::Exit(std::int32_t* exitCode) {
  *exitCode = 0;
  return true;
}

MasterEntry::MasterEntry(std::unique_ptr<IEntryToolsFactory>&& factory)
    : EntryBase(std::move(factory)) {}

bool MasterEntry::Initialize() {
  void* config_;
  GetConfig(&config_);
  const auto config = reinterpret_cast<EntryConfig*>(config_);

  bool initialized = EntryBase::Initialize();
  if (initialized && config->singleObjectName) {
    m_instanceRegistrator = m_factory->CreateInstanceRegistrator(
        utf8toWchar(config->singleObjectName.get()));
    initialized = m_instanceRegistrator->RegisterInstance();
  }
  if (initialized && config->aliveObjectName) {
    m_aliveObject = m_factory->CreateAliveObject(
        utf8toWchar(config->aliveObjectName.get()));
  }
  return initialized;
}

bool MasterEntry::EmergencyCleanUp() {
  EntryBase::EmergencyCleanUp();
  m_aliveObject->Free();
  return true;
}

struct SlaveEntry::SlaveEntryImpl : public ILifetime {
  SlaveEntryImpl(SlaveEntry& entry) : entry(entry), running(true) {}

  bool Shutdown(bool* ok) {
    *ok = true;
    entry.Terminate();
    return true;
  }

  std::unique_ptr<Services::ServiceObjectProxy<ILifetime&>> lifetime;
  std::unique_ptr<InterprocessSynchronization::ServiceTracker> tracker;
  std::future<void> run;
  std::promise<void> promise;
  std::atomic<bool> running;
  SlaveEntry& entry;
};

SlaveEntry::SlaveEntry(std::unique_ptr<IEntryToolsFactory>&& factory)
    : EntryBase(std::move(factory)),
      m_impl(std::make_unique<SlaveEntryImpl>(*this)) {}

SlaveEntry::~SlaveEntry() = default;

bool SlaveEntry::Initialize() {
  void* config_;
  GetConfig(&config_);
  auto config = reinterpret_cast<EntryConfig*>(config_);

  bool initialized = EntryBase::Initialize();
  if (initialized && config->singleObjectName) {
    m_instanceRegistrator = m_factory->CreateInstanceRegistrator(
        utf8toWchar(config->singleObjectName.get()),
        InstanceType::OnePerUserSession);
    initialized = m_instanceRegistrator->RegisterInstance();
  }
  if (initialized && config->aliveObjectName) {
    m_impl->run = m_impl->promise.get_future();
    m_impl->tracker = m_factory->CreateModuleTracker(
        utf8toWchar(config->aliveObjectName.get()), [&]() { Terminate(); });
    m_impl->tracker->Start();
  }
  if (initialized && config->lifetimeServiceId) {
    if (!m_impl->run.valid()) {
      m_impl->run = m_impl->promise.get_future();
    }
    m_impl->lifetime =
        std::make_unique<Services::ServiceObjectProxy<ILifetime&>>(
            config->lifetimeServiceId.get(), *m_impl.get());
    m_impl->lifetime->Reveal();
  }
  return initialized;
}

void SlaveEntry::Terminate() {
  if (m_impl->running.load()) {
    m_impl->promise.set_value();
    m_impl->running.store(false);

    auto terminationHandler = [this]() {
      std::this_thread::sleep_for(std::chrono::seconds(30));
      EmergencyCleanUp();
      std::exit(1);
    };
    std::thread(terminationHandler).detach();
  }
}

bool SlaveEntry::Execute(void*) {
  if (m_impl->run.valid()) {
    m_impl->run.wait();
  }
  return true;
}

bool SlaveEntry::Exit(std::int32_t* exitCode) {
  if (m_impl->tracker) {
    m_impl->tracker->Stop();
    m_impl->tracker.reset();
  }
  if (m_impl->lifetime) {
    m_impl->lifetime->Hide();
  }
  return EntryBase::Exit(exitCode);
}

}  // namespace Entries
}  // namespace Gengine