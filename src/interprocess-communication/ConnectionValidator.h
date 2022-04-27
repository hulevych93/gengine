#pragma once

#include <interprocess-communication/InterprocessCommonDefs.h>

namespace Gengine {
namespace InterprocessCommunication {

struct ConnectionValidator : boost::static_visitor<bool>
{
    bool operator()(const PipeConnection& data) const;
    bool operator()(const TcpConnection& data) const;
};
}
}
