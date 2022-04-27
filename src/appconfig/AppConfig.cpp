#include <appconfig/AppConfig.h>

namespace Gengine {
namespace AppConfig {

bool Merge(EntryConfig& left, const EntryConfig& right)
{
    if (!left.consoleLogger)
        left.consoleLogger = right.consoleLogger;
    if (!left.singleObjectName)
        left.singleObjectName = right.singleObjectName;
    if (!left.aliveObjectName)
        left.aliveObjectName = right.aliveObjectName;
    if (!left.lifetimeServiceId)
        left.lifetimeServiceId = right.lifetimeServiceId;
    if (!left.pluginsDir)
        left.pluginsDir = right.pluginsDir;
    if (!left.persistencyDir)
        left.persistencyDir = right.persistencyDir;

    std::copy(right.processRefs.begin(), right.processRefs.end(), std::inserter(left.processRefs, left.processRefs.end()));
    std::copy(right.inServices.begin(), right.inServices.end(), std::inserter(left.inServices, left.inServices.end()));
    std::copy(right.outServices.begin(), right.outServices.end(), std::inserter(left.outServices, left.outServices.end()));
    std::copy(right.processes.begin(), right.processes.end(), std::inserter(left.processes, left.processes.end()));
    std::copy(right.workers.begin(), right.workers.end(), std::inserter(left.workers, left.workers.end()));

    return true;
}

}
}