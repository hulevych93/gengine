#pragma once

#include <entries/BaseArgs.h>
#include <boost/variant.hpp>

namespace Gengine {
namespace Entries {

class gtestArgsVisitor : public boost::static_visitor<void> {
 public:
  void operator()(const wargv_type& args) const;
  void operator()(const argv_type& args) const;

#if defined(_WIN32)
  void operator()(const WinArgs& args) const;
#endif
};

}  // namespace Entries
}  // namespace Gengine