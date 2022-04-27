#include <filesystem/FileSortRules.h>

namespace Gengine {

int SortByLevel::operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs) const
{
    auto lhsLevel = lhs->Level();
    auto rhsLevel = rhs->Level();

    if (lhsLevel == rhsLevel)
        return 0;

    return lhsLevel < rhsLevel ? 1 : -1;
}

int SortByFileName::operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs) const
{
    return rhs->FileName().compare(lhs->FileName());
}

int SortByTime::operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs) const
{
    auto lhsTime = lhs->LastWriteTime();
    auto rhsTime = rhs->LastWriteTime();

    if (lhsTime == rhsTime)
        return 0;

    return lhsTime < rhsTime ? -1 : 1;
}

}