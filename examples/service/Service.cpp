#include <gengine/gengine.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

using namespace Gengine;
using namespace Gengine::Services;
using namespace Gengine::Entries;

class TestService final : public Entries::EntryBase, public Worker {
 public:
  TestService(std::unique_ptr<Entries::IEntryToolsFactory>&& factory)
      : EntryBase(std::move(factory)), Worker(0) {}

  ~TestService() = default;

  bool Initialize() override { return true; }

  bool Execute(void* args) override {
    assert(args);

    auto handler = [&]() { ++m_ticks; };

    m_timerId = GENGINE_START_TIMER(std::move(handler), std::chrono::seconds{5});

    return true;
  }

  bool Exit(std::int32_t* exitCode) override {
    assert(exitCode);

    GENGINE_STOP_TIMER_WITH_WAIT(m_timerId);

    *exitCode = 0;

    return true;
  }

  bool Finalize() override { return true; }

  bool CreateExecutor(std::shared_ptr<IExecutor>* executor) override {
    assert(executor);
    *executor = makeServiceExecutor(*this);
    return true;
  }

  bool CreateProcessors(std::vector<std::unique_ptr<Gengine::ICmdProcessor>>*
                            processors) override {
    assert(processors);
    return true;
  }

 private:
  std::uint32_t m_ticks = 0u;
  std::int32_t m_timerId;
};

REGISTER_MAIN_ENTRY(TestService)
IMPLEMENT_CONSOLE_ENTRY
