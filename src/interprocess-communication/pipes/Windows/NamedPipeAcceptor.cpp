#include "NamedPipeAcceptor.h"
#include "NamedPipeChannel.h"

#include <interprocess-communication/InterprocessServer.h>
#include <interprocess-communication/CommunicationEngine.h>

#include <Windows.h>

#include <multithreading/Event.h>
#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

struct NamedPipeAcceptor::ListeningImpl
{
    ListeningImpl()
        : handle(InvalidHandle)
    {}

    void operator()()
    {
        handle = InvalidHandle;
        callback(true, std::move(channel));
    }

    void clear()
    {
        handle = InvalidHandle;
        callback = connected_callback();
        channel.reset();
    }

    HandleType handle;
    connected_callback callback;
    std::unique_ptr<IChannel> channel;
};

NamedPipeAcceptor::NamedPipeAcceptor(const std::wstring& pipe, const std::shared_ptr<CommunicationEngine>& engine)
    : m_engine(engine)
    , m_pipe(pipe)
    , m_listeningImpl(std::make_unique<ListeningImpl>())
{}

NamedPipeAcceptor::~NamedPipeAcceptor() = default;

void NamedPipeAcceptor::AcceptConnection(connected_callback callback)
{
    if (CreateListeningHandle())
    {
        auto channel = std::make_unique<NamedPipeChannel>(m_listeningImpl->handle);
        m_listeningImpl->callback = callback;
        OVERLAPPED ovr;
        memset(&ovr, 0, sizeof(ovr));
        ovr.hEvent = reinterpret_cast<HANDLE>(channel->GetIOHandle());
        m_listeningImpl->channel = std::move(channel);
        m_engine->RegisterConnection(*m_listeningImpl->channel.get(), [this] {
            GLOG_INFO("TRACE: Getting overlapped result...");
            DWORD dwUnused = 0;
            OVERLAPPED ovr = { 0 };
            if (::GetOverlappedResult(m_listeningImpl->handle, &ovr, &dwUnused, FALSE) == TRUE)
            {
                (*m_listeningImpl)();
            }
            else
            {
                auto dwMyErr = GetLastError();
                GLOG_ERROR("GetOverlappedResult failed %d", dwMyErr);
            
                if (m_listeningImpl->handle != InvalidHandle)
                {
                    ::CloseHandle(m_listeningImpl->handle);
                    m_listeningImpl->handle = InvalidHandle;
                }
            }
        });

        BOOL bResult = ConnectNamedPipe(m_listeningImpl->handle, &ovr);
        DWORD dwErr = GetLastError();
        if (bResult)
        {
            GLOG_ERROR("ConnectNamedPipe returns %d", bResult);
            assert(0);
        }
        if (dwErr == ERROR_PIPE_CONNECTED)
        {
            GLOG_INFO("Pipe  already connected. Setting event");
            ::SetEvent(ovr.hEvent);
        }
        else if (dwErr != ERROR_IO_PENDING)
        {
            GLOG_ERROR("Unknown error from ConnectNamedPipe %d", dwErr);
        }
    }
}

bool NamedPipeAcceptor::CreateListeningHandle()
{
    if (m_listeningImpl->handle == InvalidHandle)
    {
        SECURITY_ATTRIBUTES securityAttributes;
        memset(&securityAttributes, NULL, sizeof(SECURITY_ATTRIBUTES));
        securityAttributes.bInheritHandle = true;
        SECURITY_DESCRIPTOR descriptor;
        securityAttributes.lpSecurityDescriptor = &descriptor;
        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        InitializeSecurityDescriptor(securityAttributes.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(securityAttributes.lpSecurityDescriptor, true, NULL, false);
        m_listeningImpl->handle = CreateNamedPipeW(m_pipe.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            4 * 1024,
            4 * 1024,
            0,
            &securityAttributes);
        if (m_listeningImpl->handle == INVALID_HANDLE_VALUE)
        {
            DWORD dwErr = GetLastError();
            GLOG_ERROR("Failed create pipe %s; Error %d", m_pipe.c_str(), dwErr);
            assert(0);
            return false;
        }
    }

    return true;
}

std::unique_ptr<InterprocessAcceptor> makeAcceptor(const std::wstring& connectionString,
	const std::shared_ptr<CommunicationEngine>& engine)
{
	return std::make_unique<NamedPipeAcceptor>(connectionString, engine);
}

}
}