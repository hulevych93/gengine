#include <entries/TestArgsVisitor.h>

#include <gtest/gtest.h>

namespace Gengine
{
namespace Entries
{

void gtestArgsVisitor::operator () (const wargv_type& args) const
{
    auto argc = args.argc;
    ::testing::InitGoogleTest(&argc, args.argv);
}

void gtestArgsVisitor::operator () (const argv_type& args) const
{
    auto argc = args.argc;
    ::testing::InitGoogleTest(&argc, args.argv);
}

#if defined (_WIN32)
void gtestArgsVisitor::operator () (const WinArgs& args) const
{}
#endif

}
}