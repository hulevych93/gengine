#pragma once

#include <interprocess-communication/IChannel.h>
#include <atomic>

namespace Gengine {
namespace InterprocessCommunication {
class NamedPipeChannel : public IChannel {
 public:
  NamedPipeChannel();
  NamedPipeChannel(HandleType handle);
  ~NamedPipeChannel();

  NamedPipeChannel(const NamedPipeChannel&) = delete;
  NamedPipeChannel(NamedPipeChannel&&) = delete;

  bool Connect(const std::wstring& data);
  void* GetIOHandle() const;
  bool GetOverlapped(std::uint32_t* uiBytesTransferred);

 protected:
  void Disconnect() override;
  bool IsConnected() const override;
  bool Send(const void* data,
            std::uint32_t size,
            std::uint32_t* bytesProccessed = nullptr) override;
  bool Recv(void* data,
            std::uint32_t size,
            std::uint32_t* bytesProccessed = nullptr) override;
  bool SendAsync(const void* data, std::uint32_t size) override;
  bool RecvAsync(void* data, std::uint32_t size) override;

 private:
  HandleType m_socket;
  std::atomic<bool> m_stopped;
  mutable void* m_ioReady;
  void* m_stopSignal;
  std::vector<void*> m_changeHandles;
  void* m_overlapped;
};
}  // namespace InterprocessCommunication
}  // namespace Gengine