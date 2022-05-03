#pragma once

#include <api/entries/IExecutor.h>

namespace Gengine {
class IEntry;

namespace Entries {
class SimpleExecutor : public IExecutor {
 public:
  SimpleExecutor(IEntry& entry);
  virtual ~SimpleExecutor();

  bool Execute(void* args) override;
  bool GetCode(std::int32_t* exitCode) override;
  bool CreateProcessors(
      std::vector<std::unique_ptr<ICmdProcessor>>* processors) override;

 protected:
  IEntry& GetEntry() const;

 private:
  IEntry& m_entry;

 protected:
  std::int32_t m_code;
};

std::unique_ptr<IExecutor> makeServiceExecutor(IEntry& entry);

}  // namespace Entries
}  // namespace Gengine
