#include "ConnectionValidator.h"

namespace Gengine {
namespace InterprocessCommunication {
bool ConnectionValidator::operator()(const PipeConnection& data) const
{
    return !data.pipe.empty();
}

bool ConnectionValidator::operator()(const TcpConnection& data) const
{
    return true;
}
}
}
