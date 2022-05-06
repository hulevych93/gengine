#include <interprocess-communication/pipes/Unix/UnixDomainChannel.h>
#include <interprocess-communication/pipes/Unix/UnixSocketAcceptor.h>
#include <interprocess-communication/pipes/Unix/UnixSocketEngine.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <signal.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

constexpr const std::uint32_t MaxConnectionsCount = 20u;

UnixSocketAcceptor::UnixSocketAcceptor(
    const std::wstring& socketFileName,
    const std::shared_ptr<CommunicationEngine>& engine)
    : m_socketFileName(socketFileName), m_engine(engine) {}

UnixSocketAcceptor::~UnixSocketAcceptor() = default;

void UnixSocketAcceptor::AcceptConnection(connected_callback callback) {
  if (!m_channel) {
    auto listeningHandle = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (listeningHandle != InvalidHandle) {
      int nonBlocking = 1;
      const int nRet = ioctl(listeningHandle, FIONBIO, &nonBlocking);
      if (nRet != -1) {
        const auto socketFile = toUtf8(m_socketFileName);

        sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_LOCAL;

        strcpy(addr.sun_path, socketFile.c_str());
        GLOG_INFO("Binding to %s", socketFile.c_str());

        unlink(socketFile.c_str());
        if (bind(listeningHandle, (sockaddr*)&addr, SUN_LEN(&addr)) != -1) {
          if (listen(listeningHandle, MaxConnectionsCount) != -1) {
            m_channel = std::make_unique<UnixDomainChannel>(
                listeningHandle,
                std::static_pointer_cast<UnixSocketEngine>(m_engine));
            m_engine->RegisterConnection(*m_channel, [=] {
              GLOG_INFO("TRACE: Getting overlapped result...");

              auto acceptedSocket = accept(listeningHandle, NULL, NULL);
              if (acceptedSocket != InvalidHandle) {
                if (fcntl(acceptedSocket, F_SETFL,
                          fcntl(acceptedSocket, F_GETFL) | O_NONBLOCK) != -1) {
                  linger lingerStruct = {0};
                  lingerStruct.l_onoff = 1;
                  lingerStruct.l_linger = 0;

                  if (::setsockopt(acceptedSocket, SOL_SOCKET, SO_LINGER,
                                   (char*)&lingerStruct,
                                   sizeof(lingerStruct)) != -1) {
                    callback(true,
                             std::make_unique<UnixDomainChannel>(
                                 acceptedSocket,
                                 std::static_pointer_cast<UnixSocketEngine>(
                                     m_engine)));
                    return;
                  }
                }

                close(acceptedSocket);
                callback(false, nullptr);
              }
            });

            return;
          }
        }

        close(listeningHandle);
      }
    }
  }
}

std::unique_ptr<InterprocessAcceptor> makeAcceptor(
    const std::wstring& connectionString,
    const std::shared_ptr<CommunicationEngine>& engine) {
  return std::make_unique<UnixSocketAcceptor>(connectionString, engine);
}

}  // namespace InterprocessCommunication
}  // namespace Gengine
