#pragma once

#include <core/Runnable.h>
#include <memory>
#include <thread>
#include <vector>

namespace boost {
namespace asio {
class io_context;
}
}  // namespace boost

namespace Gengine {
namespace Multithreading {
class ThreadPoolManager : public Runnable {
 protected:
  explicit ThreadPoolManager(std::int32_t threads = -1);

 public:
  ThreadPoolManager(const ThreadPoolManager&) = delete;
  ThreadPoolManager(ThreadPoolManager&&) = delete;

  ThreadPoolManager& operator=(const ThreadPoolManager&) = delete;
  ThreadPoolManager& operator=(ThreadPoolManager&&) = delete;

  ~ThreadPoolManager();

 public:
  std::shared_ptr<boost::asio::io_context> GetIOService();

  inline static ThreadPoolManager& GetInstance(std::int32_t threads = -1) {
    static ThreadPoolManager instance(threads);
    return instance;
  }

 protected:
  void StartInternal() override;
  void StopInternal() override;

 private:
  std::shared_ptr<boost::asio::io_context> m_ioService;
  void* m_ioServiceWork;
  std::vector<std::shared_ptr<std::thread>> m_threads;
  std::size_t m_threadCount;
};
}  // namespace Multithreading
}  // namespace Gengine