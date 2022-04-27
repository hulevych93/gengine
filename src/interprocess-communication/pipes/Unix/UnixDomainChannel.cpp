#include <interprocess-communication/pipes/Unix/UnixDomainChannel.h>
#include <interprocess-communication/pipes/MacOS/UnixSocketEngine.h>

#include <assert.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

UnixDomainChannel::UnixDomainChannel()
    : m_socket(INVALID_RPC_FILE)
    , m_stopped(false)
{}

UnixDomainChannel::UnixDomainChannel(const std::shared_ptr<UnixSocketEngine>& engine)
    : m_socket(INVALID_RPC_FILE)
    , m_stopped(false)
    , m_engine(engine)
{
}

UnixDomainChannel::UnixDomainChannel(RPC_FILE_HANDLE handle, const std::shared_ptr<UnixSocketEngine>& engine)
    : m_socket(handle)
    , m_stopped(false)
    , m_engine(engine)
{}

UnixDomainChannel::~UnixDomainChannel()
{
    Disconnect();
}

bool UnixDomainChannel::Connect(const std::wstring &serverSocketFileName)
{
    m_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (m_socket == INVALID_RPC_FILE)
    {
        assert(0);
        return false;
    }
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    std::string strSocketFileName = toUtf8(serverSocketFileName);
    strcpy(addr.sun_path, strSocketFileName.c_str());
    auto result = connect(m_socket, (sockaddr*)&addr, SUN_LEN(&addr));
    if (result == -1)
    {
        GLOG_ERROR("Connect error %d", errno);
        Disconnect();
        return false;
    }
    return true;
}

void UnixDomainChannel::Disconnect()
{
    if(IsConnected())
    {
        m_stopped.store(true);
        if (m_socket != INVALID_RPC_FILE)
        {
            close(m_socket);
            m_socket = INVALID_RPC_FILE;
        }
    }
}

bool UnixDomainChannel::IsConnected() const
{
    return m_socket != INVALID_RPC_FILE;
}

bool UnixDomainChannel::Send(const void *data, std::uint32_t size, std::uint32_t* bytesProccessed)
{
    if (!m_stopped.load())
    {
        int res = send(m_socket, data, size, 0);
        if (res <= 0)
        {
            return errno == EWOULDBLOCK;
        }

        if(bytesProccessed)
            *bytesProccessed = res;
        return true;
    }

    return false;
}

bool UnixDomainChannel::Recv(void *data, std::uint32_t size, std::uint32_t* bytesProccessed)
{
    if (!m_stopped.load())
    {
        std::uint32_t processed = 0;
        while (processed < size)
        {
            const auto res = recv(m_socket, reinterpret_cast<std::uint8_t*>(data) + processed, size - processed, 0);
            if (res <= 0)
            {
                return errno == EWOULDBLOCK;
            }
            processed += res;
        }
        if(bytesProccessed)
            *bytesProccessed = processed;
        return processed > 0;
    }

    return false;
}

bool UnixDomainChannel::SendAsync(const void* data, std::uint32_t size)
{
    m_engine->PostWrite(*this, data, size);
    return true;
}

bool UnixDomainChannel::RecvAsync(void* data, std::uint32_t size)
{
    m_engine->PostRead(*this, data, size);
    return true;
}

}
}
