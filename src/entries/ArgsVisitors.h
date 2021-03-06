#pragma once

#include <entries/BaseArgs.h>
#include <boost/program_options.hpp>
#include <boost/variant.hpp>

namespace Gengine {
namespace Entries {
class EmptyVisitor : public boost::static_visitor<bool> {
 public:
  bool operator()(const wargv_type& args) const;
  bool operator()(const argv_type& args) const;
#if defined(_WIN32)
  bool operator()(const WinArgs& args) const;
#endif
};

class ParseVisitor : public boost::static_visitor<void> {
 public:
  ParseVisitor(boost::program_options::variables_map& map,
               boost::program_options::options_description& desc);

  void operator()(const wargv_type& args) const;
  void operator()(const argv_type& args) const;
#if defined(_WIN32)
  void operator()(const WinArgs& args) const;
#endif
 private:
  boost::program_options::variables_map& m_map;
  boost::program_options::options_description& m_option_description;
};
}  // namespace Entries
}  // namespace Gengine
