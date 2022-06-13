#include "PatcherModule.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <core/Encoding.h>
#include <core/Logger.h>
#include <filesystem/Filesystem.h>

#include <appconfig/AppConfig.h>
#include <appconfig/BufferConfigReader.h>
#include <appconfig/FileConfigReader.h>
#include <entries/EntryRegistry.h>
#include <entries/Main.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace fs = boost::filesystem;
namespace bpo = boost::program_options;

using namespace Gengine;
using namespace Entries;
using namespace AppConfig;

const std::string ConfigDelemiterStart("#@-----%1%_CONFIG_START__-----@#");
const std::string ConfigDelemiterEnd("#@-----%1%_CONFIG_END__-----@#");

namespace {
template <class ConfigType>
std::string GetConfigurationImpl(const std::wstring& path) {
  ConfigType config;
  auto reader = makeJsonConfigReader<FileConfigReader>(config, path);
  if (reader.Load()) {
    auto bufferReader = makeBinaryConfigReader<BufferConfigReader>(config);
    bufferReader.Save();
    return bufferReader.GetBuffer();
  }
  return std::string();
}
}  // namespace

PatcherModule::PatcherBase::PatcherBase(const wchar_t* command)
    : CmdProcessorBase(command) {}

bool PatcherModule::PatcherBase::Register(void* options) {
  assert(options);
  auto& programOptions =
      *reinterpret_cast<bpo::options_description_easy_init*>(options);
  CmdProcessorBase::Register(options);
  programOptions("file",
                 boost::program_options::wvalue<std::wstring>(&m_filePath),
                 "file")(
      "config", boost::program_options::wvalue<std::wstring>(&m_configPath),
      "config");
  return true;
}

bool PatcherModule::PatcherBase::Process(void*, bool* success) {
  assert(success);
  *success = false;
  if (Filesystem::IsFileExists(m_filePath) &&
      Filesystem::IsFileExists(m_configPath)) {
    auto configBuff = GetConfiguration();
    if (!configBuff.empty()) {
      fs::ofstream file(m_filePath, std::ios::binary | std::ios::app);
      if (file.is_open()) {
        auto fileName = Filesystem::GetFileNameWithoutExtension(
            fs::path(m_filePath).filename().wstring());
        auto begin =
            boost::str(boost::format(ConfigDelemiterStart) % toUtf8(fileName));
        auto end =
            boost::str(boost::format(ConfigDelemiterEnd) % toUtf8(fileName));
        file << begin << configBuff << end;
        *success = true;
      }
    }
  }
  return true;
}

PatcherModule::EntryPatcher::EntryPatcher() : PatcherBase(L"executable") {}

std::string PatcherModule::EntryPatcher::GetConfiguration() const {
  return GetConfigurationImpl<EntryConfig>(m_configPath);
}

PatcherModule::PluginPatcher::PluginPatcher() : PatcherBase(L"plugin") {}

std::string PatcherModule::PluginPatcher::GetConfiguration() const {
  return GetConfigurationImpl<PluginConfig>(m_configPath);
}

PatcherModule::PatcherModule(std::unique_ptr<IEntryToolsFactory>&& f)
    : CommandEntry(std::move(f)) {}

bool PatcherModule::CreateProcessors(
    std::vector<std::unique_ptr<ICmdProcessor>>* processors) {
  assert(processors);
  processors->emplace_back(std::make_unique<EntryPatcher>());
  processors->emplace_back(std::make_unique<PluginPatcher>());
  return true;
}

REGISTER_MAIN_ENTRY(PatcherModule)
IMPLEMENT_ENTRY
