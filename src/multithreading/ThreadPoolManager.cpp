#include "ThreadPoolManager.h"
#include <core/Logger.h>

#include <boost/asio/io_service.hpp>

namespace Gengine {
namespace Multithreading {
ThreadPoolManager::ThreadPoolManager(std::int32_t threads)
    : m_ioServiceWork(nullptr),
      m_threadCount(threads == -1 ? std::thread::hardware_concurrency()
                                  : threads) {}

ThreadPoolManager::~ThreadPoolManager() {}

std::shared_ptr<boost::asio::io_service> ThreadPoolManager::GetIOService() {
  return m_ioService;
}

void ThreadPoolManager::StartInternal() {
  m_ioService = std::make_shared<boost::asio::io_service>();

  m_ioServiceWork = new boost::asio::io_service::work(*m_ioService);

  GLOG_INFO("starting %d threads", m_threadCount);

  for (std::size_t i = 0; i < m_threadCount; i++) {
    auto heartbeatThread = std::make_shared<std::thread>(
        boost::bind(&boost::asio::io_service::run, m_ioService));
    m_threads.push_back(heartbeatThread);
  }
}

void ThreadPoolManager::StopInternal() {
  GLOG_INFO("Stopping thread pool manager: %d threads", m_threads.size());

  if (m_ioServiceWork) {
    delete (boost::asio::io_service::work*)m_ioServiceWork;
    m_ioServiceWork = nullptr;
  }

  m_ioService->stop();

  for (std::size_t i = 0; i < m_threads.size(); ++i) {
    m_threads[i]->join();
  }

  m_threads.clear();
  m_ioService.reset();
}
}  // namespace Multithreading
}  // namespace Gengine
