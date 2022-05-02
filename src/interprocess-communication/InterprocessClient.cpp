#include "InterprocessClient.h"

#include "IChannel.h"
#include "InputParameters.h"
#include "OutputParameters.h"
#include "ChannelConnector.h"

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessCommunication {

InterprocessClient::InterprocessClient() = default;

InterprocessClient::~InterprocessClient()
{
    Dispose();
}

bool InterprocessClient::Connect(const ipc_connection& data)
{
    if (m_impl && m_impl->IsConnected())
        return true;

    return boost::apply_visitor(ChannelConnector(73, m_impl), data);
}

void InterprocessClient::Dispose()
{
    if (m_impl && m_impl->IsConnected())
    {
        m_impl->Disconnect();
    }
}

bool InterprocessClient::SendRequest(const interface_key& interfaceKey,
    std::uint8_t functionNumber,
    InputParameters& results,
    const OutputParameters& arguments)
{
    if (Send(interfaceKey, functionNumber, true, arguments))
    {
        ResponseHeader response_header;
        if (m_impl->Recv(&response_header, sizeof(response_header)))
        {
            if (response_header.responseCode == ResponseCodes::Ok)
            {
                if (response_header.responseDataSize > 0)
                {
                    auto buffer = std::make_unique<std::uint8_t[]>(response_header.responseDataSize);
                    if (m_impl->Recv(buffer.get(), response_header.responseDataSize))
                    {
                        if (!results.Deserialize(buffer.get(), response_header.responseDataSize))
                        {
                            GLOG_ERROR("Failed deserialize server response");
                            assert(0);
                            return false;
                        }
                    }
                    else
                    {
                        GLOG_ERROR("Failed get response data");
                        return false;
                    }
                }

                return true;
            }
            else
            {
                GLOG_ERROR("Response failed; Code %08X", static_cast<std::uint32_t>(response_header.responseCode));
                return false;
            }
        }
        else
        {
            GLOG_ERROR("Failed get response header");
            return false;
        }
    }
    return false;
}

bool InterprocessClient::SendEvent(const interface_key& interfaceKey, std::uint8_t functionNumber, const OutputParameters& arguments)
{
    if (Send(interfaceKey, functionNumber, false, arguments))
    {
        ResponseHeader response_header;
        if (m_impl->Recv(&response_header, sizeof(response_header)))
        {
            if (response_header.responseCode == ResponseCodes::Ok && response_header.responseDataSize == 0)
            {
                return true;
            }
            else
            {
                GLOG_ERROR("Response failed; Code %08X", static_cast<std::uint32_t>(response_header.responseCode));
                return false;
            }
        }
        else
        {
            GLOG_ERROR("Failed get response header");
            return false;
        }
    }
    return false;
}

bool InterprocessClient::Send(const interface_key& interfaceKey, std::uint8_t functionNumber, bool request, const OutputParameters& arguments)
{
    if (m_impl && m_impl->IsConnected())
    {
        RequestHeader request_header;
        memset(&request_header, 0, sizeof(request_header));

        const auto size = arguments.GetSize();
        request_header.functionCode = functionNumber;
        request_header.requestDataSize = size;
        request_header.request = request;
        std::copy(interfaceKey.begin(), interfaceKey.begin() + 8, request_header.interfaceKey);

        if (m_impl->Send(&request_header, sizeof(request_header)))
        {
            if (size > 0)
            {
                if (!m_impl->Send(arguments.GetData(), size))
                {
                    GLOG_ERROR("Failed send request data");
                    return false;
                }
            }

            return true;
        }
        else
        {
            GLOG_ERROR("Failed send request header");
            return false;
        }
    }
    return false;
}

}
}
