#pragma once

#include <boost/variant.hpp>
#include <entries/BaseArgs.h>

namespace Gengine {
namespace Entries {

class gtestArgsVisitor : public boost::static_visitor<void>
{
public:
    void operator () (const wargv_type& args) const;
    void operator () (const argv_type& args) const;

#if defined (BUILD_WINDOWS)
    void operator () (const WinArgs& args) const;
#endif
};

}
}