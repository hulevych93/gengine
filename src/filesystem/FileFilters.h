#pragma once

#include <string>
#include <boost/format.hpp>

namespace Gengine {

struct IFilter
{
    virtual ~IFilter() = default;
    virtual bool Match(const std::wstring& value) const = 0;
};

struct DefaultFilter : public IFilter
{
    bool Match(std::wstring const& value) const override;
};

struct RegexFilter : public IFilter
{
    explicit RegexFilter(const std::wstring& filter);
    RegexFilter(std::wstring const& filter, std::uint32_t flag);
    ~RegexFilter();

    bool Match(std::wstring const& value) const override;

private:
    void LazyInitRule() const;

private:
    std::wstring m_filter;
    struct RegexFilterImpl;
    std::unique_ptr<RegexFilterImpl> m_impl;
};

}