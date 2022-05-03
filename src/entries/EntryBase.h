#pragma once

#include <future>
#include <memory>
#include <vector>

#include <api/entries/IEntry.h>
#include <entries/diagnostic/IDumper.h>
#include <interprocess-syncronization/IAliveObject.h>
#include <interprocess-syncronization/ServiceTracker.h>

namespace Gengine {

namespace InterprocessSynchronization {
class InstanceRegistratorInterface;
}

struct EntryConfig;

namespace Entries {
class IEntryToolsFactory;
class CommandEntry : public IEntry {
 public:
  CommandEntry(std::unique_ptr<IEntryToolsFactory>&& factory) {}
  bool GetAppName(std::wstring* name) override { return true; }
  bool Initialize() override { return true; }
  bool Execute(void* args) override { return true; }
  bool Exit(std::int32_t* exitCode) override {
    *exitCode = 0;
    return true;
  }
  bool Finalize() override { return true; }
  bool EmergencyCleanUp() override { return true; }
  bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override {
    return true;
  }
  bool CreateProcessors(
      std::vector<std::unique_ptr<ICmdProcessor>>* processors) override {
    return true;
  }
  bool GetConfig(void** config) override {
    *config = nullptr;
    return true;
  }
};

class IEntryToolsFactory;
class EntryBase : public IEntry {
 public:
  EntryBase(std::unique_ptr<IEntryToolsFactory>&& factory);
  ~EntryBase();

  bool GetAppName(std::wstring* name) override;
  bool Initialize() override;
  bool Exit(std::int32_t* exitCode) override;
  bool Finalize() override;
  bool EmergencyCleanUp() override;
  bool GetConfig(void** config) override;

 private:
  std::vector<std::unique_ptr<Diagnostic::IDumper>> m_dumpers;
  mutable std::unique_ptr<EntryConfig> m_config;

 protected:
  std::unique_ptr<InterprocessSynchronization::InstanceRegistratorInterface>
      m_instanceRegistrator;
  std::unique_ptr<IEntryToolsFactory> m_factory;
};

class MasterEntry : public EntryBase {
 public:
  MasterEntry(std::unique_ptr<IEntryToolsFactory>&& factory);

  bool Initialize() override;
  bool EmergencyCleanUp() override;

 private:
  std::unique_ptr<InterprocessSynchronization::IAliveObject> m_aliveObject;
};

class SlaveEntry : public EntryBase {
 public:
  SlaveEntry(std::unique_ptr<IEntryToolsFactory>&& factory);
  ~SlaveEntry();

  bool Initialize() override;
  bool Execute(void* args) override;
  bool Exit(std::int32_t* exitCode) override;
  void Terminate();

 private:
  struct SlaveEntryImpl;
  std::unique_ptr<SlaveEntryImpl> m_impl;
};
}  // namespace Entries
}  // namespace Gengine
