#include <main/wxArgsVisitor.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif


namespace Gengine {
namespace Entries {

void wxArgsVisitor::operator () (const wargv_type& args) const
{
    auto argc = args.argc;
    ::wxEntry(argc, args.argv);
}

void wxArgsVisitor::operator () (const argv_type& args) const
{
    auto argc = args.argc;
    ::wxEntry(argc, args.argv);
}

#if defined (BUILD_WINDOWS)
void wxArgsVisitor::operator () (const WinArgs& args) const
{
    ::wxEntry(reinterpret_cast<HINSTANCE>(args.instance), reinterpret_cast<HINSTANCE>(args.prevInstance), args.cmdLine, args.cmdShow);
}
#endif

}
}