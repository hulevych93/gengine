#include <filesystem/FileFilters.h>

#include <boost/regex.hpp>

namespace Gengine {

bool DefaultFilter::Match(const std::wstring&) const {
  return true;
}

struct RegexFilter::RegexFilterImpl {
  mutable std::shared_ptr<boost::wregex> rule;
  std::uint32_t flag;
};

RegexFilter::RegexFilter(const std::wstring& filter)
    : m_filter(filter), m_impl(std::make_unique<RegexFilterImpl>()) {
  m_impl->flag = boost::regex_constants::normal;
}

RegexFilter::~RegexFilter() = default;

RegexFilter::RegexFilter(std::wstring const& filter, std::uint32_t flag)
    : m_filter(filter), m_impl(std::make_unique<RegexFilterImpl>()) {
  m_impl->flag = flag;
}

bool RegexFilter::Match(std::wstring const& value) const {
  if (!m_impl->rule)
    LazyInitRule();

  return boost::regex_match(value, *m_impl->rule);
}

void RegexFilter::LazyInitRule() const {
  if (m_filter.empty())
    throw std::logic_error("Path filter is empty");

  try {
    m_impl->rule = std::make_shared<boost::wregex>(m_filter, m_impl->flag);
  } catch (boost::regex_error const& err) {
    throw std::runtime_error(boost::str(
        boost::format("Path filter is incorrect: %1%") % err.what()));
  }
}

}  // namespace Gengine