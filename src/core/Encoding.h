#pragma once

#include <string>

namespace Gengine {
std::string toUtf8(const std::wstring& in);
std::wstring utf8toWchar(const std::string& in);
}
