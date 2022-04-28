#include "ArgsVisitors.h"

#if defined (_WIN32)
#include <Windows.h>
#endif

namespace bpo = boost::program_options;

namespace Gengine {
namespace Entries {

bool EmptyVisitor::operator()(const argv_type& args) const
{
    return args.argc <= 1;
}

bool EmptyVisitor::operator()(const wargv_type& args) const
{
    return args.argc <= 1;
}

#if defined (_WIN32)
bool EmptyVisitor::operator()(const WinArgs& args) const
{
    auto argc = 0;
    auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    auto result = argc <= 1;
    ::LocalFree(argv);
    return result;
}
#endif

ParseVisitor::ParseVisitor(boost::program_options::variables_map& map, boost::program_options::options_description& desc)
    : m_map(map)
    , m_option_description(desc)
{}

void ParseVisitor::operator()(const wargv_type& args) const
{
    auto options = bpo::wcommand_line_parser(args.argc, args.argv).allow_unregistered().options(m_option_description).run();
    auto unregistered = bpo::collect_unrecognized(options.options, bpo::include_positional);
    bpo::store(options, m_map);
}

void ParseVisitor::operator()(const argv_type& args) const
{
    auto options = bpo::command_line_parser(args.argc, args.argv).allow_unregistered().options(m_option_description).run();
    auto unregistered = bpo::collect_unrecognized(options.options, bpo::include_positional);
    bpo::store(options, m_map);
}

#if defined (_WIN32)
void ParseVisitor::operator()(const WinArgs& args) const
{
    auto argc = 0;
    auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
    bpo::store(bpo::wcommand_line_parser(argc, argv).allow_unregistered().options(m_option_description).run(), m_map);
    ::LocalFree(argv);
}
#endif

}
}
