#pragma once

#include <entries/CmdProcessorBase.h>
#include <entries/EntryBase.h>
#include <memory>

namespace Gengine {
class PatcherModule : public Entries::CommandEntry {
 public:
  PatcherModule(std::unique_ptr<Entries::IEntryToolsFactory>&&);
  bool CreateProcessors(
      std::vector<std::unique_ptr<ICmdProcessor>>* processors) override;

 private:
  class PatcherBase : public Entries::CmdProcessorBase {
   public:
    PatcherBase(const wchar_t* command);

    bool Register(void* options) override;
    bool Process(void* args, bool* success) override;

   private:
    virtual std::string GetConfiguration() const = 0;

   private:
    std::wstring m_filePath;

   protected:
    std::wstring m_configPath;
  };

  class EntryPatcher : public PatcherBase {
   public:
    EntryPatcher();

   protected:
    std::string GetConfiguration() const override;
  };

  class PluginPatcher : public PatcherBase {
   public:
    PluginPatcher();

   protected:
    std::string GetConfiguration() const override;
  };
};
}  // namespace Gengine