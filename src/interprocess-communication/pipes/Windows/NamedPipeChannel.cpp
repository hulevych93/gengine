#include "NamedPipeChannel.h"

#include <Windows.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {
NamedPipeChannel::NamedPipeChannel()
    : m_socket(INVALID_RPC_FILE)
    , m_changeHandles(2)
    , m_stopped(false)
{
    m_ioReady.Create(true, false);
    m_overlapped = new OVERLAPPED;
    memset(reinterpret_cast<OVERLAPPED*>(m_overlapped), 0, sizeof(OVERLAPPED));
    reinterpret_cast<OVERLAPPED*>(m_overlapped)->hEvent = m_ioReady.GetOSHandle();
    m_evtStopped.Create(true, false);
}

NamedPipeChannel::NamedPipeChannel(RPC_FILE_HANDLE handle)
    : NamedPipeChannel()
{
    m_socket = handle;
}

NamedPipeChannel::~NamedPipeChannel()
{
    if (m_socket != INVALID_RPC_FILE)
    {
        ::CloseHandle(reinterpret_cast<HANDLE>(m_socket));
    }
    if (m_overlapped != nullptr)
    {
        delete reinterpret_cast<OVERLAPPED*>(m_overlapped);
    }
}

bool NamedPipeChannel::Connect(const std::wstring& data)
{
    m_stopped.store(false);

    m_socket = CreateFileW(data.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,              // no sharing 
        NULL,           // default security attributes
        OPEN_EXISTING,  // opens existing pipe 
        FILE_FLAG_OVERLAPPED,              // default attributes 
        NULL);
    if (m_socket != INVALID_HANDLE_VALUE)
        return true;
    auto dwErr = GetLastError();
    if (dwErr != ERROR_PIPE_BUSY)
    {
        GLOG_ERROR_INTERNAL("Can not open pipe. Error %d", dwErr);
        return false;
    }

    // All pipe instances are busy, so wait for for some time while it will become available
    if (!WaitNamedPipeW(data.c_str(), 1000))
    {
        dwErr = GetLastError();
        GLOG_ERROR_INTERNAL("Could not open pipe:  timed out; Error %d", dwErr);
    }

    return true;
}

void NamedPipeChannel::Disconnect()
{
    if (IsConnected())
    {
        m_stopped.store(true);
        m_evtStopped.Set();

        ::CloseHandle(m_socket);
        m_socket = INVALID_RPC_FILE;
    }
}

bool NamedPipeChannel::IsConnected() const
{
    return m_socket != INVALID_RPC_FILE;
}

bool NamedPipeChannel::Send(const void* data, std::uint32_t size, std::uint32_t* bytesProccessed)
{
    if (!m_stopped.load())
    {
        if (SendAsync(data, size))
        {
            m_changeHandles[0] = m_evtStopped.GetOSHandle();
            m_changeHandles[1] = reinterpret_cast<HANDLE>(GetIOHandle());
            auto waitStatus = ::WaitForMultipleObjects(m_changeHandles.size(), &m_changeHandles[0], FALSE, INFINITE);
            switch (waitStatus)
            {
            case WAIT_OBJECT_0:
            {
                GLOG_ERROR("Named pipe disposed;");
                return false;
            }
            break;
            case (WAIT_OBJECT_0 + 1):
                //empty
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

            DWORD written{ 0 };
            if (!GetOverlappedResult(m_socket, reinterpret_cast<OVERLAPPED*>(m_overlapped), &written, TRUE))
            {
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

bool NamedPipeChannel::Recv(void* data, std::uint32_t size, std::uint32_t* bytesProccessed)
{
    if (!m_stopped.load())
    {
        if (RecvAsync(data, size))
        {
            m_changeHandles[0] = m_evtStopped.GetOSHandle();
            m_changeHandles[1] = reinterpret_cast<HANDLE>(GetIOHandle());
            auto waitStatus = ::WaitForMultipleObjects(m_changeHandles.size(), &m_changeHandles[0], FALSE, INFINITE);
            switch (waitStatus)
            {
            case WAIT_OBJECT_0:
            {
                GLOG_ERROR("Named pipe disposed;");
                return false;
            }
            break;
            case (WAIT_OBJECT_0 + 1):
                //empty
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

            DWORD readed{ 0 };
            if (!GetOverlappedResult(m_socket, reinterpret_cast<OVERLAPPED*>(m_overlapped), &readed, TRUE))
            {
                auto dwErr = GetLastError();
                if (dwErr == ERROR_BROKEN_PIPE)
                {
                    GLOG_ERROR("Connection closed");
                    return false;
                }
                //weird error
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

bool NamedPipeChannel::SendAsync(const void* data, std::uint32_t size)
{
    if (!m_stopped.load())
    {
        DWORD dwWritten = 0;
        BOOL bOk = WriteFile(m_socket, data, size, &dwWritten, reinterpret_cast<OVERLAPPED*>(m_overlapped));
        DWORD dwErr = GetLastError();
        if (!bOk && dwErr != ERROR_IO_PENDING)
        {
            if (dwErr == ERROR_NO_DATA ||
                dwErr == ERROR_BROKEN_PIPE)
            {
                GLOG_WARNING("Server pipe being closed");
                return false;
            }
            GLOG_ERROR("Failed Write to pipe %d", dwErr);
            return false;
        }
        return true;
    }

    return false;
}

bool NamedPipeChannel::RecvAsync(void* data, std::uint32_t size)
{
    if (!m_stopped.load())
    {
        DWORD dwRead = 0;
        BOOL bOk = ReadFile(m_socket, data, size, &dwRead, reinterpret_cast<OVERLAPPED*>(m_overlapped));
        DWORD dwErr = GetLastError();
        if (!bOk && dwErr != ERROR_IO_PENDING)
        {
            if (dwErr == ERROR_NO_DATA ||
                dwErr == ERROR_BROKEN_PIPE)
            {
                GLOG_WARNING("Server pipe being closed");
                return false;
            }
            GLOG_ERROR("Failed Read pipe %d", dwErr);
            assert(0);
            return false;
        }
        return true;
    }

    return false;
}

void* NamedPipeChannel::GetIOHandle() const
{
    return m_ioReady.GetOSHandle();
}

bool NamedPipeChannel::GetOverlapped(std::uint32_t *uiBytesTransferred)
{
    DWORD nTransferred = 0;
    BOOL bOk = GetOverlappedResult(m_socket, reinterpret_cast<OVERLAPPED*>(m_overlapped), &nTransferred, TRUE);
    *uiBytesTransferred = nTransferred;
    if (!bOk)
    {
        DWORD dwErr = GetLastError();
        if (dwErr == ERROR_BROKEN_PIPE)
            return false;//client exited unexpectedly
        if (dwErr == ERROR_MORE_DATA)
        {
            GLOG_WARNING("More data available!");
            return true;//seems, client send more data, we will process it later
        }
        //oops - unexpected error
        GLOG_ERROR("GetOverlappedResult failed! Error %d", dwErr);
        assert(0);
        return false;
    }
    return true;
}

}
}