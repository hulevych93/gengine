#pragma once

#include <filesystem/FileDetails.h>

namespace Gengine {

struct SortByLevel
{
    int operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs) const;
};

struct SortByFileName
{
    int operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs) const;
};

struct SortByTime
{
    int operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs) const;
};

struct ISortRule
{
    virtual bool operator()(SearchResult_t::value_type const& rhs, SearchResult_t::value_type const& lhs) const = 0;
};

template<typename Rule = SortByLevel, typename ExtraRule = SortByFileName>
struct SortRule : public ISortRule
{
    SortRule(const Rule& rule, const ExtraRule& extraRule) 
        : _Rule(rule)
        , _ExtraRule(extraRule)
    {}

    bool operator()(SearchResult_t::value_type const& lhs, SearchResult_t::value_type const& rhs)  const override
    {
        int result = _Rule(lhs, rhs);

        if (result == 0)
            result = _ExtraRule(lhs, rhs);

        return result > 0;
    }

    Rule _Rule;
    ExtraRule _ExtraRule;
};

template<typename Rule, typename ExtraRule>
SortRule<Rule, ExtraRule> MakeSortRule(const Rule& rule, const ExtraRule& extraRule)
{
    return SortRule<Rule, ExtraRule>(rule, extraRule);
}

}