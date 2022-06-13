#include "NamedPipeChannel.h"

#include <Windows.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {
NamedPipeChannel::NamedPipeChannel()
    : m_socket(InvalidHandle), m_changeHandles(2), m_stopped(false) {
  m_stopSignal = CreateEvent(NULL, true, false, NULL);
  m_ioReady = CreateEvent(NULL, true, false, NULL);

  m_overlapped = new OVERLAPPED;
  memset(reinterpret_cast<OVERLAPPED*>(m_overlapped), 0, sizeof(OVERLAPPED));
  reinterpret_cast<OVERLAPPED*>(m_overlapped)->hEvent = m_ioReady;
}

NamedPipeChannel::NamedPipeChannel(HandleType handle) : NamedPipeChannel() {
  m_socket = handle;
}

NamedPipeChannel::~NamedPipeChannel() {
  if (m_socket != InvalidHandle) {
    ::CloseHandle(reinterpret_cast<HANDLE>(m_socket));
  }
  if (m_overlapped != nullptr) {
    delete reinterpret_cast<OVERLAPPED*>(m_overlapped);

	::CloseHandle(m_ioReady);
	::CloseHandle(m_stopSignal);
  }
}

bool NamedPipeChannel::Connect(const std::wstring& data) {
  m_stopped.store(false);

  m_socket = CreateFileW(data.c_str(), GENERIC_READ | GENERIC_WRITE,
                         0,                     // no sharing
                         NULL,                  // default security attributes
                         OPEN_EXISTING,         // opens existing pipe
                         FILE_FLAG_OVERLAPPED,  // default attributes
                         NULL);
  if (m_socket != INVALID_HANDLE_VALUE)
    return true;
  auto dwErr = GetLastError();
  if (dwErr != ERROR_PIPE_BUSY) {
    GLOG_ERROR_INTERNAL("Can not open pipe. Error %d", dwErr);
    return false;
  }

  // All pipe instances are busy, so wait for for some time while it will become
  // available
  if (!WaitNamedPipeW(data.c_str(), 1000)) {
    dwErr = GetLastError();
    GLOG_ERROR_INTERNAL("Could not open pipe:  timed out; Error %d", dwErr);
  }

  return true;
}

void NamedPipeChannel::Disconnect() {
  if (IsConnected()) {
    m_stopped.store(true);
	::SetEvent(m_stopSignal);

    ::CloseHandle(m_socket);
    m_socket = InvalidHandle;
  }
}

bool NamedPipeChannel::IsConnected() const {
  return m_socket != InvalidHandle;
}

bool NamedPipeChannel::Send(const void* data,
                            std::uint32_t size,
                            std::uint32_t* bytesProccessed) {
  if (!m_stopped.load()) {
    if (SendAsync(data, size)) {
      m_changeHandles[0] = m_stopSignal;
      m_changeHandles[1] = reinterpret_cast<HANDLE>(GetIOHandle());
      auto waitStatus = ::WaitForMultipleObjects(
          m_changeHandles.size(), &m_changeHandles[0], FALSE, INFINITE);
      switch (waitStatus) {
        case WAIT_OBJECT_0: {
          GLOG_ERROR("Named pipe disposed;");
          return false;
        } break;
        case (WAIT_OBJECT_0 + 1):
          // empty
          break;
        case WAIT_FAILED:
          GLOG_ERROR("Wait failed! Error: %d", ::GetLastError());
          break;
        case WAIT_TIMEOUT:
          GLOG_ERROR("Timeout in infinite mode? Is it possible?");
          break;
        default:
          GLOG_ERROR("unknown wait status: %u", waitStatus);
          break;
      }

      DWORD written{0};
      if (!GetOverlappedResult(m_socket,
                               reinterpret_cast<OVERLAPPED*>(m_overlapped),
                               &written, TRUE)) {
        auto dwErr = GetLastError();
        GLOG_ERROR("Failed Write to pipe %d", dwErr);
        assert(0);
        written = -1;
      }
      if (bytesProccessed)
        *bytesProccessed = (std::uint32_t)written;
      return written > 0;
    }
  }

  return false;
}

bool NamedPipeChannel::Recv(void* data,
                            std::uint32_t size,
                            std::uint32_t* bytesProccessed) {
  if (!m_stopped.load()) {
    if (RecvAsync(data, size)) {
      m_changeHandles[0] = m_stopSignal;
      m_changeHandles[1] = reinterpret_cast<HANDLE>(GetIOHandle());
      auto waitStatus = ::WaitForMultipleObjects(
          m_changeHandles.size(), &m_changeHandles[0], FALSE, INFINITE);
      switch (waitStatus) {
        case WAIT_OBJECT_0: {
          GLOG_ERROR("Named pipe disposed;");
          return false;
        } break;
        case (WAIT_OBJECT_0 + 1):
          // empty
          break;
        case WAIT_FAILED:
          GLOG_ERROR("Wait failed! Error: %d", ::GetLastError());
          break;
        case WAIT_TIMEOUT:
          GLOG_ERROR("Timeout in infinite mode? Is it possible?");
          break;
        default:
          GLOG_ERROR("unknown wait status: %u", waitStatus);
          break;
      }

      DWORD readed{0};
      if (!GetOverlappedResult(m_socket,
                               reinterpret_cast<OVERLAPPED*>(m_overlapped),
                               &readed, TRUE)) {
        auto dwErr = GetLastError();
        if (dwErr == ERROR_BROKEN_PIPE) {
          GLOG_ERROR("Connection closed");
          return false;
        }
        // weird error
        GLOG_ERROR("Failed receive from pipe %d", dwErr);
        assert(0);
        readed = -1;
      }
      if (bytesProccessed)
        *bytesProccessed = (std::uint32_t)readed;
      return readed > 0;
    }
  }

  return false;
}

bool NamedPipeChannel::SendAsync(const void* data, std::uint32_t size) {
  if (!m_stopped.load()) {
    DWORD processed = 0;
    BOOL ok = WriteFile(m_socket, data, size, &processed,
                        reinterpret_cast<OVERLAPPED*>(m_overlapped));
    DWORD err = GetLastError();
    if (!ok && err != ERROR_IO_PENDING) {
      if (err == ERROR_NO_DATA || err == ERROR_BROKEN_PIPE) {
        return false;
      }
      GLOG_ERROR("Failed Write to pipe %d", err);
      return false;
    }
    return true;
  }

  return false;
}

bool NamedPipeChannel::RecvAsync(void* data, std::uint32_t size) {
  if (!m_stopped.load()) {
    DWORD processed = 0;
    BOOL ok = ReadFile(m_socket, data, size, &processed,
                       reinterpret_cast<OVERLAPPED*>(m_overlapped));
    DWORD err = GetLastError();
    if (!ok && err != ERROR_IO_PENDING) {
      if (err == ERROR_NO_DATA || err == ERROR_BROKEN_PIPE) {
        return false;
      }
      GLOG_ERROR("Failed Read pipe %d", err);
      assert(0);
      return false;
    }
    return true;
  }

  return false;
}

void* NamedPipeChannel::GetIOHandle() const {
  return m_ioReady;
}

bool NamedPipeChannel::GetOverlapped(std::uint32_t* bytesProcessed) {
  DWORD processed = 0;
  BOOL ok = GetOverlappedResult(
      m_socket, reinterpret_cast<OVERLAPPED*>(m_overlapped), &processed, TRUE);
  if (bytesProcessed)
    *bytesProcessed = processed;
  if (!ok) {
    DWORD err = GetLastError();
    if (err == ERROR_BROKEN_PIPE)
      return false;
    if (err == ERROR_MORE_DATA) {
      GLOG_WARNING("More data available!");
      return true;
    }

    GLOG_ERROR("GetOverlappedResult failed! Error %d", err);
    assert(0);
    return false;
  }
  return true;
}

std::unique_ptr<IChannel> makeChannel(const std::wstring& connectionString) {
  auto channel = std::make_unique<NamedPipeChannel>();
  if (channel->Connect(connectionString)) {
    return channel;
  }
  return nullptr;
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
