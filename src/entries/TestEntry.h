#pragma once

#include <entries/EntryBase.h>
#include <entries/SimpleExecutor.h>

namespace Gengine {
namespace Entries {
class GTestModule : public MasterEntry {
 public:
  GTestModule(std::unique_ptr<IEntryToolsFactory>&& factory);
  ~GTestModule() = default;

  bool Execute(void* args) override;
  bool Exit(std::int32_t* exitCode) override;
  bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override;
  bool CreateProcessors(std::vector<std::unique_ptr<ICmdProcessor>>*) override;
};

}  // namespace Entries
}  // namespace Gengine
